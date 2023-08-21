// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Engine.h"
#include "Sockets.h"
#include "Networking.h"
#include "SocketSubsystem.h"

/**
 * host 간 정보를 주고 받을 때 사용하는 버퍼 객체이다.
 */
class IOCPGAME_API NetBuffer
{
public:
	NetBuffer();
	~NetBuffer();
};