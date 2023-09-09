// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Level.h"

class SD_Actor
{
public:
	SD_Actor() {};
	SD_Actor(const AActor* Actor)
	{
		const FTransform& transform = Actor->GetTransform();
		xLoc = transform.GetLocation().X;
		yLoc = transform.GetLocation().Y;
		zLoc = transform.GetLocation().Z;

		xRot = transform.GetRotation().X;
		yRot = transform.GetRotation().Y;
		zRot = transform.GetRotation().Z;

		xVel = Actor->GetVelocity().X;
		yVel = Actor->GetVelocity().Y;
		zVel = Actor->GetVelocity().Z;
	}

	friend FArchive& operator<<(FArchive& Archive, SD_Actor& Data)
	{
		Archive << Data.xLoc;
		Archive << Data.yLoc;
		Archive << Data.zLoc;

		Archive << Data.xRot;
		Archive << Data.yRot;
		Archive << Data.zRot;

		Archive << Data.xVel;
		Archive << Data.yVel;
		Archive << Data.zVel;
		return Archive;
	}

	float xLoc = 0, yLoc = 0, zLoc = 0;
	float xRot = 0, yRot = 0, zRot = 0;
	float xVel = 0, yVel = 0, zVel = 0;
};
