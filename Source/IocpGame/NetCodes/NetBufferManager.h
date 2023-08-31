// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NetBuffer.h"
#include "NetBufferPool.h"

/**
 * 버퍼 풀을 관리한다
 */
class IOCPGAME_API NetBufferManager
{
	enum
	{
		SendBufferSize = 2048, // 버퍼 하나 당 바이트 크기
		SendBufferPoolSize = 1024, // 버퍼 풀에 몇 개의 버퍼를 가지고 시작할 것인가
		RecvBufferSize = 2048,
		RecvBufferPoolSize = 100000,
	}; // TODO: 클라, 로직서버에 따라 Recv 풀과 Send 풀 중 어느 쪽에 더 많은 자원을 할당할 것인지가 달라짐; 추후 런타임 이전에 실행 타입이 결정된다면 이에 맞게 변경되도록 수정

public:
	NetBufferManager();
	~NetBufferManager();

	void Init();

public:
	TUniquePtr<NetBufferPool<SendBuffer>> SendPool = nullptr;
	TUniquePtr<NetBufferPool<RecvBuffer>> RecvPool = nullptr;
};
