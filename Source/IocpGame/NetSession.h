// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Engine.h"
#include "Sockets.h"
#include "Networking.h"
#include "SocketSubsystem.h"
#include "NetSocket.h"
#include "NetBuffer.h"
#include "NetAddress.h"

/**
 * 이 host와 다른 host 하나와의 연결을 관리한다.
 */
class IOCPGAME_API NetSession
{
public:
	NetSession();
	~NetSession();

	void Send(NetBuffer sendBuffer);
	void Recv();
	bool Connect(NetAddress connectAddr);
	void Disconnect();

	const NetAddress& GetPeerAddr();

private:
	NetSocket NetSock;
	TQueue<NetBuffer> SendPendingQueue;
	TQueue<NetBuffer> RecvQueue; // TODO: 락 필요 혹은 Lock free queue 사용
	NetAddress _peerAddr;
};


/**
 * 로직 서버(언리얼)와 메인 서버의 연결을 관리한다.
 */
class IOCPGAME_API ServerToServerSession
{
};


/**
 * 클라이언트와 메인 서버의 연결을 관리한다.
 */
class IOCPGAME_API ClientToServerSession
{
};
