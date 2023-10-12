// Copyright 2023 Haguk Kim
// Author: Haguk Kim

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
