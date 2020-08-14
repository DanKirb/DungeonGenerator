// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonGenerator.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/Engine.h"
#include "UObject/UObjectGlobals.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

// Sets default values
ADungeonGenerator::ADungeonGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Configure defaults
	TileSize = 600;
	MinRoomSize = 3;
	MaxRoomSize = 6;
	MaxNumberOfRooms = 15;
	MinRoomDistance = 1;
	MaxRoomDistance = 3;
	StreamInput = 0;
}

// Called when the game starts or when spawned
void ADungeonGenerator::BeginPlay()
{
	Super::BeginPlay();
	
	// Check if there is an input seed, if not generate a new one
	if (StreamInput)
	{
		Stream.Initialize(StreamInput);
	}
	else
	{
		Stream.GenerateNewSeed();
	}

	// Create the constant InstancedStaticMeshComponents
	CreateInstancedStaticMeshComponents(DoorTileMeshes, DoorTiles);
	CreateInstancedStaticMeshComponents(CorridorFloorTileMeshes, CorridorFloorTiles);
	CreateInstancedStaticMeshComponents(CorridorWallTileMeshes, CorridorWallTiles);
	CreateInstancedStaticMeshComponents(CorridorCeilingTileMeshes, CorridorCeilingTiles);

	// Create array of rooms that can be placed in the world
	while (Rooms.Num() < MaxNumberOfRooms)
	{
		TryPlaceRoom();
	}


	// Move dungeon so lowest room connects to starting area
	MoveDungeonToStartArea();

	CreateCorridors(RoomConnections);
	SpawnRooms();
	SpawnLightsInRooms();
}

// Called every frame
void ADungeonGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

URoom* ADungeonGenerator::TryPlaceRoom()
{
	URoom* NewRoom = GenerateNewRoom();

	if (Rooms.Num() > 0)
	{
		// Pick a random room to place the NewRoom next to
		int32 RoomIndexToConnectRoomTo = UKismetMathLibrary::RandomIntegerInRangeFromStream(0, Rooms.Num() - 1, Stream);
		URoom* ConnectingRoom = Rooms[RoomIndexToConnectRoomTo];

		int32 SpaceBetweenRooms = UKismetMathLibrary::RandomIntegerInRangeFromStream(MinRoomDistance, MaxRoomDistance, Stream);
		FVector NewRoomPosition = FVector(0.f);

		// Pick a random direction to spawn the room
		int32 DirectionToPlaceRoom = UKismetMathLibrary::RandomIntegerInRangeFromStream(0, 3, Stream);
		switch (DirectionToPlaceRoom)
		{
		case 0: // Top
			NewRoomPosition = FVector(ConnectingRoom->GetRoomExtent().X + SpaceBetweenRooms, ConnectingRoom->Position.Y, 0.f);
			break;
		case 1: // Right
			NewRoomPosition = FVector(ConnectingRoom->Position.X, ConnectingRoom->GetRoomExtent().Y + SpaceBetweenRooms, 0.f);
			break;
		case 2: // Bottom
			NewRoomPosition = FVector(ConnectingRoom->Position.X - NewRoom->Size.X - SpaceBetweenRooms, ConnectingRoom->Position.Y, 0.f);
			break;
		case 3: // Left
			NewRoomPosition = FVector(ConnectingRoom->Position.X, ConnectingRoom->Position.Y - NewRoom->Size.Y - SpaceBetweenRooms, 0.f);
			break;
		default:
			break;
		}

		NewRoom->SetRoomPosition(NewRoomPosition);

		CheckRoomIsNotOverlappingOtherRooms(NewRoom);

		if (NewRoom->bCanBePlaced)
		{
			Rooms.Add(NewRoom);

			FConnectingRoom RoomConnection;
			RoomConnection.RoomAIndex = ConnectingRoom->Index;
			RoomConnection.RoomBIndex = NewRoom->Index;

			RoomConnections.Add(RoomConnection);
		}
	}
	else
	{
		Rooms.Add(NewRoom);

		NewRoom->SetRoomPosition(FVector(0.f));
	}

	return NewRoom;
}

