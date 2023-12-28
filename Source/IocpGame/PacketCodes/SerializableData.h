// Copyright 2023 Haguk Kim
// Author: Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "Containers/BitArray.h"
#include "Components/PrimitiveComponent.h"
#include "../Enumerations.h"

class APlayerPawn;

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


#define MAX_COMPRESSED_COORDINATE_RANGE_PER_DIRECTION 100000.0f // (cm); 100000일 경우 +-1km 표현 가능함을 의미
#define COMPRESSED_COORDINATE_PRECISION 5.12f // (cm당 등분 개수); e.g. 1m를 512 등분 -> 5.12f
#define COMPRESSED_COORDINATE_BIT_SIZE 20 // log2 (max range / precision)을 ceil한 값에 1 더한 값(부호비트) = 혹은 log2 (max range*2 / precision)을 ceil한 값

class CompressedCoord
{
	/*
	2mm 정밀도 = 1m 당 512 values(2^9)

	X,Y,Z축:
	+-1km 범위 = 2000m 범위 = 512 * 2000 values = log2(1024000) ~= 20 bits
	맵 2km * 2km * 2km 표현 가능

	NOTE: 필요 시 Z축은 더 범위를 좁힐 수 있음
	*/
public:
	CompressedCoord()
	{
		bitArray.Init(false, COMPRESSED_COORDINATE_BIT_SIZE);
	}

	bool CompressCoordinate(float in)
	{
		if (in > MAX_COMPRESSED_COORDINATE_RANGE_PER_DIRECTION || in < -MAX_COMPRESSED_COORDINATE_RANGE_PER_DIRECTION)
		{
			UE_LOG(LogTemp, Error, TEXT("%f: 압축 가능한 좌표 범위를 벗어났습니다!"), in);
			return false;
		}

		uint32 count = (FMath::Abs(in) * COMPRESSED_COORDINATE_PRECISION); // 1000m (100,000cm)일 경우 512,000 저장
		bool msb = (in < -SMALL_NUMBER) ? 1 : 0;

		bitArray[0] = msb;
		for (uint32 i = 1; i < COMPRESSED_COORDINATE_BIT_SIZE; ++i)
		{
			bitArray[i] = count & (1 << (COMPRESSED_COORDINATE_BIT_SIZE - 1 - i)); // bitarray로 복사
		}
		return true;
	}

	float DecompressCoordinate() const
	{
		bool msb = bitArray[0];
		uint32 count = 0;
		for (uint32 i = 1; i < COMPRESSED_COORDINATE_BIT_SIZE; ++i)
		{
			count |= (bitArray[i] << (COMPRESSED_COORDINATE_BIT_SIZE - 1 - i));
		}
		float decompressed = (float)count / COMPRESSED_COORDINATE_PRECISION;
		return decompressed;
	}

	friend FArchive& operator<<(FArchive& Archive, CompressedCoord& Data)
	{
		Archive << Data.bitArray;
		return Archive;
	}

public:
	TBitArray<FDefaultBitArrayAllocator> bitArray;
};

class CompressedLoc
{
public:
	bool CompressLocation(const FVector in)
	{
		bool succeeded = true;
		succeeded &= this->X.CompressCoordinate(in.X);
		succeeded &= this->Y.CompressCoordinate(in.Y);
		succeeded &= this->Z.CompressCoordinate(in.Z);
		return succeeded;
	}

	FVector DecompressLocation() const
	{
		FVector decompressed;
		decompressed.X = X.DecompressCoordinate();
		decompressed.Y = Y.DecompressCoordinate();
		decompressed.Z = Z.DecompressCoordinate();
		return decompressed;
	}

	friend FArchive& operator<<(FArchive& Archive, CompressedLoc& Data)
	{
		Archive << Data.X;
		Archive << Data.Y;
		Archive << Data.Z;
		return Archive;
	}

public:
	CompressedCoord X;
	CompressedCoord Y;
	CompressedCoord Z;
};

class SD_ActorPhysics : SD_Data
{
public:
	void SetDataFromTransform(const FTransform& transform)
	{
		CompressedLocation.CompressLocation(transform.GetLocation());

		const FVector& Vec = transform.GetRotation().Vector();
		RotX = Vec.X;
		RotY = Vec.Y;
		RotZ = Vec.Z;
		return;
	}

