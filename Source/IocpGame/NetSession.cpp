// Fill out your copyright notice in the Description page of Project Settings.
#include "NetSession.h"

NetSession::NetSession()
{
	if (NetSock.InitSocket())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("NetHandler: 소켓 생성 성공")));
	}
}

NetSession::~NetSession()
{
}

const NetAddress& NetSession::GetPeerAddr()
{
	// 이 소켓과 연결되어있는 상대방의 주소와 포트를 반환한다.
	// 사용 전 소켓의 Connection 여부 확인이 필요하다.
	return _peerAddr;
}

void NetSession::Send(NetBuffer sendBuffer)
{
}

void NetSession::Recv()
{
}

bool NetSession::Connect(NetAddress connectAddr)
{
	if (!NetSock.IsValid()) return false;

	TSharedRef<FInternetAddr> addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	FIPv4Address ip;
	FIPv4Address::Parse(connectAddr.GetIp(), ip);
	addr->SetIp(ip.Value);
	addr->SetPort(connectAddr.GetPort());

	bool ret = NetSock.GetSocket()->Connect(*addr);
	if (ret)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("NetSession: 소켓 연결 성공 %s:%i"), *connectAddr.GetIp(), connectAddr.GetPort()));
		_peerAddr = connectAddr;
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("NetSession: 소켓 연결 실패 %s:%i"), *connectAddr.GetIp(), connectAddr.GetPort()));
	}
	return ret;
}

void NetSession::Disconnect()
{
}