URoom* ADungeonGenerator::GenerateNewRoom()
{
	float RoomSizeX = UKismetMathLibrary::RandomIntegerInRangeFromStream(MinRoomSize, MaxRoomSize, Stream);
	float RoomSizeY = UKismetMathLibrary::RandomIntegerInRangeFromStream(MinRoomSize, MaxRoomSize, Stream);

	URoom* NewRoom = NewObject<URoom>(this);
	NewRoom->Size = FVector(RoomSizeX, RoomSizeY, 0.f);
	NewRoom->bCanBePlaced = false;
	NewRoom->Index = Rooms.Num();

	return NewRoom;
}

void ADungeonGenerator::CheckRoomIsNotOverlappingOtherRooms(URoom* &RoomToCheck)
{
	for(URoom* CurrentRoom : Rooms)
	{
		// Check if the two rooms overlap on the X
		float X1 = FMath::Max(CurrentRoom->Position.X, RoomToCheck->Position.X);
		float X2 = FMath::Min(CurrentRoom->GetRoomExtent().X, RoomToCheck->GetRoomExtent().X);
		
		bool OverlappingOnX = X1 <= X2;

		// Check if the two rooms overlap on the Y
		float Y1 = FMath::Max(CurrentRoom->Position.Y, RoomToCheck->Position.Y);
		float Y2 = FMath::Min(CurrentRoom->GetRoomExtent().Y, RoomToCheck->GetRoomExtent().Y);

		bool OverlappingOnY = Y1 <= Y2;

		// If they overlap on both the X and the Y then the rooms are overlapping
		if (OverlappingOnX && OverlappingOnY)
		{
			TryPlacingRoomCount++;
			RoomToCheck->bCanBePlaced = false;
			return;
		}
	}
	
	TryPlacingRoomCount = 0;
	RoomToCheck->bCanBePlaced = true;
}

void ADungeonGenerator::SpawnRooms()
{
	for (URoom* Room : Rooms)
	{
		CreateInstancedStaticMeshesForCurrentRoom(Room);

		for (float x = 0; x < Room->Size.X; x++)
		{
			for (float y = 0; y < Room->Size.Y; y++)
			{
				FVector ActualRoomPosition = Room->Position * TileSize;
				FVector PositionOfTile = ActualRoomPosition + FVector(x * TileSize, y * TileSize, 0.f);

				// Spawn floor tile				
				SpawnTile(RoomFloorTiles, FTransform(PositionOfTile));

				// Spawn ceiling tile				
				SpawnTile(RoomCeilingTiles, FTransform(PositionOfTile + FVector(0.f, 0.f, TileSize * Room->WallHeight)));
			}
		}

		SpawnRoomWalls(Room);
	}	
}

void ADungeonGenerator::SpawnRoomWalls(URoom* &Room)
{
	FRotator WallRotation = FRotator(0.f);

	// Spawn wall tiles along bottom wall
	FVector StartPoint = Room->Position;
	FVector EndPoint = Room->Position + FVector(0.f, Room->Size.Y, 0.f);
	SpawnWall(StartPoint, EndPoint, WallRotation, Room->WallHeight, Room->DoorLocations);

	// Spawn wall tiles along top wall
	WallRotation = FRotator(0.f, 180.f, 0.f);
	StartPoint = Room->Position + FVector(Room->Size.X, Room->Size.Y, 0.f);
	EndPoint = Room->Position + FVector(Room->Size.X, 0.f, 0.f);
	SpawnWall(StartPoint, EndPoint, WallRotation, Room->WallHeight, Room->DoorLocations);

	// Spawn wall tiles along left wall
	WallRotation = FRotator(0.f, 90.f, 0.f);
	StartPoint = Room->Position + FVector(Room->Size.X, 0.f, 0.f);
	EndPoint = Room->Position;
	SpawnWall(StartPoint, EndPoint, WallRotation, Room->WallHeight, Room->DoorLocations);

	// Spawn wall tiles along right wall
	WallRotation = FRotator(0.f, -90.f, 0.f);
	StartPoint = Room->Position + FVector(0.f, Room->Size.Y, 0.f);;
	EndPoint = Room->GetRoomExtent();
	SpawnWall(StartPoint, EndPoint, WallRotation, Room->WallHeight ,Room->DoorLocations);
}

