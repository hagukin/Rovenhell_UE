// Fill out your copyright notice in the Description page of Project Settings.
#include "NetSession.h"

NetSession::NetSession()
{
}

NetSession::~NetSession()
{
}

bool NetSession::Init()
{
	BufManager = MakeUnique<NetBufferManager>();
	BufManager->Init();

	NetSock = MakeUnique<NetSocket>();
	if (NetSock->InitSocket())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("소켓 생성 성공")));
		Sender = MakeUnique<SendHandler>();
		Sender->SetSession(this);
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
		// TODO: Receiver 가동
	}
	return ret;
}

bool NetSession::StartSender()
{
	return Sender->CreateThread();
}

bool NetSession::StartReceiver()
{
	return false;
}

bool NetSession::Kill()
{
	bool bKilled = true;
	if (Sender) bKilled &= KillSend();
	// if (Receiver) bKilled &= KillRecv(); // TODO
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
	// TODO
	return false;
}

const NetAddress& NetSession::GetPeerAddr()
{
	// 이 소켓과 연결되어있는 상대방의 주소와 포트를 반환한다.
	// 사용 전 소켓의 Connection 여부 확인이 필요하다.
	return PeerAddr;
}

bool NetSession::RegisterSend(TSharedPtr<NetBuffer> sendBuffer)
{
	while (!Sender->Lock.TryLock());
	Sender->SendQueue.Enqueue(sendBuffer);
	Sender->Lock.Unlock();
	return true;
}

bool NetSession::Send(TSharedPtr<NetBuffer> sendBuffer)
{
	bytesSent = 0;
	while (!NetSock->GetSocket()->Send(sendBuffer.Get()->GetBuf(), sendBuffer.Get()->GetSize(), bytesSent)) // TODO: 추가 안전장치 - 시간 제한 등
	{
		if (!NetSock->IsConnected() || !NetSock->IsValid()) return false;
	}
	return true;
}

bool NetSession::Recv()
{
	return false;
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