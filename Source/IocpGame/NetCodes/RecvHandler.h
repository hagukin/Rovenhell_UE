// Copyright 2023 Haguk Kim
// Author: Haguk Kim

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
	RecvHandler(HostTypeEnum hostType);
	~RecvHandler();

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
	TMap<uint64, TSharedPtr<TQueue<TSharedPtr<RecvBuffer>>>> PendingClientBuffers; // 클라이언트 id - 처리 대기중인 해당 클라이언트의 패킷

private:
	/* UE SERVER */
	NetSession* Session = nullptr;
	FRunnableThread* Thread = nullptr;
	HostTypeEnum HostType = HostTypeEnum::NONE;
};
