// Fill out your copyright notice in the Description page of Project Settings.


#include "NetBufferPool.h"

NetBufferPool::NetBufferPool() : BUFFER_SIZE(1024), POOL_SIZE(1024)
{
}

NetBufferPool::NetBufferPool(uint32 bufferSize, uint32 poolSize) : BUFFER_SIZE(bufferSize), POOL_SIZE(poolSize)
{
}

NetBufferPool::~NetBufferPool()
{
}

TSharedPtr<NetBuffer> NetBufferPool::CreateNewBuffer()
{
	TSharedPtr<NetBuffer> buffer = MakeShared<NetBuffer>(BUFFER_SIZE);
	buffer->Init();
	return buffer;
}

void NetBufferPool::Init()
{
	for (uint32 i = 0; i < POOL_SIZE; ++i)
	{
		Pool.Enqueue(CreateNewBuffer());
	}
}

void NetBufferPool::PushBuffer(TSharedPtr<NetBuffer> buffer)
{
	while (!Lock.TryLock());
	buffer->Clear();
	Pool.Enqueue(buffer);
	CurrentPoolCapacity += 1;
	Lock.Unlock();
	return;
}

TSharedPtr<NetBuffer> NetBufferPool::PopBuffer()
{
	while (!Lock.TryLock());
	TSharedPtr<NetBuffer> out = nullptr;
	if (!Pool.Dequeue(out))
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("버퍼에 공간이 부족합니다, 버퍼 풀의 크기를 증가시키는 것을 고려하십시오.")));
		out = CreateNewBuffer();
	}
	Lock.Unlock();
	return out;
}
