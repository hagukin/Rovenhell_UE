// Copyright 2023 Haguk Kim
// Author: Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#include "NetBuffer.h"
#include "NetSession.h"

class NetSession;

/**
 * Session에서의 Send Runnable 객체
 */
class IOCPGAME_API SendHandler : public FRunnable
{
public:
	SendHandler(HostTypeEnum hostType);
	~SendHandler();

	// 오버라이드 함수들, FRunnableThread에서 제어하므로 절대 임의로 직접 호출하지 말 것
	virtual bool Init(); // true를 반환해야 스레드가 동작
	virtual uint32 Run(); // 스레드 로직
	virtual void Stop(); // Run과 별도의 스레드에서 동작, FRunnableThread::Kill()에서 실행
	virtual void Exit(); // Run과 별도의 스레드에서 동작, Run 정지 후 실행

	FRunnableThread* GetThread();
	bool CreateThread();
	void SetSession(NetSession* session);

public:
	FThreadSafeCounter StopCounter; // Notifier; 0이 아닐 경우 스레드 작동 정지
	mutable FCriticalSection Lock;
	TQueue<TSharedPtr<SendBuffer>> SendQueue; // Pop: Send 스레드; Push: 게임 스레드

private:
	NetSession* Session = nullptr;
	TSharedPtr<SendBuffer> SendPending = nullptr; // 현재 발송중인 버퍼
	FRunnableThread* Thread = nullptr;
	HostTypeEnum HostType = HostTypeEnum::NONE;
};
