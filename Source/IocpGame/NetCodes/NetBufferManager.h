// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NetBufferPool.h"

/**
 * 버퍼 풀을 관리한다
 */
class IOCPGAME_API NetBufferManager
{
	enum
	{
		SendBufferSize = 512, // 버퍼 하나 당 바이트 크기
		SendBufferPoolSize = 64, // 버퍼 풀에 몇 개의 버퍼를 가지고 시작할 것인가
		RecvBufferSize = 512,
		RecvBufferPoolSize = 64,
	};

public:
	NetBufferManager();
	~NetBufferManager();

	void Init();

public:
	TUniquePtr<NetBufferPool> SendPool = nullptr;
	TUniquePtr<NetBufferPool> RecvPool = nullptr;
};
