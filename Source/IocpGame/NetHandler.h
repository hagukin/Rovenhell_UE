// Fill out your copyright notice in the Description page of Project Settings.
#pragma once
#include "CoreMinimal.h"
#include "Engine.h"
#include "Sockets.h"
#include "Networking.h"
#include "SocketSubsystem.h"

/**
 *
 */
class IOCPGAME_API NetBuffer
{
public:
	NetBuffer();
	~NetBuffer();
};

/**
 * 하나의 네트워크 주소를 나타내기 위해 사용할 수 있는 객체이다.
 */
class IOCPGAME_API NetAddress
{
public:
	NetAddress();
	NetAddress(FString address, int32 port) : _ipAddr(address), _port(port) { _isDummy = false; };
	~NetAddress();

	FString GetIp();
	int32 GetPort();

	void SetIp(const FString& s) { _isDummy = false; _ipAddr = s; }
	void SetPort(const int32& n) { _isDummy = false; _port = n; }

private:
	FString _ipAddr;
	int32 _port;
	bool _isDummy = true; // 아이피 또는 포트를 부여받았는지 여부. 둘 중 하나만 부여받아도 Dummy가 아니다.
};


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

	FSocket* const GetSocket() { return Socket; }

private:
	FSocket* Socket = nullptr;
};


/**
 * 이 host와 다른 host 하나와의 연결을 관리한다.
 * 기본적으로 딱 두 개의 스레드만을 사용하는데,
 * 하나는 Recv를 처리하고 하나는 Send를 처리한다.
 * NOTE: 추후 개선이 필요할 경우 Worker 스레드풀을 사용하는 등의 방식을 채택할 수도 있다
 *
 * Recv한 값들을 꺼내 사용하는 로직은 NetSession 바깥의 별도 스레드에서 처리한다.
 * Send할 값들을 추가하는 로직 역시 NetSession 바깥의 별도 스레드에서 처리한다.
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


/**
 * 
 */
class IOCPGAME_API NetHandler
{
public:
	NetHandler();
	~NetHandler();
};