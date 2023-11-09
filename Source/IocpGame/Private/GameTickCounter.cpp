// Copyright 2023 Haguk Kim
// Author: Haguk Kim

#include "GameTickCounter.h"
#include "RovenhellGameInstance.h"

GameTickCounter::GameTickCounter()
{
}

void GameTickCounter::Tick(float DeltaTime)
{
	lastDelta = DeltaTime;
	if (TotalTickCount == GFrameCounter) // 한 프레임 당 여러 번 호출 방지
		return;
	TotalTickCount = (uint32)GFrameCounter;
}

const uint32 GameTickCounter::GetTick()
{
	return TotalTickCount; // 서버의 경우 ServerTick이 0이므로 그냥 LocalTickPassed를 반환
}

const float GameTickCounter::GetTime(UWorld* world)
{
	URovenhellGameInstance* gameInstance = Cast<URovenhellGameInstance>(world->GetGameInstance());
	if (!gameInstance)
	{
		UE_LOG(LogTemp, Fatal, TEXT("GameInstance를 찾을 수 없습니다!"));
		return 0.0f;
	}

	switch (gameInstance->GetExecType()->GetHostType())
	{
		case HostTypeEnum::CLIENT:
		case HostTypeEnum::CLIENT_HEADLESS:
		{
			return GetTime_UEClient(world);
		}
		case HostTypeEnum::LOGIC_SERVER:
		case HostTypeEnum::LOGIC_SERVER_HEADLESS:
		{
			return GetTime_UEServer(world);
		}
	}
	UE_LOG(LogTemp, Fatal, TEXT("알 수 없는 HostTypeEnum입니다!"));
	return 0.0f;
}

const float GameTickCounter::GetTime_UEClient(UWorld* world)
{
	return LastServerTimestamp + (world->GetRealTimeSeconds() - LastClientTimestamp) + PacketTripTime;
}

const float GameTickCounter::GetAdjustedTime_UEClient(UWorld* world, float adjustAmount)
{
	return LastServerTimestamp + (world->GetRealTimeSeconds() - LastClientTimestamp) + FMath::Max(adjustAmount, PacketTripTime);
}

void GameTickCounter::SetServerTime_UEClient(UWorld* world, float time)
{
	float currentHostTime = world->GetRealTimeSeconds();
	if (!CanGetTripTime) // 첫 패킷 수신
	{
		CanGetTripTime = true;
	}
	else
	{
		PacketTripTime = GetTime_UEClient(world) - time; // 호스트 시간과 패킷에 기재된 시간의 차이로 설정
	}
	LastServerTimestamp = time;
	LastClientTimestamp = currentHostTime;
}

const float GameTickCounter::GetTime_UEServer(UWorld* world)
{
	return world->GetRealTimeSeconds();
}
