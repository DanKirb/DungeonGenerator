// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Room.h"
#include "Generator.h"
#include "DungeonGenerator.generated.h"

UENUM(BlueprintType)
enum class EObjectLocation : uint8
{
	EOL_AroundRoom UMETA(DisplayName = "AroundRoom"),
	EOL_Ceiling UMETA(DisplayName = "Ceiling")
};

USTRUCT(BlueprintType)
struct FLightSource
{
	GENERATED_BODY()

	/** The light actor to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AActor> LightActor;

	/** The tile distance between each light */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 TileDistanceBetweenNext;
	
	/** The section of the room the lights will be placed along */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EObjectLocation Location;
};

USTRUCT(BlueprintType)
struct FTile
{
	GENERATED_BODY()

	/** The mesh to be used as the tile */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UStaticMesh* Mesh;

	/** The InstancedStaticMeshComponent to be used to spawn the tile */
	UInstancedStaticMeshComponent* InstancedMeshComponent;
};

USTRUCT(BlueprintType)
struct FRandomTile : public FTile
{
	GENERATED_BODY()

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
	TArray<FRandomTile> FloorTileMeshes;

	/** The meshes to be used as the wall tiles */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FRandomTile> WallTileMeshes;

	/** The meshes to be used as the wall addition tiles */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FRandomTile> WallAdditionTileMeshes;

	/** The meshes to be used as the door tiles */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FRandomTile> DoorTileMeshes;

	/** The meshes to be used as the door addition tiles */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FRandomTile> DoorAdditionTileMeshes;

	/** The meshes to be used as the ceiling tiles */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FRandomTile> CeilingTileMeshes;
	
	/** The light actors to be spawned in room */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FLightSource> LightActors;
	
	/** The number of tiles high the room is */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 WallHeight;

	/** The probability of this room being spawned */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Probability;
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
class DUNGEON_CPP_API ADungeonGenerator : public AGenerator
{
	GENERATED_BODY()
	
public:	
	
	ADungeonGenerator();

	/** The room types data */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon | Rooms")
	class UDataTable* RoomTypesDataTable;

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
	int32 NumberOfRooms;

	/** The tiles to be used for each section of the rooms */
	TArray<FRandomTile> RoomFloorTiles;
	TArray<FRandomTile> RoomWallTiles;
	TArray<FRandomTile> RoomWallAdditionTiles;
	TArray<FRandomTile> RoomCeilingTiles;
	TArray<FRandomTile> RoomDoorTiles;
	TArray<FRandomTile> RoomDoorAdditionTiles;

	/** The meshes to be used as the floor tiles in the corridors */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon | Corridors")
	TArray<FRandomTile> CorridorFloorTileMeshes;

	/** The meshes to be used as the wall tiles in the corridors */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon | Corridors")
	TArray<FRandomTile> CorridorWallTileMeshes;

	/** The meshes to be used as the ceiling tiles in the corridors */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon | Corridors")
	TArray<FRandomTile> CorridorCeilingTileMeshes;

	/** The tiles to be used for each section of the corridors */
	TArray<FRandomTile> CorridorFloorTiles;
	TArray<FRandomTile> CorridorWallTiles;
	TArray<FRandomTile> CorridorCeilingTiles;
	
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

private:

	/** Places a newly generated room if it doesn't overlap with an existing room */
	URoom* TryPlaceRoom();

	/** Generates a Room using MinRoomSize and MaxRoomSize */
	URoom* GenerateNewRoom();

	/** Checks if RoomToCheck overlaps with any of the rooms in the Rooms array */
	void CheckRoomIsNotOverlappingOtherRooms(URoom*& RoomToCheck);

	/** Spawns a random tile from the TilesArray at the AtLocation*/
	void SpawnRandomTile(const TArray<FRandomTile>& InstancedTileMeshesArray, const FTransform& AtLocation);

	/** Spawns a random tile from the TilesArray at the AtLocation*/
	void SpawnAllTiles(const TArray<FRandomTile>& InstancedTileMeshesArray, const FTransform& AtLocation, const int32 CurrentHeight);

	/** Loops through the Rooms array and spawns the tiles */
	void SpawnRooms();

	/** Creates arrays of InstancedStaticMeshes from the SelectedRoomType to be spawned */
	void CreateInstancedStaticMeshesForCurrentRoom(FRoomType*& SelectedRoomType);

	/** Randomly selects a row from the RoomTypes DataTable and applies it to the CurrentRoom  */
	FRoomType* PickRandomRoomTypeForRoom(URoom*& CurrentRoom);

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
	void CreateInstancedStaticMeshComponents(const TArray<FRandomTile>& TileMeshes, TArray<FRandomTile>& InstancedTileMeshes);

	/** Spawns light sources in all rooms */
	void SpawnLightsInRooms();

	/** Pick a random point where the two rooms will still overlap, pass in all X values or Y values */
	int32 GetRandomPointWhereRoomsOverlap(const float ConnectingRoomPosition, const float ConnectingRoomExtent, const float NewRoomSize);

	/** Spawns as many lights as possible from StartLocation to EndLocation with at least the GapBetweenLights between them */
	void SpawnLightsAlongLength(FVector StartLocation, FVector EndLocation, FRotator Rotation, int32 GapBetweenLights, TSubclassOf<AActor> ActorToSpawn);
};
