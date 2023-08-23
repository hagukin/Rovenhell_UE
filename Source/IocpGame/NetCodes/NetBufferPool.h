// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NetBuffer.h"

/**
 * 
 */
class IOCPGAME_API NetBufferPool
{
public:
	NetBufferPool();
	NetBufferPool(uint32 bufferSize, uint32 poolSize);
	~NetBufferPool();

	TSharedPtr<NetBuffer> CreateNewBuffer(); // 새 버퍼를 동적할당해 반환한다

	void Init();
	void PushBuffer(TSharedPtr<NetBuffer> buffer); // 사용 완료된 버퍼를 반납한다
	TSharedPtr<NetBuffer> PopBuffer(); // 사용하기 위한 버퍼를 가져온다

private:
	///////////////// Critical Sections
	mutable FCriticalSection Lock;
	TQueue<TSharedPtr<NetBuffer>> Pool;
	uint32 CurrentPoolCapacity = 0; // 상황에 따라 초기 POOL_SIZE보다 커질 수 있음. Capacity이므로 실제 큐 내의 size는 이것보다 작을 수 있음.

private:
	const uint32 BUFFER_SIZE;
	const uint32 POOL_SIZE;
};
