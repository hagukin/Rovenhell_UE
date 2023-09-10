// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

const float DESIRED_SERVER_BROADCAST_TIME = 0.2; // 200ms
const float DESIRED_DELTATIME = 0.0166;

class IOCPGAME_API GameTickCounter : public FTickableGameObject
{
public:
	GameTickCounter();
	virtual ~GameTickCounter() {};

public:
	virtual void Tick(float DeltaTime) override;
	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Always; } // 매 프레임 Tick
	TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(MyTickableClass, STATGROUP_Tickables); }

	const uint32 GetTick(); // 서버: True game tick 반환; 클라: 로컬 게임 틱 카운트 반환
	const uint32 GetServerTick() { return ServerTick; }
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
