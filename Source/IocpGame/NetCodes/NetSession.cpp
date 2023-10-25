// Copyright 2023 Haguk Kim
// Author: Haguk Kim

#include "NetSession.h"

NetSession::NetSession()
{
}

NetSession::~NetSession()
{
}

bool NetSession::Init(HostTypeEnum hostType)
{
	HostType = hostType;

	BufManager = MakeUnique<NetBufferManager>();
	BufManager->Init();

	NetSock = MakeUnique<NetSocket>();
	if (NetSock->InitSocket())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("소켓 생성 성공")));

		Sender = MakeUnique<SendHandler>(HostType);
		Sender->Init();
		Sender->SetSession(this);

		Receiver = MakeUnique<RecvHandler>(HostType);
		Receiver->Init();
		Receiver->SetSession(this);
		return true;
	}
	return false;
}

bool NetSession::Start()
{
	bool ret = false;
	if (NetSock->IsValid() && NetSock->IsConnected())
	{
		ret = true;
		// SendHandler와 RecvHandler 스레드를 가동한다.
		ret &= StartSender();
		ret &= StartReceiver();
	}
	return ret;
}

bool NetSession::StartSender()
{
	return Sender->CreateThread();
}

bool NetSession::StartReceiver()
{
	return Receiver->CreateThread();
}

bool NetSession::Kill()
{
	bool bKilled = true;
	if (Sender) bKilled &= KillSend();
	if (Receiver) bKilled &= KillRecv();
	return bKilled;
}

bool NetSession::KillSend()
{
	// FRunnableThread::Kill -> FRunnable::Stop()
	// 그 후 Run() 중지 -> FRunnable::Exit() 호출 순으로 처리됨
	if (!Sender) return false;
	FRunnableThread* Thread = Sender->GetThread();
	if (Thread)
	{
		Thread->Kill(false);
		return true;
	}
	return false;
}

bool NetSession::KillRecv()
{
	if (!Receiver) return false;
	FRunnableThread* Thread = Receiver->GetThread();
	if (Thread)
	{
		Thread->Kill(false);
		return true;
	}
	return false;
}

const NetAddress& NetSession::GetPeerAddr() const
{
	// 이 소켓과 연결되어있는 상대방의 주소와 포트를 반환한다.
	// 사용 전 소켓의 Connection 여부 확인이 필요하다.
	return PeerAddr;
}

bool NetSession::PushSendQueue(TSharedPtr<SendBuffer> sendBuffer)
{
	Sender->SendQueue.Enqueue(sendBuffer);
	return true;
}

bool NetSession::Send(TSharedPtr<SendBuffer> sendBuffer)
{
	bytesSent = 0;
	while (!NetSock->GetSocket()->Send(sendBuffer.Get()->GetBuf(), sendBuffer.Get()->GetSize(), bytesSent))
	{
		if (!NetSock->IsConnected() || !NetSock->IsValid()) return false;
	}
	return true;
}

bool NetSession::IsSendQueueEmpty()
{
	// Lock-free 
	// 큐를 Pop하는 스레드가 하나이기 때문에 락을 잡지 않아도 비어있는지 상태 확인은 할 수 있다
	return (Sender->SendQueue.IsEmpty());
}

bool NetSession::PushRecvQueue(TSharedPtr<RecvBuffer> recvBuffer)
{
	uint16 sessionId = recvBuffer->GetHeader()->senderId;
	while (!Receiver->Lock.TryLock());
	if (!Receiver->PendingClientBuffers.Contains(sessionId))
	{
		Receiver->PendingClientBuffers.Add(sessionId, MakeShared<TQueue<TSharedPtr<RecvBuffer>>>());
	}
	(*(Receiver->PendingClientBuffers.Find(sessionId)))->Enqueue(recvBuffer);
	Receiver->Lock.Unlock();
	return true;
}

bool NetSession::IsRecvQueueEmpty(uint16 sessionId)
{
	return ((*(Receiver->PendingClientBuffers.Find(sessionId)))->IsEmpty());
	// 큐를 Pop하는 스레드가 하나이기 때문에 락을 잡지 않아도 비어있는지 상태 확인은 할 수 있다
}

bool NetSession::Recv(TSharedPtr<RecvBuffer> recvBuffer)
{
	if (!NetSock->IsConnected() || !NetSock->IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("소켓이 연결되어 있지 않거나, 소켓이 유효하지 않습니다."));
		return false;
	}

	int32 bytesRead = 0; // Recv 호출 한번으로 읽어들인 버퍼 크기
	int32 bytesToRead = sizeof(PacketHeader);
	bool receivedHeader = false;

	while (bytesRead < bytesToRead)
	{
		if (bytesToRead > (int32)recvBuffer->GetLeftover())
		{
			UE_LOG(LogTemp, Fatal, TEXT("버퍼의 잔여 공간이 부족합니다, 현재까지 Recv한 버퍼는 유실됩니다. 읽어야 하는 데이터의 크기: %i"), bytesToRead);
			return false;
		}
		if (!NetSock->GetSocket()->Recv(recvBuffer->GetWriteCursor(), bytesToRead, bytesRead, ESocketReceiveFlags::None))
		{
			UE_LOG(LogTemp, Error, TEXT("소켓 연결 실패 혹은 알 수 없는 에러로 인해 패킷 수신에 실패하였습니다."));
			return false;
		}

		recvBuffer->MoveWriteCursor(bytesRead);
		recvBuffer->SetSize(recvBuffer->GetSize() + bytesRead);

		bytesToRead -= bytesRead;
		if (bytesToRead == 0)
		{
			if (!receivedHeader)
			{
				bytesToRead = (int32)recvBuffer->GetHeader()->size - sizeof(PacketHeader);
				receivedHeader = true;
			}
		}
		bytesRead = 0;
	}
	return true;
}

bool has_passed(FDateTime t1, FDateTime t2, int32 minutes, int32 seconds)
{
	// t1에서 minutes + seconds 이상이 경과했으면 true를, 그렇지 않다면 false를 반환한다.
	FTimespan timeLimit(0, minutes, seconds);
	return (t2 - t1 >= timeLimit);
}

bool NetSession::TryConnect(NetAddress connectAddr, int32 minutes, int32 seconds)
{
	FDateTime startTime = FDateTime::Now();
	while (!has_passed(startTime, FDateTime::Now(), minutes, seconds))
	{
		if (Connect(connectAddr))
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("소켓 연결 성공 %s:%i"), *connectAddr.GetIp(), connectAddr.GetPort()));
			return true;
		}
	}
	return false;
}

bool NetSession::Connect(NetAddress connectAddr)
{
	if (!NetSock->IsValid() || NetSock->IsConnected()) return false;

	TSharedRef<FInternetAddr> addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	FIPv4Address ip;
	FIPv4Address::Parse(connectAddr.GetIp(), ip);
	addr->SetIp(ip.Value);
	addr->SetPort(connectAddr.GetPort());

	NetSock->GetSocket()->Connect(*addr);
	bool ret = NetSock->IsConnected();
	if (ret) PeerAddr = connectAddr;
	return ret;
}

void NetSession::Disconnect()
{
}