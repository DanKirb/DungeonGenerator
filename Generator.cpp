// Fill out your copyright notice in the Description page of Project Settings.


#include "Generator.h"
#include "Kismet/GameplayStatics.h"
#include "DungeonGenerator_GameInstance.h"
#include "Engine/World.h"

AGenerator::AGenerator()
{
}

FRandomStream& AGenerator::InitializeStream(int32 StreamInput)
{
	UDungeonGenerator_GameInstance* DungeonGameInstance = GetDungeonGameInstance();
	if (DungeonGameInstance)
	{
		Stream = DungeonGameInstance->InitalizeStream(StreamInput);
	}

	return Stream;
}

FRandomStream& AGenerator::GetStream()
{
	UDungeonGenerator_GameInstance* DungeonGameInstance = GetDungeonGameInstance();
	if (DungeonGameInstance)
	{
		Stream = DungeonGameInstance->GetStream();
	}

	return Stream;
}

UDungeonGenerator_GameInstance* AGenerator::GetDungeonGameInstance()
{
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(GetWorld());

	if (GameInstance)
	{
		return Cast<UDungeonGenerator_GameInstance>(GameInstance);
	}

	return nullptr;
}