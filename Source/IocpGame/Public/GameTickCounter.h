// Copyright 2023 Haguk Kim
// Author: Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "../Enumerations.h"

class IOCPGAME_API GameTickCounter : public FTickableGameObject
{
public:
	GameTickCounter();
	virtual ~GameTickCounter() {};

public:
	virtual void Tick(float DeltaTime) override;
	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Always; } // 매 프레임 Tick
	TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(MyTickableClass, STATGROUP_Tickables); }

	const uint32 GetTick(); // 로컬 틱 반환; 주의: 서버와 동기화된 값이 아님, 동기화된 시간이 필요하다면 GetTime을 이용할 것
	const float GetDelta() { return lastDelta; }
	
	const int32 ConvertTimeToInt(const float time) { return (int32)(time * 100); } // 0.01초 단위 정밀도로 시간을 정수로 변환
	const float GetTime(UWorld* world);
	const int32 GetTimeInt(UWorld* world) { return ConvertTimeToInt(GetTime(world)); }
	const float GetPacketTripTime() { return PacketTripTime; }

	/*UEClient*/
	const float GetTime_UEClient(UWorld* world); // 호출 시점의 net synced time 추측 값을 반환
	const float GetAdjustedTime_UEClient(UWorld* world, float adjustAmount); // 오차를 최소화하면서 서버 시간보다 adjustAmount 이상 앞서있음이 보장되는 시간을 반환
	void SetServerTime_UEClient(UWorld* world, float time);

	/*UEServer*/
	const float GetTime_UEServer(UWorld* world);
public:
	float lastDelta = 0.0f;

private:
	uint32 TotalTickCount = 0;

	/*UEClient*/
	float LocalTimePassed = 0.0f; // 마지막 서버 시간 수신 이후 경과한 시간
	float LastServerTimestamp = 0.0f; // 마지막으로 수신한 서버 시간; 0.0f일 경우 아직 한 번도 수신받지 않았음을 의미함
	float LastClientTimestamp = 0.0f; // 마지막으로 서버 시간을 수신했을 때의 클라이언트 시간
	float PacketTripTime = 0.0f; // 서버에서 클라이언트까지 패킷이 도달하는데 걸리는 시간의 추측값
	bool CanGetTripTime = false; // 서버로부터 최소 1회 패킷을 수신받았는지 여부
};
