// Fill out your copyright notice in the Description page of Project Settings.
#include "NetSocket.h"

NetSocket::NetSocket()
{
}

NetSocket::~NetSocket()
{
}

bool NetSocket::InitSocket()
{
	ISocketSubsystem* sockSys = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	if (!sockSys) return false;
	Socket = sockSys->CreateSocket(TEXT("Stream"), TEXT("DefaultSocket"), false);
	if (!Socket) return false;
	Socket->SetNonBlocking(true); // 논블로킹 소켓 사용
	return (Socket != nullptr);
}

bool NetSocket::IsConnected()
{
	return Socket->GetConnectionState() == ESocketConnectionState::SCS_Connected;
}
