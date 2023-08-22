// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Engine.h"
#include "Sockets.h"
#include "Networking.h"
#include "SocketSubsystem.h"

using BYTE = uint8;

/**
 * host 간 정보를 주고 받을 때 사용하는 버퍼 객체이다.
 */
class IOCPGAME_API NetBuffer
{
public:
	NetBuffer();
	~NetBuffer();

	void Init(); // 공간을 할당받는다

	const BYTE* GetBuf() const; // 버퍼 포인터 getter
	const int32 GetCnt() const; // 버퍼의 크기를 반환한다 (몇 바이트인지)
private:
	BYTE* Buffer = nullptr; // 버퍼
};