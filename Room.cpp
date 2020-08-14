// Fill out your copyright notice in the Description page of Project Settings.


#include "Room.h"

URoom::URoom()
{

}

FVector URoom::GetCenterOfRoom()
{
	return Position - (Size / 2);
}

FVector URoom::GetRoomExtent()
{
	return Position + Size;
}