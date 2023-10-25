// Copyright 2023 Haguk Kim
// Author: Haguk Kim

#pragma once

#include "CoreMinimal.h"

const float DESIRED_DELTATIME = 0.0166;
const float CONSUME_HISTORY_BUFFER_CYCLE = 0.1f; // 클라이언트 패킷 발송 주기 (sec)
// 주기가 너무 길 경우 인풋이 히스토리 버퍼 공간을 완전히 채워버릴 수 있으니 주의
// 플레이어 인풋은 게임 틱 당 1회 받을 수 있기 때문에 0.016f의 배수보다 약간 더 큰 수로 설정하는 게 이상적이다
// 단 제 때 연산결과를 수신받기 위해 서버 브로드캐스트 주기보다는 작은 것이 좋다


class IOCPGAME_API GameTickCounter : public FTickableGameObject
{
public:
	GameTickCounter();
	virtual ~GameTickCounter() {};

public:
	virtual void Tick(float DeltaTime) override;
	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Always; } // 매 프레임 Tick
	TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(MyTickableClass, STATGROUP_Tickables); }

	const uint32 GetTick(); // 서버: True game tick 반환; 클라: 서버 틱과 동기화된 Fake game tick 반환
	const uint32 GetLocalTick() { return TotalTickCount; } // 이 호스트에서 몇 틱이 흘렀는지를 반환
	const float GetDelta() { return lastDelta; }
	void SetServerTick_UEClient(uint32 tick);

public:
	float lastDelta = 0.0f;

private:
	uint32 TotalTickCount = 0; // 로컬 총 이벤트 틱 수 (GFrameCounter)
	uint32 LastUETick = 0; // 마지막 서버 틱 수신 시점에서의 언리얼 게임 틱 번호
	uint32 ServerTick = 0; // 마지막으로 서버로부터 수신한 게임 틱
	uint32 LocalTickPassed = 0; // 마지막 서버 틱 수신 시점 이후 경과한 로컬 틱
};
