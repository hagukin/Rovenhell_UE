// Fill out your copyright notice in the Description page of Project Settings.


#include "NetBufferManager.h"

NetBufferManager::NetBufferManager()
{
}

NetBufferManager::~NetBufferManager()
{
}

void NetBufferManager::Init()
{
	SendPool = MakeUnique<NetBufferPool<SendBuffer>>(SendBufferSize, SendBufferPoolSize);
	SendPool->Init();
	RecvPool = MakeUnique<NetBufferPool<RecvBuffer>>(RecvBufferSize, RecvBufferPoolSize);
	RecvPool->Init();
}
