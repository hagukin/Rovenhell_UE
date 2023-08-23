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
	SendPool = MakeUnique<NetBufferPool>(SendBufferSize, SendBufferPoolSize);
	SendPool->Init();
	RecvPool = MakeUnique<NetBufferPool>(RecvBufferSize, RecvBufferPoolSize);
	RecvPool->Init();
}
