// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"

#include "Room.h"
#include "DungeonGenerator.generated.h"


UENUM(BlueprintType)
enum class EObjectLocation : uint8
{
	EOL_Floor UMETA(DisplayName = "Floor"),
	EOL_Wall UMETA(DisplayName = "Wall"),
	EOL_Ceiling UMETA(DisplayName = "Ceiling")
};

USTRUCT(BlueprintType)
struct FLightSource
{
	GENERATED_BODY()

	/** The light source actor to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AActor> LightActor;

	/** Maximum number of lights per wall or ceiling */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxPerLength;

	/** The section of the room the lights will be placed along */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EObjectLocation Location;
};

USTRUCT(BlueprintType)
struct FTileMesh
{
	GENERATED_BODY()

	/** The mesh to be used as the tile */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UStaticMesh* Mesh;

	/** The probability of the mesh being spawned */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Probability;
};

USTRUCT(BlueprintType)
struct FInstancedTileMesh
{
	GENERATED_BODY()

	/** The InstancedStaticMeshComponent to be used to spawnt the tile */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UInstancedStaticMeshComponent* InstancedMeshComponent;

	/** The probability of the mesh being spawned */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Probability;
};

USTRUCT(BlueprintType)
struct FRoomType : public FTableRowBase
{
	GENERATED_BODY()

	/** The meshes to be used as the floor tiles */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FTileMesh> FloorTileMeshes;

	/** The meshes to be used as the wall tiles */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FTileMesh> WallTileMeshes;

	/** The meshes to be used as the ceiling tiles */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FTileMesh> CeilingTileMeshes;
	
	/** The light actor to be spawned in room */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FLightSource> LightActors;

	/** The number of tiles high the room is */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 WallHeight;
};

USTRUCT()
struct FConnectingRoom
{
	GENERATED_BODY()

	/** The indexes of the rooms in the Rooms array */
	int32 RoomAIndex;
	int32 RoomBIndex;
};

UCLASS()
class DUNGEON_CPP_API ADungeonGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	
	ADungeonGenerator();

	/** The mesh to be used as the floor tiles */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon | Rooms")
	class UDataTable* RoomTypesDataTable;

	/** InstancedStaticMeshComponents are added to these arrays using the meshes from RoomTypesDataTable */
	TArray<FInstancedTileMesh> RoomFloorTiles;
	TArray<FInstancedTileMesh> RoomWallTiles;
	TArray<FInstancedTileMesh> RoomCeilingTiles;

	/** The meshes to be used as the door tiles */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon | Doors")
	TArray<FTileMesh> DoorTileMeshes;

	/** InstancedStaticMeshComponent are added to this array using  the meshes from DoorTileMeshes */
	TArray<FInstancedTileMesh> DoorTiles;

	/** The meshes to be used as the floor tiles */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon | Corridors")
	TArray<FTileMesh> CorridorFloorTileMeshes;

	/** The meshes to be used as the floor tiles */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon | Corridors")
	TArray<FTileMesh> CorridorWallTileMeshes;

	/** The meshes to be used as the ceiling tiles */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon | Corridors")
	TArray<FTileMesh> CorridorCeilingTileMeshes;

	/** The InstancedStaticMeshComponents used to spawn the corridor tiles */
	TArray<FInstancedTileMesh> CorridorFloorTiles;
	TArray<FInstancedTileMesh> CorridorWallTiles;
	TArray<FInstancedTileMesh> CorridorCeilingTiles;

	/** The size of each tile, this needs to match the width and height of the selected tiles */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon | Config")
	int32 TileSize;

	/** The minimum number of tiles a room can have */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon | Config")
	int32 MinRoomSize;

	/** The maximum number of tiles a room can have */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon | Config")
	int32 MaxRoomSize;

	/** The minimum number of tiles a room can have */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon | Config")
	int32 MinRoomDistance;

	/** The maximum number of tiles a room can have */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon | Config")
	int32 MaxRoomDistance;

	/** The maximum number of rooms to generate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon | Config")
	int32 MaxNumberOfRooms;

	/** The stream used to generate random numbers */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon | Stream")
	FRandomStream Stream;

	/** The text to use for the FRandomStream */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon | Stream")
	int32 StreamInput;

private:

	/** The generated rooms */
	TArray<URoom*> Rooms;

	/** Each room connection contains the index of the two rooms */
	TArray<FConnectingRoom> RoomConnections;

	/** The amount the dungeon has been moved to align with the starting area */
	FVector DungeonOffset;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:

	/** Generates a Room and checks if it overlaps with an existing room, then returns the room */
	URoom* TryPlaceRoom();

	/** Generates a Room using MinRoomSize and MaxRoomSize */
	URoom* GenerateNewRoom();

	/** Checks if RoomToCheck overlaps with any of the rooms in the Rooms array */
	void CheckRoomIsNotOverlappingOtherRooms(URoom*& RoomToCheck);

	/** Spawns a random tile from the TilesArray at the AtLocation*/
	void SpawnTile(const TArray<FInstancedTileMesh>& InstancedTileMeshesArray, const FTransform& AtLocation);

	/** Loops through the Rooms array and spawns the tiles */
	void SpawnRooms();

	/** Randomly selects a row from the RoomTypes DataTable and creates arrays of InstancedStaticMeshes to be spawned */
	void CreateInstancedStaticMeshesForCurrentRoom(URoom*& CurrentRoom);

	/** Spawn floor tiles from the CorridorStart to the CorridorEnd */
	void SpawnCorridorTiles(const FVector& CorridorStart, const FVector& CorridorEnd);

	/** Spawn walls for the passed in URoom */
	void SpawnRoomWalls(URoom*& Room);

	/** Spawns wall and door tiles from the StartPoint to the EndPoint with the Rotation */
	void SpawnWall(FVector& StartPoint, FVector& EndPoint, FRotator& Rotation, const int32& Height, TArray<FVector> DoorLocations);

	/** Spawns the corridor tiles between the rooms in each FConnectingRoom */
	void CreateCorridors(const TArray<FConnectingRoom> &ConnectingRooms);
		
	/**  Finds the lowest room on the X and aligns the center of it with the starting location */
	void MoveDungeonToStartArea();

	/** Create InstancedStaticMeshComponent from the Meshes passed in */
	void CreateInstancedStaticMeshComponents(const TArray<FTileMesh>& TileMeshes, TArray<FInstancedTileMesh>& InstancedTileMeshes);

	/** Spawns light sources in all rooms */
	void SpawnLightsInRooms();
};
