// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Engine.h"
#include "Sockets.h"
#include "Networking.h"
#include "SocketSubsystem.h"
#include "memory.h"

using T_BYTE = uint8; // BYTE 매크로와 중첩 방지를 위함

struct PacketHeader
{
	uint32 size;
	uint32 id;
};

enum PacketId
{
	DEFAULT,
	TEST_PACKET
};

/**
 * host 간 정보를 주고 받을 때 사용하는 버퍼 객체이다.
 */
class IOCPGAME_API NetBuffer
{

public:
	NetBuffer();
	NetBuffer(uint32 capacity);
	~NetBuffer();

	void Init();
	void SetDefaultHeader(); // 기본 헤더를 버퍼에 추가한다. 기존 헤더 정보를 기본값으로 바꾸기 위해 사용할 수 있다. WriteCursor는 헤더 끝으로 움직인다.
	void Clear(); // 버퍼 데이터를 0으로 만들고 헤더 정보를 초기화한다. 커서를 헤더 옆으로 이동시킨다.
	void Write(T_BYTE* data, uint32 size); // 버퍼의 WriteCursor부터 data를 복사해 채운다.

	T_BYTE* GetBuf(); // 헤더를 포함한 버퍼 전체를 반환한다.
	T_BYTE* GetData(); // 헤더를 제외한 데이터를 반환한다.
	const uint32 GetSize() const; // 버퍼 할당 크기를 바이트로 반환한다. (sizeof)
private:
	const int CAPACITY;
	T_BYTE* Buffer = nullptr; // 버퍼
	T_BYTE* WriteCursor = nullptr; // Write 시작 지점
};