void ADungeonGenerator::SpawnWall(FVector &StartPoint, FVector &EndPoint, FRotator &Rotation, const int32 &Height, TArray<FVector> DoorLocations)
{
	float WallLength = (StartPoint - EndPoint).Size();
	bool SpawnAlongX = StartPoint.X != EndPoint.X;
	bool Subract = false;

	// Check if we are placing tiles forward or backward
	if (SpawnAlongX)
		Subract = StartPoint.X > EndPoint.X;
	else
		Subract = StartPoint.Y > EndPoint.Y;

	for (float i = 0; i < WallLength; i++)
	{
		for (int32 h = 0; h < Height; h++)
		{
			float X = StartPoint.X;
			float Y = StartPoint.Y;

			if (SpawnAlongX)
				Subract ? X -= i : X += i;
			else
				Subract ? Y -= i : Y += i;

			FVector Location = FVector(X, Y, h) * TileSize;

			TArray<FInstancedTileMesh> TilesToSpawn = RoomWallTiles;

			// Check if the current location should be a door
			if (DoorLocations.Contains(FVector(X, Y, h)))
			{		
				TilesToSpawn = DoorTiles;
				
			}

			SpawnTile(TilesToSpawn, FTransform(Rotation, Location));
		}
	}
}

void ADungeonGenerator::CreateInstancedStaticMeshComponents(const TArray<FTileMesh>& TileMeshes, TArray<FInstancedTileMesh>& InstancedTileMeshes)
{
	// Empty previously used meshes
	InstancedTileMeshes.Empty();

	if (TileMeshes.Num() > 0)
	{
		for (FTileMesh Tile : TileMeshes)
		{
			UInstancedStaticMeshComponent* Instance = NewObject<UInstancedStaticMeshComponent>(this);
			Instance->RegisterComponent();
			Instance->SetStaticMesh(Tile.Mesh);
			Instance->AttachTo(GetRootComponent());

			FInstancedTileMesh InstancedTileMesh;
			InstancedTileMesh.InstancedMeshComponent = Instance;
			InstancedTileMesh.Probability = Tile.Probability;

			InstancedTileMeshes.Add(InstancedTileMesh);
		}
	}
}

