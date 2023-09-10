// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "../Enumerations.h"

class SD_Data
{
public:
	virtual void Serialize(FMemoryWriter& writer) = 0;
	virtual void Deserialize(FMemoryReader& reader) = 0;
};

class SD_Transform : SD_Data
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

	void Serialize(FMemoryWriter& writer) override { writer << *this; }
	void Deserialize(FMemoryReader& reader) override { reader << *this; }

public:
	FTransform Transform;
};


class SD_ActorPhysics : SD_Data
{
public:
	SD_ActorPhysics() {};
	SD_ActorPhysics(const AActor& actor)
	{
		Transform = actor.GetTransform();
		xVelocity = actor.GetVelocity().X;
		yVelocity = actor.GetVelocity().Y;
		zVelocity = actor.GetVelocity().Z;
	}
	SD_ActorPhysics(const FTransform& transform, const FVector& velocity)
	{
		Transform = transform;
		xVelocity = velocity.X;
		yVelocity = velocity.Y;
		zVelocity = velocity.Z;
	}

	friend FArchive& operator<<(FArchive& Archive, SD_ActorPhysics& Data)
	{
		Archive << Data.Transform;
		Archive << Data.xVelocity;
		Archive << Data.yVelocity;
		Archive << Data.zVelocity;
		return Archive;
	}

	void Serialize(FMemoryWriter& writer) override { writer << *this; }
	void Deserialize(FMemoryReader& reader) override { reader << *this; }

public:
	FTransform Transform;
	double xVelocity = 0.f;
	double yVelocity = 0.f;
	double zVelocity = 0.f;
};


// InputAction과 InputActionValue에 대한 정보를 담는다
class SD_GameInput : SD_Data
{
public:
	SD_GameInput() {}
	SD_GameInput(ActionTypeEnum actionType, const FInputActionValue& inputValue)
	{
		ActionType = actionType;
		switch (inputValue.GetValueType())
		{
			case EInputActionValueType::Axis1D:
				{
					X = inputValue.Get<float>();
					axisDimension = 1;
					break;
				}
			case EInputActionValueType::Axis2D:
				{
					X = inputValue.Get<FVector2D>().X;
					Y = inputValue.Get<FVector2D>().Y;
					axisDimension = 2;
					break;
				}
			case EInputActionValueType::Axis3D:
				{
					X = inputValue.Get<FVector>().X;
					Y = inputValue.Get<FVector>().Y;
					Z = inputValue.Get<FVector>().Z;
					axisDimension = 3;
					break;
				}
			default:
				{
					UE_LOG(LogTemp, Error, TEXT("알 수 없는 InputActionValueType입니다!"));
					break;
				}
		}
	}

	friend FArchive& operator<<(FArchive& Archive, SD_GameInput& Data)
	{
		Archive << Data.ActionType;
		Archive << Data.axisDimension;
		Archive << Data.X;
		Archive << Data.Y;
		Archive << Data.Z;
		return Archive;
	}

	void Serialize(FMemoryWriter& writer) override { writer << *this; }
	void Deserialize(FMemoryReader& reader) override { reader << *this; }

public:
	uint32 ActionType = ActionTypeEnum::UNDEFINED;
	int axisDimension = 0;
	double X = 0.0f;
	double Y = 0.0f;
	double Z = 0.0f;
};