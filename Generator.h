// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Generator.generated.h"

UCLASS()
class DUNGEON_CPP_API AGenerator : public AActor
{
	GENERATED_BODY()

public:
    AGenerator();

	/** The stream used to generate random numbers */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stream")
	FRandomStream Stream;

public:

	/** Initialize the stream in the game instance */
	FRandomStream& InitializeStream(int32 StreamInput);

	/** Get the stream from the game instance */
	FRandomStream& GetStream();

	/** Get the game instance */
	class UDungeonGenerator_GameInstance* GetDungeonGameInstance();
};
