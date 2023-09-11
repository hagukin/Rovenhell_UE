// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Engine.h"
#include "Sockets.h"
#include "Networking.h"
#include "SocketSubsystem.h"
#include "NetSocket.h"
#include "NetBuffer.h"
#include "NetAddress.h"
#include "SendHandler.h"
#include "RecvHandler.h"
#include "NetBufferManager.h"
#include "../PacketCodes/PacketHeader.h"

class SendHandler;
class RecvHandler;
class NetBufferManager;

/**
 * 이 host와 다른 host 하나와의 연결을 관리한다.
 */
class IOCPGAME_API NetSession
{
public:
	NetSession();
	~NetSession();

	bool Init(HostTypeEnum hostType); // 내부 요소를 초기화하고 성공 여부를 반환한다
	bool Start(); // 세션 작동을 개시하고 성공 여부를 반환한다
	bool StartSender(); // Sender 스레드를 생성하고 동작을 개시한다
	bool StartReceiver(); // Receiver 스레드를 생성하고 동작을 개시한다
	bool Kill(); // 세션 작동을 완전히 정지한다
	bool KillSend(); // Sender 스레드를 Kill한다
	bool KillRecv(); // Receiver 스레드를 Kill한다

	// NOTE:
	// Pop의 경우 직접 락 잡고 Pop해야 함, 혹은 TQueue의 경우 lock-free
	bool PushSendQueue(TSharedPtr<SendBuffer> sendBuffer); // 블로킹; Sender에게 Send를 요청할 버퍼를 대기열에 추가한다
	bool Send(TSharedPtr<SendBuffer> sendBuffer); // 블로킹 (단 SendHandler 스레드에서 실행된다)
	bool IsSendQueueEmpty();

	bool PushRecvQueue(TSharedPtr<RecvBuffer> recvBuffer); // 블로킹; 게임 처리가 필요한 버퍼를 대기열에 추가한다
	bool IsRecvQueueEmpty(uint64 sessionId);
	bool Recv(TSharedPtr<RecvBuffer> recvBuffer); // 블로킹 (단 RecvHandler 스레드에서 실행된다)

	bool TryConnect(NetAddress connectAddr, int32 minutes, int32 seconds); // waitForMs 밀리세컨드 동안 Connect를 시도하고 결과를 반환한다 (Blocking)
	bool Connect(NetAddress connectAddr); // 논블로킹 Connect I/O의 결과를 반환한다
	void Disconnect();

	const NetAddress& GetPeerAddr() const;
	void SetSessionId(uint64 id) { _sessionId = id; return; }
	const uint64 GetSessionId() const { return _sessionId; }

public:
	TUniquePtr<NetBufferManager> BufManager = nullptr;
	NetAddress PeerAddr;
	TUniquePtr<SendHandler> Sender = nullptr;
	TUniquePtr<RecvHandler> Receiver = nullptr;
	TUniquePtr<NetSocket> NetSock = nullptr;

private:
	int32 bytesSent = 0; // Send()에서 사용; 현재 소켓이 Send 중일 경우, 몇 바이트를 발송 완료했는지
	HostTypeEnum HostType;
	uint64 _sessionId = 0; // 미들맨에서 할당받는 세션의 고유 ID; 클라이언트 구분에 사용된다 (서버는 크게 의미없다)
};
