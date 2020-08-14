// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Room.generated.h"

/**
 * 
 */

UCLASS()
class DUNGEON_CPP_API URoom : public UObject
{
	GENERATED_BODY()

public:

	URoom();

	/** The rooms position in World Space */
	FVector Position;

	/** The rooms size */
	FVector Size;

	/** Whether or not the can be placed in the world */
	bool bCanBePlaced;

	/** A unique identifier for the room */
	int32 Index;

	/** The location of the doors on this room */
	TArray<FVector> DoorLocations;

	/** The number of tiles high the room is */
	int32 WallHeight;

	/** The name of the row in the Data Table being used for this room */
	FName RoomTypeRowName;

public:

	/** Returns the centre point of the room */
	FVector GetCenterOfRoom();
	
	/** Returns the extent of the room */
	FVector GetRoomExtent();

	FORCEINLINE void SetRoomPosition(FVector Pos) { Position = Pos; }

	FORCEINLINE void SetWallHeight(int32 Height) { WallHeight = Height; }
};
