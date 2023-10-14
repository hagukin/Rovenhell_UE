// Copyright 2023 Haguk Kim
// Author: Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "Engine.h"
#include "Sockets.h"
#include "Networking.h"
#include "SocketSubsystem.h"
#include "memory.h"
#include "../PacketCodes/PacketHeader.h"
#include "../IocpGameGameMode.h"

#define MIN_BUFFER_CAPACITY sizeof(PacketHeader)
#define MAX_BUFFER_CAPACITY sizeof(PacketHeader) + 4096 // 단일 버퍼가 가질 수 있는 최대 크기

using T_BYTE = uint8; // BYTE 매크로와 중첩 방지를 위함

/**
 * host 간 정보를 주고 받을 때 사용하는 버퍼이다.
 */
class IOCPGAME_API NetBuffer
{
public:
	NetBuffer();
	NetBuffer(uint32 capacity);
	~NetBuffer();

	void Init();
	void Clear(); // 버퍼 데이터를 헤더를 포함해 완전히 비운다
	void Reset(); // 버퍼를 초기 상태로 되돌린다
	bool Write(T_BYTE* data, uint32 size); // 버퍼의 WriteCursor부터 data를 복사해 채운다.

	T_BYTE* GetBuf(); // 헤더를 포함한 버퍼 전체를 반환한다.
	T_BYTE* GetData(); // 헤더가 있다고 가정하고, 헤더를 제외한 데이터를 반환한다.
	T_BYTE* GetWriteCursor(); // 버퍼 데이터의 끝부분을 반환한다
	const uint32 GetSize() const; // 버퍼 유효 크기를 바이트로 반환한다. (sizeof)
	const uint32 GetCapacity() const; // 버퍼 할당 크기를 바이트로 반환한다.
	const uint32 GetLeftover() const; // 버퍼의 남은 공간 크기를 반환한다

	void MoveWriteCursor(int32 bytes); // bytes 바이트 만큼 커서를 뒤로 이동한다.
	void SetSize(uint32 size); // 버퍼 크기가 Write() 이외의 방법으로 외부에서 변경되었을 때 BufferSize를 최신화하기 위해 사용한다. Recv() 이후 사용한다.

	PacketHeader* GetHeader(); // 헤더가 있음을 가정한다
protected:
	const uint32 CAPACITY;
	uint32 BufferSize = 0;
	T_BYTE* Buffer = nullptr; // 버퍼
	T_BYTE* WriteCursor = nullptr; // Write 시작 지점
};


class IOCPGAME_API SendBuffer : public NetBuffer
{
public:
	SendBuffer() {};
	SendBuffer(uint32 capacity) : NetBuffer(capacity) {};
	~SendBuffer() {};
	void Init();
	void Reset();
	bool Write(T_BYTE* data, uint32 size); // 버퍼의 WriteCursor부터 data를 복사해 채운다.

private:
	void SetDefaultHeader(); // 버퍼가 비어있다는 가정 하에 기본 헤더를 버퍼에 추가한다. WriteCursor는 헤더 끝으로 움직인다.
};


class IOCPGAME_API RecvBuffer : public NetBuffer
{
public:
	RecvBuffer() {};
	RecvBuffer(uint32 capacity) : NetBuffer(capacity) {};
	~RecvBuffer() {};
};