void ADungeonGenerator::CreateCorridors(const TArray<FConnectingRoom>& ConnectingRooms)
{
	// Sort the URooms array by the Index property so rooms can easily be pulled from the array by index
	Rooms.Sort([](const URoom& A, const URoom& B) { return A.Index < B.Index; });
	
	// Loop through the connecting rooms
	for (FConnectingRoom RoomConnection : ConnectingRooms)
	{
		// Get the two connecting rooms by their index
		URoom* RoomA = Rooms[RoomConnection.RoomAIndex];
		URoom* RoomB = Rooms[RoomConnection.RoomBIndex];

		// Find the min and max of the room positions and extents to calculate where the rooms are in 
		float MaxOriginX = FMath::Max(RoomA->Position.X, RoomB->Position.X);
		float MinExtentX = FMath::Min(RoomA->GetRoomExtent().X, RoomB->GetRoomExtent().X);

		float MaxOriginY = FMath::Max(RoomA->Position.Y, RoomB->Position.Y);
		float MinExtentY = FMath::Min(RoomA->GetRoomExtent().Y, RoomB->GetRoomExtent().Y);

		// Check if rooms are next to each other on the Y axis
		if (MaxOriginX < MinExtentX && MaxOriginY > MinExtentY)
		{			
			URoom* LeftRoom;
			URoom* RightRoom;

			// Check which room is on the right on the Y axis
			if (RoomB->Position.Y > RoomA->GetRoomExtent().Y)
			{
				RightRoom = RoomB;
				LeftRoom = RoomA;
			}
			else
			{
				RightRoom = RoomA;
				LeftRoom = RoomB;
			}

			// Pick the random point between the points the rooms overlap on the X
			float CorridorX = UKismetMathLibrary::RandomIntegerInRangeFromStream(MaxOriginX, MinExtentX - 1, Stream);

			// Corridor will go from the right side of the left room to the right room at the random point on the X
			FVector CorridorStart = FVector(CorridorX, LeftRoom->GetRoomExtent().Y, 0.f);
			FVector CorridorEnd = FVector(CorridorX, RightRoom->Position.Y, 0.f);

			// Add doors location to rooms
			LeftRoom->DoorLocations.Add(CorridorStart);

			FVector RightRoomDoorLocation = CorridorEnd;
			RightRoomDoorLocation.X += 1;
			RightRoom->DoorLocations.Add(RightRoomDoorLocation);

			SpawnCorridorTiles(CorridorStart, CorridorEnd);
		}
		// else rooms are aligned on the X axis
		else
		{
			URoom* TopRoom;
			URoom* BottomRoom;

			// Check which room is higher on the X axis
			if (RoomB->Position.X > RoomA->GetRoomExtent().X)
			{
				TopRoom = RoomB;
				BottomRoom = RoomA;
			}
			else
			{
				TopRoom = RoomA;
				BottomRoom = RoomB;
			}

			// Pick the random point between the points the rooms overlap on the Y
			float CorridorY = UKismetMathLibrary::RandomIntegerInRangeFromStream(MaxOriginY, MinExtentY - 1, Stream);			

			// Corridor will go from the top of the bottom room to the top room at the random point on the Y
			FVector CorridorStart = FVector(BottomRoom->GetRoomExtent().X, CorridorY, 0.f);
			FVector CorridorEnd = FVector(TopRoom->Position.X, CorridorY, 0.f);

			// Add door locations to rooms
			FVector BottomRoomDoorLocation = CorridorStart;
			BottomRoomDoorLocation.Y += 1;
			BottomRoom->DoorLocations.Add(BottomRoomDoorLocation);
			TopRoom->DoorLocations.Add(CorridorEnd);

			SpawnCorridorTiles(CorridorStart, CorridorEnd);
		}
	}
}

void ADungeonGenerator::SpawnCorridorTiles(const FVector &CorridorStart, const FVector &CorridorEnd)
{	
	float NumberofTiles = (CorridorStart - CorridorEnd).Size() == 0 ? 1 : (CorridorStart - CorridorEnd).Size();

	for (int32 i = 0; i <= NumberofTiles - 1; i++)
	{
		FVector PositionOfTile;
		FVector OpositeWallPosition;
		FVector WallPosition;
		FRotator WallRotation = FRotator(0.f, 0.f, 0.f);
		FRotator OpositeWallRotation = FRotator(0.f, 180.f, 0.f);
		if (CorridorStart.X == CorridorEnd.X)
		{
			// Tiles being placed along the Y axis
			PositionOfTile = FVector(CorridorStart.X, CorridorStart.Y + i, 0.f) * TileSize;
						
			WallPosition = FVector(CorridorStart.X, CorridorEnd.Y - i - 1, 0.f) * TileSize;
			OpositeWallPosition = FVector(CorridorStart.X + 1, CorridorStart.Y + i + 1, 0.f) * TileSize;
		}
		else
		{
			// Tiles being placed along the X axis
			PositionOfTile = FVector(CorridorStart.X + i, CorridorStart.Y, 0.f) * TileSize;

			WallPosition = FVector(CorridorEnd.X - i, CorridorEnd.Y, 0.f) * TileSize;
			OpositeWallPosition = FVector(CorridorStart.X + i, CorridorStart.Y + 1, 0.f) * TileSize;

			WallRotation = FRotator(0.f, 90.f, 0.f);
			OpositeWallRotation = FRotator(0.f, -90.f, 0.f);
		}

		SpawnTile(CorridorFloorTiles, FTransform(PositionOfTile));

		SpawnTile(CorridorWallTiles, FTransform(WallRotation, WallPosition));
		SpawnTile(CorridorWallTiles, FTransform(OpositeWallRotation, OpositeWallPosition));

		SpawnTile(CorridorCeilingTiles, FTransform(PositionOfTile + FVector(0.f, 0.f, TileSize)));
	}
}

