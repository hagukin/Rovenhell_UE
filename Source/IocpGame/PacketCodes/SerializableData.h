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


	///////////// TESTING FIXME TODO
	// 추후 서버에서 GameState를 발송하면 그 패킷 내부로 이동해야함
	uint32 tick = 0;
	float deltaTime = 0.0f; // 마찬가지
};


// InputAction과 InputActionValue에 대한 정보를 담는다
class SD_GameInput : SD_Data
{
public:
	SD_GameInput() {}
	SD_GameInput(ActionTypeEnum actionType, const FInputActionValue& inputValue, float deltaTime)
	{
		ActionType = actionType;
		DeltaTime = deltaTime;
		switch (inputValue.GetValueType())
		{
			case EInputActionValueType::Axis1D:
				{
					X = inputValue.Get<float>();
					AxisDimension = 1;
					break;
				}
			case EInputActionValueType::Axis2D:
				{
					X = inputValue.Get<FVector2D>().X;
					Y = inputValue.Get<FVector2D>().Y;
					AxisDimension = 2;
					break;
				}
			case EInputActionValueType::Axis3D:
				{
					X = inputValue.Get<FVector>().X;
					Y = inputValue.Get<FVector>().Y;
					Z = inputValue.Get<FVector>().Z;
					AxisDimension = 3;
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
		Archive << Data.AxisDimension;
		Archive << Data.X;
		Archive << Data.Y;
		Archive << Data.Z;
		Archive << Data.DeltaTime;
		return Archive;
	}

	void Serialize(FMemoryWriter& writer) override { writer << *this; }
	void Deserialize(FMemoryReader& reader) override { reader << *this; }

public:
	uint32 ActionType = ActionTypeEnum::UNDEFINED;
	int AxisDimension = 0;
	double X = 0.0f;
	double Y = 0.0f;
	double Z = 0.0f;
	float DeltaTime = 0.0f; // 서버측 재연산을 위해 필요한 값
};


// 단위시간 동안 처리된 모든 플레이어 "게임플레이" 인풋을 저장하기 위해 사용할 수 있다
class SD_GameInputHistory : SD_Data
{
public:
	SD_GameInputHistory() {}
	SD_GameInputHistory(TSharedPtr<TArray<SD_GameInput>> gameInputs) { GameInputs = *gameInputs; }
	virtual ~SD_GameInputHistory() {}

	friend FArchive& operator<<(FArchive& Archive, SD_GameInputHistory& Data)
	{
		if (Archive.IsLoading())
		{
			Archive << Data.InputCounts;
			for (int32 i = 0; i < Data.InputCounts; ++i)
			{
				Archive << Data.Temp;
				Data.GameInputs.Add(Data.Temp);
			}
		}
		else if (Archive.IsSaving())
		{
			Data.InputCounts = Data.GameInputs.Num();
			Archive << Data.InputCounts;
			for (int32 i = 0; i < Data.InputCounts; ++i)
			{
				Archive << Data.GameInputs[i];
			}
		}
		return Archive;
	}

	void Serialize(FMemoryWriter& writer) override { writer << *this; }
	void Deserialize(FMemoryReader& reader) override { reader << *this; }

public:
	// 역직렬화 시 사용
	int32 InputCounts = 0;
	SD_GameInput Temp;

	TArray<SD_GameInput> GameInputs;
};