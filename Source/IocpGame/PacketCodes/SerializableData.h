// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Level.h"

class SD_Transform
{
public:
	SD_Transform() {};
	SD_Transform(const FTransform* transform)
	{
		Transform = *transform;
	}

	friend FArchive& operator<<(FArchive& Archive, SD_Transform& Data)
	{
		Archive << Data.Transform;
		return Archive;
	}

	FTransform Transform;
};
