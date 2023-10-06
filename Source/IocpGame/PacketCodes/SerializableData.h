// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "Components/PrimitiveComponent.h"
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
	SD_ActorPhysics(AActor* actor)
	{
		Transform = actor->GetTransform();
		XVelocity = actor->GetRootComponent()->ComponentVelocity.X;
		YVelocity = actor->GetRootComponent()->ComponentVelocity.Y;
		ZVelocity = actor->GetRootComponent()->ComponentVelocity.Z;
	}
	SD_ActorPhysics(const FTransform& transform, const FVector& velocity)
	{
		Transform = transform;
		XVelocity = velocity.X;
		YVelocity = velocity.Y;
		ZVelocity = velocity.Z;
	}

	friend FArchive& operator<<(FArchive& Archive, SD_ActorPhysics& Data)
	{
		Archive << Data.Transform;
		Archive << Data.XVelocity;
		Archive << Data.YVelocity;
		Archive << Data.ZVelocity;

		return Archive;
	}

	void Serialize(FMemoryWriter& writer) override { writer << *this; }
	void Deserialize(FMemoryReader& reader) override { reader << *this; }

public:
	FTransform Transform;
	double XVelocity = 0.f; // 대상 액터의 RootComponent의 ComponentVelocity 사용; LinearVelocity;
	double YVelocity = 0.f;
	double ZVelocity = 0.f;
};

