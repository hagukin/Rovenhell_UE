// Copyright 2023 Haguk Kim
// Author: Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "Engine.h"
#include "Sockets.h"
#include "Networking.h"
#include "SocketSubsystem.h"

/**
 * 
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
	const bool& isDummy() { return _isDummy; }

private:
	FString _ipAddr;
	int32 _port;
	bool _isDummy = true; // 아이피 또는 포트를 부여받았는지 여부. 둘 중 하나만 부여받아도 Dummy가 아니다.
};
