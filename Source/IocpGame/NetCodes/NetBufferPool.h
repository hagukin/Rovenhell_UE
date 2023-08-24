// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NetBuffer.h"

/**
 * 
 */
template <typename BufferType>
class IOCPGAME_API NetBufferPool
{
public:
	NetBufferPool() : BUFFER_SIZE(1024), POOL_SIZE(1024) {}
	NetBufferPool(uint32 bufferSize, uint32 poolSize) : BUFFER_SIZE(bufferSize), POOL_SIZE(poolSize) {}
	~NetBufferPool() {}

	TSharedPtr<BufferType> CreateNewBuffer() // 새 버퍼를 동적할당해 반환한다
	{
		TSharedPtr<BufferType> buffer = MakeShared<BufferType>(BUFFER_SIZE);
		buffer->Init();
		return buffer;
	}

	void Init()
	{
		for (uint32 i = 0; i < POOL_SIZE; ++i)
		{
			Pool.Enqueue(CreateNewBuffer());
		}
	}

	void PushBuffer(TSharedPtr<BufferType> buffer) // 사용 완료된 버퍼를 반납한다
	{
		while (!Lock.TryLock());
		buffer->Reset(); // send 버퍼의 경우 default 헤더를 다시 만들어준다
		Pool.Enqueue(buffer);
		Lock.Unlock();
		return;
	}

	TSharedPtr<BufferType> PopBuffer() // 사용하기 위한 버퍼를 가져온다; 만약 여유 버퍼가 부족하면 nullptr를 반환한다
	{
		while (!Lock.TryLock());
		TSharedPtr<BufferType> out = nullptr;
		if (!Pool.Dequeue(out))
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("버퍼 풀에 여유 버퍼가 부족합니다. 패킷 생성/처리 속도와 패킷 송신/수신 속도를 비교해보는 것을 권장합니다.")));
			// 버퍼 풀의 크기를 늘리면 잠시 시간을 벌 수는 있지만 근본적인 송수신과 생성처리 속도의 차이를 극복할 수 없다.
			// 즉 버퍼 풀 고갈이 일어나는 시간을 늦출 뿐이지, 그 자체를 막을 수는 없다.
			// 그러나 게임틱, 서버틱의 속도는 상황에 따라 변할 수 있기 때문에, 버퍼 풀의 크기를 늘리는 것은 일종의 완충재 역할을 해준다.
			// 특히 게임에서 많은 연산이 필요한 시점에서 틱 속도가 느려지고 프레임 드랍이 발생할 경우 충분한 버퍼 풀 크기를 확보하는 것은 더욱 중요하다.
			// 다만, 아무리 버퍼 풀 크기를 늘려도 프레임 저하가 계속 유지되어 큐 pop보다 push 속도가 빨라진다면 결국 어느 시점엔 버퍼 풀은 가득 차게 된다.
		}
		Lock.Unlock();
		return out;
	}

	bool IsEmpty()
	{
		return Pool.IsEmpty();
	}
private:
	///////////////// Critical Sections
	mutable FCriticalSection Lock;
	TQueue<TSharedPtr<BufferType>> Pool;

private:
	const uint32 BUFFER_SIZE;
	const uint32 POOL_SIZE;
};