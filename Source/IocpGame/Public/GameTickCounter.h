// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class IOCPGAME_API GameTickCounter : public FTickableGameObject
{
public:
	GameTickCounter();
	virtual ~GameTickCounter() {};

public:
	virtual void Tick(float DeltaTime) override;
	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Always; } // 매 프레임 Tick
	TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(MyTickableClass, STATGROUP_Tickables); }
	const uint32 GetTick() { return TickCorrection + TickCount; } // 서버: True game tick 반환; 클라: 로컬 게임 틱 반환
	const uint32 GetTickCount() { return TickCount; } // lock free

private:
	int32 TickCorrection = 0; // 서버와의 틱 싱크를 위해 더해주는 값; NetHandler::Tick에서만 write하므로 lock free
	uint32 TickCount = -1;
};
