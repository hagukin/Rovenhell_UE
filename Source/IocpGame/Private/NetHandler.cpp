// Fill out your copyright notice in the Description page of Project Settings.


#include "NetHandler.h"

// Sets default values
ANetHandler::ANetHandler()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ANetHandler::BeginPlay()
{
	Super::BeginPlay();
	Init();
}

// Called every frame
void ANetHandler::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	switch (HostType)
	{
		case HostTypeEnum::CLIENT:
		case HostTypeEnum::CLIENT_HEADLESS:
			{
				Tick_UEClient(DeltaTime);
				break;
			}
		case HostTypeEnum::LOGIC_SERVER:
		case HostTypeEnum::LOGIC_SERVER_HEADLESS:
			{
				Tick_UEServer(DeltaTime);
				break;
			}
		case HostTypeEnum::NONE:
		default:
			{
				UE_LOG(LogTemp, Fatal, TEXT("호스트 타입이 설정되지 않았거나, 혹은 호스트 타입을 알 수 없습니다."));
				return;
			}
	}
}

void ANetHandler::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Session->Kill();
	Super::EndPlay(EndPlayReason);
}

void ANetHandler::Init()
{
	// 호스트 타입 캐시 (초기화 이후 변하지 않으므로)
	InitGameHostType();
	if (HostType == HostTypeEnum::NONE)
	{
		UE_LOG(LogTemp, Fatal, TEXT("호스트 타입이 초기화되지 않았습니다."));
	}

	// 세션 생성
	Session = MakeShared<NetSession>();
	Session->Init(HostType);

	// 커넥션
	int32 port = 0;

	// Applier 생성
	ChatApplier = MakeUnique<ChatPacketApplier>();
	ChatApplier->Init(GetSessionShared());
	PhysApplier = MakeUnique<PhysicsApplier>();
	PhysApplier->Init(GetSessionShared());
	MiddleApplier = MakeUnique<MiddlemanPacketApplier>();
	MiddleApplier->Init(GetSessionShared());


	// 호스트 타입 별 초기화
	if (HostType == HostTypeEnum::CLIENT || HostType == HostTypeEnum::CLIENT_HEADLESS)
	{
		port = 7777;
	}
	else if (HostType == HostTypeEnum::LOGIC_SERVER || HostType == HostTypeEnum::LOGIC_SERVER_HEADLESS)
	{
		port = 8888;
	}
	
	NetAddress serverAddr(TEXT("127.0.0.1"), port);

	bool connected = Session->TryConnect(serverAddr, 0, 1); // TODO: 만약 일정시간동안 시도를 반복하는 방식을 사용할 경우, BeginPlay에서 이를 처리하는 것은 좋지 못함, 비동기적 처리가 필요
	UE_LOG(LogTemp, Log, TEXT("커넥션 포트 번호: %i, 커넥션 상태: %d"), port, (int)connected);

	// 세션 작동
	if (Session->Start()) { UE_LOG(LogTemp, Log, TEXT("세션이 정상적으로 작동중입니다.")); }
}

void ANetHandler::FillPacketSenderTypeHeader(TSharedPtr<SendBuffer> buffer)
{
	((PacketHeader*)(buffer->GetBuf()))->senderType = HostType;
}

void ANetHandler::PacketDebug(float DeltaTime)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("틱 초: %f"), DeltaTime));
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("수신 패킷 크기: %d"), (int32)RecvPending->GetSize()));
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("수신 패킷 0번 문자: %i"), RecvPending->GetData()[0]));
}

bool ANetHandler::DistributePendingPacket()
{
	// RecvPending 버퍼를 적절한 Applier로 전달한다
	const PacketHeader header = *((PacketHeader*)(RecvPending->GetBuf()));
	bool applied = false;
	switch (header.id)
	{
	case PacketId::CHAT_GLOBAL:
		{
			applied = ChatApplier->ApplyPacket(RecvPending);
			break;
		}
	case PacketId::ACTOR_PHYSICS:
		{
			applied = PhysApplier->ApplyPacket(RecvPending);
			break;
		}
	case PacketId::SESSION_INFO:
		{
			applied = MiddleApplier->ApplyPacket(RecvPending);
			break;
		}
	default:
		UE_LOG(LogTemp, Warning, TEXT("패킷 id %i는 Applier에 의해 처리되지 않았습니다."), header.id);
		break;
	}
	return applied;
}

