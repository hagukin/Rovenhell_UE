// Copyright 2023 Haguk Kim
// Author: Haguk Kim

#include "NetSocket.h"

NetSocket::NetSocket()
{
}

NetSocket::~NetSocket()
{
	if (Socket)
	{
		Socket->Close();
		delete Socket;
	}
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
	if (!IsValid()) return false;
	return Socket->GetConnectionState() == ESocketConnectionState::SCS_Connected;
}
