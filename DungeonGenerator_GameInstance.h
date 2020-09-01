// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "DungeonGenerator_GameInstance.generated.h"

/**
 * 
 */
UCLASS()
class DUNGEON_CPP_API UDungeonGenerator_GameInstance : public UGameInstance
{
	GENERATED_BODY()
	
protected:

	/** The stream used to generate random numbers */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon | Stream")
	FRandomStream Stream;

public:

	/** Generates a new stream or initialiszes it with the input seed  */
	FRandomStream InitalizeStream(int32 StreamInput = 0);

	FORCEINLINE FRandomStream GetStream() { return Stream; }
};
