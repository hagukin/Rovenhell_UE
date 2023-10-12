// Copyright 2023 Haguk Kim
// Author: Haguk Kim

#include "GameTickCounter.h"

GameTickCounter::GameTickCounter()
{
}

void GameTickCounter::Tick(float DeltaTime)
{
	lastDelta = DeltaTime;
	if (TotalTickCount == GFrameCounter) // 한 프레임 당 여러 번 호출 방지
		return;
	TotalTickCount = (uint32)GFrameCounter;

	LocalTickPassed = TotalTickCount - LastUETick;
}

const uint32 GameTickCounter::GetTick()
{
	return ServerTick + LocalTickPassed; // 서버의 경우 ServerTick이 0이므로 그냥 LocalTickPassed를 반환
}

void GameTickCounter::SetServerTick_UEClient(uint32 tick)
{
	// 반드시 클라이언트에서만 호출해야 함
	LastUETick = (uint32)GFrameCounter;
	ServerTick = tick;
	LocalTickPassed = 0;
}

