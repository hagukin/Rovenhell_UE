// Copyright 2023 Haguk Kim
// Author: Haguk Kim

#include "NetAddress.h"

NetAddress::NetAddress()
{
	_ipAddr = "0.0.0.0";
	_port = 0;
}

NetAddress::~NetAddress()
{
}

FString NetAddress::GetIp()
{
	return _ipAddr;
}

int32 NetAddress::GetPort()
{
	return _port;
}