void ANetHandler::InitGameHostType()
{
	URovenhellGameInstance* game = Cast<URovenhellGameInstance, UGameInstance>(UGameplayStatics::GetGameInstance(GetWorld())); // downcasting
	if (game)
	{
		if (game->GetExecType())
		{
			HostType = game->GetExecType()->GetHostType();
		}
	}
}

void ANetHandler::Tick_UEClient(float DeltaTime)
{
	//// 발송
	// 테스트용 패킷 생성
	//T_BYTE sendTestData[10] = { T_BYTE(65), T_BYTE(65), T_BYTE(65), T_BYTE(65), T_BYTE(65), T_BYTE(65), T_BYTE(65), T_BYTE(65), T_BYTE(65), T_BYTE(65) }; // AAAAAAAAAA
	//T_BYTE sendTestData2[2] = { T_BYTE(66), T_BYTE(66) }; // BB
	//TSharedPtr<SendBuffer> writeBuf;
	//while (!writeBuf) writeBuf = Session->BufManager->SendPool->PopBuffer(); // TODO: Receiver에서 처리하듯이 대기 시간 늘려가면서 버퍼 가져올때까지 스레드 대기하도록 만들기
	//writeBuf->Write(sendTestData, sizeof(sendTestData));
	//writeBuf->Write(sendTestData2, sizeof(sendTestData2));
	//FillPacketSenderTypeHeader(writeBuf);
	//((PacketHeader*)(writeBuf->GetBuf()))->id = PacketId::CHAT_GLOBAL;
	//Session->PushSendQueue(writeBuf);

	//// 수신
	// 1 event tick에 하나의 패킷을 처리
	while (!RecvPending && !Session->Receiver->RecvPriorityQueue.IsEmpty())
	{
		RecvPriorityQueueNode node;

		while (!Session->Receiver->Lock.TryLock());
		Session->Receiver->RecvPriorityQueue.HeapPop(node);
		Session->Receiver->Lock.Unlock();

		RecvPending = node.recvBuffer;

		uint16 recvBufferProtocol = ((PacketHeader*)RecvPending->GetBuf())->protocol;
		switch (recvBufferProtocol)
		{
			case PacketProtocol::LOGIC_EVENT:
			case PacketProtocol::MIDDLEMAN_EVENT:
			{
				break;
			}
			// 클라이언트가 처리할 수 없는 프로토콜
			case PacketProtocol::NO_PROTOCOL:
			case PacketProtocol::CLIENT_EVENT_ON_RECV:
			case PacketProtocol::CLIENT_EVENT_ON_TICK_STRICT:
			case PacketProtocol::CLIENT_EVENT_ON_TICK_LOOSE:
			{
				UE_LOG(LogTemp, Warning, TEXT("클라이언트에서 처리할 수 없는 잘못된 패킷 프로토콜 %i입니다. 패킷을 무시합니다."), recvBufferProtocol);
				Session->BufManager->RecvPool->PushBuffer(MoveTemp(RecvPending));
				RecvPending = nullptr; // 다시 루프
				break;
			}
		}
	}

	if (RecvPending)
	{
		//PacketDebug(DeltaTime);
		if (!DistributePendingPacket())
		{
			UE_LOG(LogTemp, Error, TEXT("수신 완료한 패킷 내용을 적용하는 과정에서 문제가 발생했습니다, 해당 패킷 내용은 무시됩니다."));
		}
		Session->BufManager->RecvPool->PushBuffer(MoveTemp(RecvPending));
		RecvPending = nullptr;
	}
}

