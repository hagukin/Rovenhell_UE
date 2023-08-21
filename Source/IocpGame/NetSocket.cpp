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
	Socket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("default"), false);
	return (Socket != nullptr);
}