// InputAction과 InputActionValue에 대한 정보를 담는다
class SD_GameInput : SD_Data
{
public:
	SD_GameInput() {}
	SD_GameInput(ActionTypeEnum actionType, const FInputActionValue& inputValue, float deltaTime, uint32 tick)
	{
		ActionType = actionType;
		DeltaTime = deltaTime;
		Tick = tick;
		switch (inputValue.GetValueType())
		{
			case EInputActionValueType::Axis1D:
				{
					X = inputValue.Get<float>();
					break;
				}
			case EInputActionValueType::Axis2D:
				{
					X = inputValue.Get<FVector2D>().X;
					Y = inputValue.Get<FVector2D>().Y;
					break;
				}
			case EInputActionValueType::Axis3D:
				{
					X = inputValue.Get<FVector>().X;
					Y = inputValue.Get<FVector>().Y;
					Z = inputValue.Get<FVector>().Z;
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
		Archive << Data.X;
		Archive << Data.Y;
		Archive << Data.Z;
		Archive << Data.DeltaTime;
		Archive << Data.Tick;
		return Archive;
	}

	void Serialize(FMemoryWriter& writer) override { writer << *this; }
	void Deserialize(FMemoryReader& reader) override { reader << *this; }

public:
	uint8 ActionType = ActionTypeEnum::UNDEFINED;
	double X = 0.0f;
	double Y = 0.0f;
	double Z = 0.0f;
	float DeltaTime = 0.0f; // 서버측 재연산을 위해 필요한 값
	uint32 Tick = 0; // 이 인풋이 발생한 시점; 인풋을 발생시킨 호스트의 로컬 틱값으로 표현한다.
};


// 단위시간 동안 처리된 모든 플레이어 "게임플레이" 인풋을 저장하기 위해 사용할 수 있다
// TODO: 현재는 플레이어 한 명이기 때문에 GameInputHistory도 하나만 발송한다
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


// 단일 플레이어의 물리 정보를 나타낸다
class SD_PawnPhysics : public SD_ActorPhysics
{
public:
	SD_PawnPhysics() {}
	virtual ~SD_PawnPhysics() {}
	SD_PawnPhysics(uint64 sessionId, AActor* actor) : SD_ActorPhysics(actor) { SessionId = sessionId; }
	SD_PawnPhysics(uint64 sessionId, const FTransform& transform, const FVector& velocity) : SD_ActorPhysics(transform, velocity)
	{ 
		SessionId = sessionId; 
	}

	friend FArchive& operator<<(FArchive& Archive, SD_PawnPhysics& Data)
	{
		Archive << *(SD_ActorPhysics*)&Data; // 피직스 데이터 전달
		Archive << Data.SessionId;
		return Archive;
	}

public:
	uint64 SessionId = 0;
};


// 단일 플레이어의 PlayerState를 나타낸다
class SD_PlayerState : SD_Data
{
public:
	SD_PlayerState() {}
	virtual ~SD_PlayerState() {}
	SD_PlayerState(uint64 sessionId, const AActor& actor) 
	{
		SessionId = sessionId;
		// TODO: Hp 등 각종 정보 전달
	}

	friend FArchive& operator<<(FArchive& Archive, SD_PlayerState& Data)
	{
		Archive << Data.SessionId;
		return Archive;
	}

	void Serialize(FMemoryWriter& writer) override { writer << *this; }
	void Deserialize(FMemoryReader& reader) override { reader << *this; }

public:
	uint64 SessionId = 0;
};


// 서버가 클라이언트들에게 브로드캐스트하는 모든 정보들을 포함한다
class SD_GameState : SD_Data
{
public:
	SD_GameState() {}
	SD_GameState(AGameStateBase* gameState, uint32 tick, float deltaTime, const TMap<uint64, uint32>& processedTicks)
	{
		Tick = tick;
		DeltaTime = deltaTime;
		ProcessedTicks = processedTicks;

		// NOTE: 콘텐츠 제작 시 동기화해야 하는 GameState 데이터들을 여기에서 복사해 저장
		// 현재는 콘텐츠가 없는 관계로 비어있음

	}
	virtual ~SD_GameState() {}

	friend FArchive& operator<<(FArchive& Archive, SD_GameState& Data)
	{
		Archive << Data.Tick;
		Archive << Data.DeltaTime;

		if (Archive.IsLoading())
		{
			// 피직스
			Archive << Data.UpdatedPlayerPhysicsCount;
			for (uint32 i = 0; i < Data.UpdatedPlayerPhysicsCount; ++i)
			{
				Archive << Data.TempPhysics;
				Data.UpdatedPlayerPhysics.Add(Data.TempPhysics);
			}

			// 스테이트
			Archive << Data.UpdatedPlayerStatesCount;
			for (uint32 i = 0; i < Data.UpdatedPlayerStatesCount; ++i)
			{
				Archive << Data.TempState;
				Data.UpdatedPlayerStates.Add(Data.TempState);
			}

			// ProcessedTicks
			uint64 tempKey;
			uint32 tempValue;
			Archive << Data.ProcessedTicksCount;
			for (uint32 i = 0; i < Data.ProcessedTicksCount; ++i)
			{
				Archive << tempKey;
				Archive << tempValue;
				Data.ProcessedTicks.Add(tempKey, tempValue);
			}
		}
		else if (Archive.IsSaving())
		{
			// 피직스
			Data.UpdatedPlayerPhysicsCount = Data.UpdatedPlayerPhysics.Num();
			Archive << Data.UpdatedPlayerPhysicsCount;
			for (uint32 i = 0; i < Data.UpdatedPlayerPhysicsCount; ++i)
			{
				Archive << Data.UpdatedPlayerPhysics[i];
			}

			// 스테이트
			Data.UpdatedPlayerStatesCount = Data.UpdatedPlayerStates.Num();
			Archive << Data.UpdatedPlayerStatesCount;
			for (uint32 i = 0; i < Data.UpdatedPlayerStatesCount; ++i)
			{
				Archive << Data.UpdatedPlayerStates[i];
			}

			// ProcessedTicks
			Data.ProcessedTicksCount = Data.ProcessedTicks.Num();
			Archive << Data.ProcessedTicksCount;
			for (auto& element : Data.ProcessedTicks)
			{
				Archive << element.Key;
				Archive << element.Value;
			}
		}

		return Archive;
	}

	void AddPlayerPhysics(SD_PawnPhysics* playerPhysics)
	{
		UpdatedPlayerPhysics.Add(*playerPhysics);
		UpdatedPlayerPhysicsCount++;
	}

	void AddPlayerStates(SD_PlayerState* playerState)
	{
		UpdatedPlayerStates.Add(*playerState);
		UpdatedPlayerStatesCount++;
	}

	void Serialize(FMemoryWriter& writer) override { writer << *this; }
	void Deserialize(FMemoryReader& reader) override { reader << *this; }
public:
	uint32 UpdatedPlayerPhysicsCount = 0;
	TArray<SD_PawnPhysics> UpdatedPlayerPhysics; // 접속한 모든 플레이어들의 물리 값; 현재 접속한 모든 플레이어들을 클라이언트와 동기화하기 위해서도 사용됨. 이는 패킷 크기 및 발송빈도를 줄이기 위함
	SD_PawnPhysics TempPhysics;

	uint32 UpdatedPlayerStatesCount = 0;
	TArray<SD_PlayerState> UpdatedPlayerStates; // State정보 갱신이 필요한 플레이어들의 State 값
	SD_PlayerState TempState;
	// TODO: 플레이어 id가 중복해서 들어가지 않게 피직스와 스테이트를 합치고, 대신 플레이어 접속 싱크를 맞추는 용도로 접속 플레이어 아이디를 담은 별도의 어레이를 만들어주면 지금보다 최적화 가능
	// TODO: 플레이어 외의 액터들도 피직스 정보 싱크 맞도록 고유 id 부여

	uint32 Tick = 0; // 서버 틱 (Real tick)
	float DeltaTime = 0.0f;

	uint32 ProcessedTicksCount = 0;
	TMap<uint64, uint32> ProcessedTicks; // 클라가 발송한 인풋들 중 몇틱까지 처리를 완료했는지; 이때 틱 기준은 발송한 클라이언트의 로컬 틱이 기준이다
};