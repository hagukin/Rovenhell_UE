// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Engine.h"
#include "Sockets.h"
#include "Networking.h"
#include "SocketSubsystem.h"

/**
 * 외부와 통신할 때 사용하는 소켓 객체를 나타낸다.
 * 클라, 서버 여부에 관계없이 모두 NetSocket 객체를 사용한다.
 */
class IOCPGAME_API NetSocket
{
public:
	NetSocket();
	~NetSocket();

	bool InitSocket();
	bool IsValid() { return (Socket != nullptr); }
	bool IsConnected();

	FSocket* const GetSocket() { return Socket; }

private:
	FSocket* Socket;
};
