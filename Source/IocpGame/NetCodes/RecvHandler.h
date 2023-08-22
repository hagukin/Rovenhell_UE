// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "NetBuffer.h"
#include "NetSession.h"

class NetSession;

/**
 * Session에서의 Recv Runnable 객체
 */
class IOCPGAME_API RecvHandler : public FRunnable
{
public:
	RecvHandler();
	~RecvHandler();

	// 오버라이드
	virtual bool Init();
	virtual uint32 Run();
	virtual void Stop();
	virtual void Exit();

private:
	TQueue<NetBuffer> RecvQueue;
	TSharedPtr<NetSession> Session = nullptr;
};
