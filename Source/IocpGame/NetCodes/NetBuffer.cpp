// Fill out your copyright notice in the Description page of Project Settings.
#include "NetBuffer.h"

NetBuffer::NetBuffer() : CAPACITY((uint32)(MAX_BUFFER_CAPACITY))
{
}

NetBuffer::NetBuffer(uint32 capacity) : CAPACITY(FMath::Clamp<uint32>(capacity, uint32(MIN_BUFFER_CAPACITY), uint32(MAX_BUFFER_CAPACITY)))
{
	if (capacity > uint32(MAX_BUFFER_CAPACITY) || capacity < uint32(MIN_BUFFER_CAPACITY))
		UE_LOG(LogTemp, Warning, TEXT("요청된 버퍼 Capacity가 기준 범위를 벗어납니다."));
}

NetBuffer::~NetBuffer()
{
}

T_BYTE* NetBuffer::GetBuf()
{
	return Buffer;
}

T_BYTE* NetBuffer::GetData()
{
	return Buffer + sizeof(PacketHeader);
}

T_BYTE* NetBuffer::GetWriteCursor()
{
	return Buffer + BufferSize;
}

const uint32 NetBuffer::GetSize() const
{
	return BufferSize;
}

const uint32 NetBuffer::GetCapacity() const
{
	return CAPACITY;
}

const uint32 NetBuffer::GetLeftover() const
{
	return CAPACITY - BufferSize;
}

void NetBuffer::MoveWriteCursor(int32 bytes)
{
	WriteCursor += bytes;
}

void NetBuffer::SetSize(uint32 size)
{
	BufferSize = size;
}

void NetBuffer::Init()
{
	Buffer = new T_BYTE[CAPACITY];
	Clear(); // 완전히 버퍼를 비운다
}

void NetBuffer::Clear()
{
	memset(Buffer, 0, sizeof(Buffer));
	BufferSize = 0;
}

void NetBuffer::Reset()
{
	Clear();
}

bool NetBuffer::Write(T_BYTE* data, uint32 size)
{
	if (BufferSize + size > CAPACITY)
	{
		UE_LOG(LogTemp, Fatal, TEXT("버퍼 할당 크기를 초과해 Write 할 수 없습니다!"));
		return false;
	}
	memcpy(GetWriteCursor(), data, size);
	BufferSize += size;
	return true;
}


/**
* SendBuffer
*/
void SendBuffer::Reset()
{
	NetBuffer::Reset();
	SetDefaultHeader();
}

void SendBuffer::Init()
{
	NetBuffer::Init();
	SetDefaultHeader();
}

bool SendBuffer::Write(T_BYTE* data, uint32 size)
{
	if (!NetBuffer::Write(data, size)) return false;
	((PacketHeader*)Buffer)->size += size;
	return true;
}

void SendBuffer::SetDefaultHeader()
{
	// 버퍼가 비었다고 가정
	((PacketHeader*)Buffer)->size = sizeof(PacketHeader);

	AIocpGameGameMode* gameMode = nullptr;
	UGameplayStatics::GetGameMode(gameMode);
	if (gameMode)
	{
		((PacketHeader*)Buffer)->senderType = gameMode->GetExecType()->GetHostType();
	}
	((PacketHeader*)Buffer)->id = PacketId::DEFAULT;

	BufferSize = sizeof(PacketHeader); // += 가 아니라 = 임에 유의
}


/**
* RecvBuffer
*/
