// Fill out your copyright notice in the Description page of Project Settings.
#include "NetBuffer.h"

NetBuffer::NetBuffer() : CAPACITY(1024)
{
}

NetBuffer::NetBuffer(uint32 capacity) : CAPACITY(capacity)
{
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

const uint32 NetBuffer::GetSize() const
{
	return ((PacketHeader*)Buffer)->size;
}

void NetBuffer::Init()
{
	Buffer = new T_BYTE[CAPACITY];
	Clear(); // 헤더 생성 및 커서 이동
}

void NetBuffer::SetDefaultHeader()
{
	// 기본값 정보들
	((PacketHeader*)Buffer)->size = sizeof(PacketHeader);
	((PacketHeader*)Buffer)->id = PacketId::DEFAULT;
	WriteCursor = Buffer + sizeof(PacketHeader);
}

void NetBuffer::Clear()
{
	memset(Buffer, 0, sizeof(Buffer));
	SetDefaultHeader();
}

void NetBuffer::Write(T_BYTE* data, uint32 size)
{
	memcpy(WriteCursor, data, size);
	WriteCursor += size;
	((PacketHeader*)Buffer)->size += size;
}
