// Fill out your copyright notice in the Description page of Project Settings.
#include "NetBuffer.h"

NetBuffer::NetBuffer()
{
}

NetBuffer::~NetBuffer()
{
}

const BYTE* NetBuffer::GetBuf() const
{
	return Buffer;
}

const int32 NetBuffer::GetCnt() const
{
	return sizeof(Buffer);
}

void NetBuffer::Init()
{
}