void ANetHandler::Tick_UEServer(float DeltaTime)
{
	AccumulatedTickTime += DeltaTime;
	// 인터벌이 되지 않아도 처리 대기 큐가 비어있다면 바로 가져온다
	if ((AccumulatedTickTime >= SERVER_TICK_INTERVAL) || SortedRecvPendings.IsEmpty()) // TODO: 아래와 마찬가지로 1 frame 제한 시간 초과할 경우 중단
	{
		AccumulatedTickTime = 0;

		RecvPriorityQueueNode node;
		while (!Session->Receiver->Lock.TryLock());
		while (!Session->Receiver->RecvPriorityQueue.IsEmpty())
		{
			Session->Receiver->RecvPriorityQueue.HeapPop(node);
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("정렬 순서 %i"), ((PacketHeader*)node.recvBuffer->GetBuf())->tick));
			SortedRecvPendings.Enqueue(node.recvBuffer);
		}
		Session->Receiver->Lock.Unlock();
	}

	//// 수신
	// 같은 틱 값을 가진 패킷들을 묶어 처리한다
	while (!SortedRecvPendings.IsEmpty() || RecvPending) // TODO: calculatingTick에 해당하는 패킷들의 처리가 다 안끝났어도 1 frame 제한 시간을 초과할 경우 다음 함수 콜에서 처리하도록 미루는 기능
	{
		if (RecvPending)
		{
			//PacketDebug(DeltaTime);
			if (!DistributePendingPacket())
			{
				UE_LOG(LogTemp, Error, TEXT("수신 완료한 패킷 내용을 적용하는 과정에서 문제가 발생했습니다, 해당 패킷 내용은 무시됩니다."));
			}
			Session->BufManager->RecvPool->PushBuffer(MoveTemp(RecvPending));
			RecvPending = nullptr;
		}
		if (!SortedRecvPendings.IsEmpty())
		{
			SortedRecvPendings.Dequeue(RecvPending);

			uint32 recvBufferTick = ((PacketHeader*)RecvPending->GetBuf())->tick;
			uint16 recvBufferProtocol = ((PacketHeader*)RecvPending->GetBuf())->protocol;
			if (recvBufferTick > lastAppliedTick)
			{
				lastAppliedTick = recvBufferTick; // 다음 함수 콜에서 처리해야 할 틱 번호 설정
				break;
			}
			else if (recvBufferTick < lastAppliedTick)
			{
				switch (recvBufferProtocol)
				{
					// 순서 지났더라도 처리 보장
					case PacketProtocol::CLIENT_EVENT_ON_TICK_LOOSE:
						UE_LOG(LogTemp, Warning, TEXT("패킷 처리 순서가 지켜지지 않았습니다. 패킷을 발송한 클라이언트의 네트워크 지연시간이 다른 클라이언트와 크게 차이가 날 가능성이 있습니다."));
					case PacketProtocol::CLIENT_EVENT_ON_RECV:
					case PacketProtocol::MIDDLEMAN_EVENT:
						{
							break;
						}
					// 순서 고려하는 경우
					case PacketProtocol::CLIENT_EVENT_ON_TICK_STRICT:
						{
							UE_LOG(LogTemp, Warning, TEXT("순서가 지나간 패킷을 무시합니다. 패킷을 발송한 클라이언트의 네트워크 지연시간이 다른 클라이언트와 크게 차이가 날 가능성이 있습니다."));
							Session->BufManager->RecvPool->PushBuffer(MoveTemp(RecvPending));
							RecvPending = nullptr;
							break;
						}
					// 로직 서버가 처리할 수 없는 프로토콜
					case PacketProtocol::NO_PROTOCOL:
					case PacketProtocol::LOGIC_EVENT:
						{
							UE_LOG(LogTemp, Warning, TEXT("로직서버에서 처리할 수 없는 잘못된 패킷 프로토콜 %i입니다. 패킷을 무시합니다."), recvBufferProtocol);
							Session->BufManager->RecvPool->PushBuffer(MoveTemp(RecvPending));
							RecvPending = nullptr;
							break;
						}
				}
			}
			// 처리 순서에 해당될 경우 continue 해서 처리
		}
	}
}