	FTransform GetTransformFromData() const
	{
		FTransform transform;
		FVector location = CompressedLocation.DecompressLocation();
		transform.SetLocation(location);
		transform.SetRotation(FVector(RotX, RotY, RotZ).ToOrientationQuat());
		return transform;
	}

	SD_ActorPhysics() {};
	SD_ActorPhysics(AActor* actor)
	{
		SetDataFromTransform(actor->GetTransform());
	}
	SD_ActorPhysics(const FTransform& transform)
	{
		SetDataFromTransform(transform);
	}
	virtual ~SD_ActorPhysics() {}

	friend FArchive& operator<<(FArchive& Archive, SD_ActorPhysics& Data)
	{
		Archive << Data.CompressedLocation;

		Archive << Data.RotX;
		Archive << Data.RotY;
		Archive << Data.RotZ;

		return Archive;
	}

	void Serialize(FMemoryWriter& writer) override { writer << *this; }
	void Deserialize(FMemoryReader& reader) override { reader << *this; }

public:
	CompressedLoc CompressedLocation;
	float RotX, RotY, RotZ;
};


// InputAction과 InputActionValue에 대한 정보를 담는다
class SD_GameInput : SD_Data
{
public:
	FORCEINLINE bool IsNotZero(float x)
	{
		return x > KINDA_SMALL_NUMBER || x < KINDA_SMALL_NUMBER * -1;
	}
	FORCEINLINE bool GetSignBit(float x)
	{
		return x < -KINDA_SMALL_NUMBER;
	}
	SD_GameInput() {}
	SD_GameInput(ActionTypeEnum actionType, const FInputActionValue& inputValue, float deltaTime, float time)
	{
		ActionType = actionType;
		DeltaTime = deltaTime;
		Time = time;
		switch (inputValue.GetValueType())
		{
			case EInputActionValueType::Axis1D:
			{
				X = IsNotZero(inputValue.Get<float>());
				XSign = GetSignBit(inputValue.Get<float>());
				break;
			}
			case EInputActionValueType::Axis2D:
			{
				X = IsNotZero(inputValue.Get<float>());
				XSign = GetSignBit(inputValue.Get<float>());
				Y = IsNotZero(inputValue.Get<FVector2D>().Y);
				YSign = GetSignBit(inputValue.Get<FVector2D>().Y);
				break;
			}
			case EInputActionValueType::Axis3D:
			{
				X = IsNotZero(inputValue.Get<float>());
				XSign = GetSignBit(inputValue.Get<float>());
				Y = IsNotZero(inputValue.Get<FVector2D>().Y);
				YSign = GetSignBit(inputValue.Get<FVector2D>().Y);
				Z = IsNotZero(inputValue.Get<FVector>().Z);
				ZSign = GetSignBit(inputValue.Get<FVector>().Z);
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
		Archive << Data.XSign;
		Archive << Data.YSign;
		Archive << Data.ZSign;
		Archive << Data.DeltaTime;
		Archive << Data.Time;
		return Archive;
	}

	void Serialize(FMemoryWriter& writer) override { writer << *this; }
	void Deserialize(FMemoryReader& reader) override { reader << *this; }

public:
	uint8 ActionType = ActionTypeEnum::UNDEFINED;
	bool X = 0;
	bool Y = 0;
	bool Z = 0;
	bool XSign = 0; // 0: 양수
	bool YSign = 0;
	bool ZSign = 0;
	float DeltaTime = 0.0f; // 서버측 재연산을 위해 필요한 값
	float Time = 0; // 이 인풋이 발생한 시점
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
			for (uint8 i = 0; i < Data.InputCounts; ++i)
			{
				Archive << Data.Temp;
				Data.GameInputs.Add(Data.Temp);
			}
		}
		else if (Archive.IsSaving())
		{
			Data.InputCounts = Data.GameInputs.Num();
			Archive << Data.InputCounts;
			for (uint8 i = 0; i < Data.InputCounts; ++i)
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
	uint8 InputCounts = 0;
	SD_GameInput Temp;

	TArray<SD_GameInput> GameInputs;
};


// 단일 폰의 물리 정보를 나타낸다
class SD_PawnPhysics : public SD_ActorPhysics
{
public:
	SD_PawnPhysics() {}
	virtual ~SD_PawnPhysics() {}
	SD_PawnPhysics(AActor* actor) : SD_ActorPhysics(actor) {}
	SD_PawnPhysics(const FTransform& transform) : SD_ActorPhysics(transform) {}

	friend FArchive& operator<<(FArchive& Archive, SD_PawnPhysics& Data)
	{
		Archive << *(SD_ActorPhysics*)&Data; // 피직스 데이터 전달
		return Archive;
	}
};


// 단일 플레이어에 대한 모든 정보들을 나타낸다
class SD_PlayerState : SD_Data
{
public:
	SD_PlayerState() {}
	virtual ~SD_PlayerState() {}
	SD_PlayerState(uint16 sessionId, APlayerPawn* player);

	friend FArchive& operator<<(FArchive& Archive, SD_PlayerState& Data)
	{
		Archive << Data.SessionId;
		Archive << Data.AnimState;
		Archive << Data.AnimDelta1D;
		Archive << Data.PlayerPhysics;
		return Archive;
	}

	void Serialize(FMemoryWriter& writer) override { writer << *this; }
	void Deserialize(FMemoryReader& reader) override { reader << *this; }

public:
	uint16 SessionId = 0;
	uint8 AnimState = AnimStateEnum::NO_ANIM;
	float AnimDelta1D = 0.0f; // blendspace 싱크를 위해 전송하는 값; 2D blendspace 사용 시 하나 더 추가할 것
	SD_PawnPhysics PlayerPhysics;
};


// 서버가 클라이언트들에게 브로드캐스트하는 모든 정보들을 포함한다
class SD_GameState : SD_Data
{
public:
	SD_GameState() {}
	SD_GameState(AGameStateBase* gameState, float gameStateTimestamp)
	{
		GameStateTimestamp = gameStateTimestamp;
		// NOTE: 콘텐츠 제작 시 동기화해야 하는 GameState 데이터들을 여기에서 복사해 저장
		// 현재는 콘텐츠가 없는 관계로 비어있음

	}
	virtual ~SD_GameState() {}

	friend FArchive& operator<<(FArchive& Archive, SD_GameState& Data)
	{
		Archive << Data.GameStateTimestamp;

		if (Archive.IsLoading())
		{
			// 스테이트
			Archive << Data.UpdatedPlayerStatesCount;
			for (uint16 i = 0; i < Data.UpdatedPlayerStatesCount; ++i)
			{
				Archive << Data.TempState;
				Data.UpdatedPlayerStates.Add(Data.TempState);
			}
		}
		else if (Archive.IsSaving())
		{
			// 스테이트
			Data.UpdatedPlayerStatesCount = Data.UpdatedPlayerStates.Num();
			Archive << Data.UpdatedPlayerStatesCount;
			for (uint16 i = 0; i < Data.UpdatedPlayerStatesCount; ++i)
			{
				Archive << Data.UpdatedPlayerStates[i];
			}
		}

		return Archive;
	}

	void AddPlayerStates(SD_PlayerState* playerState)
	{
		UpdatedPlayerStates.Add(*playerState);
		UpdatedPlayerStatesCount++;
	}

	void Serialize(FMemoryWriter& writer) override { writer << *this; }
	void Deserialize(FMemoryReader& reader) override { reader << *this; }
public:
	uint16 UpdatedPlayerStatesCount = 0;
	TArray<SD_PlayerState> UpdatedPlayerStates; // 접속한 모든 플레이어들의 State; 현재 접속한 플레이어 정보를 동기화하기 위해서도 사용됨.
	SD_PlayerState TempState;
	// TODO: 플레이어 외의 액터들도 피직스 정보 싱크 맞도록 고유 id 부여

	float GameStateTimestamp = 0.0f; // 이 게임스테이트 패킷이 담고있는 시점 (서버)
	uint32 ProcessedTicksCount = 0;
};