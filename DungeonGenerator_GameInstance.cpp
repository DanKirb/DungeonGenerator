// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonGenerator_GameInstance.h"

FRandomStream UDungeonGenerator_GameInstance::InitalizeStream(int32 StreamInput)
{
	// Check if there is an input seed, if not generate a new one
	if (StreamInput)
	{
		Stream.Initialize(StreamInput);
	}
	else
	{
		Stream.GenerateNewSeed();
	}

	return Stream;
}