void ADungeonGenerator::SpawnTile(const TArray<FInstancedTileMesh> &InstancedTileMeshesArray, const FTransform &AtLocation)
{
	if (InstancedTileMeshesArray.Num() > 0)
	{
		bool SpawnTile = false;

		while (!SpawnTile)
		{
			int32 TileToSpawnIndex = UKismetMathLibrary::RandomIntegerInRangeFromStream(0, InstancedTileMeshesArray.Num() - 1, Stream);

			// Check the tiles probablity isn't 0 to prevent infinite loop
			float Probability = InstancedTileMeshesArray[TileToSpawnIndex].Probability == 0 ? 1 : InstancedTileMeshesArray[TileToSpawnIndex].Probability;

			SpawnTile = UKismetMathLibrary::RandomBoolWithWeightFromStream(Probability, Stream);

			if (SpawnTile)
			{
				InstancedTileMeshesArray[TileToSpawnIndex].InstancedMeshComponent->AddInstance(AtLocation);
			}
		}
	}
}

void ADungeonGenerator::MoveDungeonToStartArea()
{
	URoom* LowestRoom = Rooms[0];

	for (URoom* Room : Rooms)
	{
		if (Room->Position.X < LowestRoom->Position.X)
		{
			LowestRoom = Room;
		}
	}

	FVector LowestPoint = FVector(LowestRoom->Position.X, LowestRoom->Position.Y + FMath::FloorToFloat(LowestRoom->Size.Y / 2), 0.f);

	// Add door between the starting area and the first room
	LowestRoom->DoorLocations.Add(LowestPoint);

	DungeonOffset = LowestPoint * TileSize;
	FVector EndPosition = GetActorLocation() - DungeonOffset;
	
	SetActorLocation(EndPosition);
}

void ADungeonGenerator::CreateInstancedStaticMeshesForCurrentRoom(URoom* &CurrentRoom)
{
	if (RoomTypesDataTable)
	{
		// Randomly select room row from the datatable
		auto RowNames = RoomTypesDataTable->GetRowNames();
		int32 RoomIndex = UKismetMathLibrary::RandomIntegerInRangeFromStream(0, RowNames.Num() - 1, Stream);
		FName RoomTypeRowName = RowNames[RoomIndex];

		const FString ContextString(TEXT("Selected Room Context"));
		FRoomType* SelectedRoomType = RoomTypesDataTable->FindRow<FRoomType>(RoomTypeRowName, ContextString, true);

		if (SelectedRoomType)
		{
			// Create instanced static meshes of the meshes selected in the row
			CreateInstancedStaticMeshComponents(SelectedRoomType->FloorTileMeshes, RoomFloorTiles);
			CreateInstancedStaticMeshComponents(SelectedRoomType->WallTileMeshes, RoomWallTiles);
			CreateInstancedStaticMeshComponents(SelectedRoomType->CeilingTileMeshes, RoomCeilingTiles);

			CurrentRoom->SetWallHeight(SelectedRoomType->WallHeight);
			CurrentRoom->RoomTypeRowName = RoomTypeRowName;
		}
	}
}

void ADungeonGenerator::SpawnLightsInRooms()
{
	if(RoomTypesDataTable)
	{
		const FString ContextString(TEXT("Selected Room Context"));

		for (URoom* Room : Rooms)
		{
			FRoomType* RoomType = RoomTypesDataTable->FindRow<FRoomType>(Room->RoomTypeRowName, ContextString);

			if (RoomType->LightActors.Num())
			{
				for (FLightSource LightActor : RoomType->LightActors)
				{
					FActorSpawnParameters SpawnParams;
					FVector LightLocation(0.f);

					switch (LightActor.Location)
					{
					case EObjectLocation::EOL_Wall:

						// Temporary code, just spawns the light once in the centre of the left wall
						LightLocation = Room->Position * TileSize;
						LightLocation.X += (Room->Size.X * TileSize) / 2;
						LightLocation.Z = TileSize / 2;

						LightLocation -= DungeonOffset;
						GetWorld()->SpawnActor<AActor>(LightActor.LightActor, LightLocation, FRotator(0.f, 90.f, 0.f));

						break;
					default:
						break;
					}
				}
			}
		}
	}
}