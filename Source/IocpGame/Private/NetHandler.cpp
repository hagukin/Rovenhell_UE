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
	ChatApplier->Init();
	ClEventApplier = MakeUnique<ClientEventApplier>();
	ClEventApplier->Init();


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
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("커넥션 포트 번호: %i, 커넥션 상태: %d"), port, (int)connected));

	// 세션 작동
	if (Session->Start()) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("세션이 정상적으로 작동중입니다.")));
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
	case PacketId::CLIENT_EVENT:
		{
			applied = ClEventApplier->ApplyPacket(RecvPending);
			break;
		}
	default:
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
	// 상대방이 DeltaTime 이상의 시간을 주기로 패킷을 보내야 틱에서 데이터를 제대로 처리할 수 있다
	//if (RecvPending)
	//{
	//	//PacketDebug(DeltaTime);
	//	if (!DistributePendingPacket())
	//	{
	//		UE_LOG(LogTemp, Error, TEXT("수신 완료한 패킷 내용을 적용하는 과정에서 문제가 발생했습니다, 해당 패킷 내용은 무시됩니다."));
	//	}
	//	Session->BufManager->RecvPool->PushBuffer(MoveTemp(RecvPending));
	//	RecvPending = nullptr;
	//}
	//if (!Session->IsRecvQueueEmpty())
	//{
	//	RecvPriorityQueueNode node;

	//	while (!Session->Receiver->Lock.TryLock());
	//	Session->Receiver->RecvPriorityQueue.HeapPop(node);
	//	Session->Receiver->Lock.Unlock();

	//	RecvPending = node.recvBuffer;
	//}
}

void ANetHandler::Tick_UEServer(float DeltaTime)
{
	AccumulatedTickTime += DeltaTime;

	if ((AccumulatedTickTime >= SERVER_TICK_INTERVAL)) // TODO: 아래와 마찬가지로 1 frame 제한 시간 초과할 경우 중단
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
			if (recvBufferTick > ignoreTickLowerThan)
			{
				ignoreTickLowerThan = recvBufferTick; // 현재 함수 콜에서 처리해야 할 틱 번호 설정
				break;
			}
			else if (recvBufferTick < ignoreTickLowerThan)
			{
				UE_LOG(LogTemp, Warning, TEXT("Gameplay Contradiction 발생 가능한 패킷을 무시합니다. 클라이언트 접속 허용 최대 Ping을 낮추는 것을 고려하십시오."));
				Session->BufManager->RecvPool->PushBuffer(MoveTemp(RecvPending));
				RecvPending = nullptr;
			}
			// 같을 경우 루프 continue
		}
	}
}