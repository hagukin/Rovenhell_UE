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
				Tick_UEClient();
				break;
			}
		case HostTypeEnum::LOGIC_SERVER:
		case HostTypeEnum::LOGIC_SERVER_HEADLESS:
			{
				Tick_UEServer();
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
	SetGameHostType();
	if (HostType == HostTypeEnum::NONE)
	{
		UE_LOG(LogTemp, Fatal, TEXT("호스트 타입이 초기화되지 않았습니다."));
	}

	// 세션 생성
	Session = MakeUnique<NetSession>();
	Session->Init();

	// Applier 생성
	ChatApplier = MakeUnique<ChatPacketApplier>();
	ChatApplier->Init();

	// 커넥션
	int32 port = 7777;
	if (HostType == HostTypeEnum::CLIENT || HostType == HostTypeEnum::CLIENT_HEADLESS) port = 7777;
	else if (HostType == HostTypeEnum::LOGIC_SERVER || HostType == HostTypeEnum::LOGIC_SERVER_HEADLESS) port = 8888;
	NetAddress serverAddr(TEXT("127.0.0.1"), port);

	bool connected = Session->TryConnect(serverAddr, 0, 1); // TODO: 만약 일정시간동안 시도를 반복하는 방식을 사용할 경우, BeginPlay에서 이를 처리하는 것은 좋지 못함, 비동기적 처리가 필요
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("커넥션 포트: %i, 커넥션 상태: %d"), port, (int)connected));

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
	default:
		break;
	}
	return applied;
}

void ANetHandler::SetGameHostType()
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

void ANetHandler::Tick_UEClient()
{
	//// 발송
	// 테스트용 패킷 생성
	T_BYTE sendTestData[10] = { T_BYTE(65), T_BYTE(65), T_BYTE(65), T_BYTE(65), T_BYTE(65), T_BYTE(65), T_BYTE(65), T_BYTE(65), T_BYTE(65), T_BYTE(65) }; // AAAAAAAAAA
	T_BYTE sendTestData2[2] = { T_BYTE(66), T_BYTE(66) }; // BB
	TSharedPtr<SendBuffer> writeBuf;
	while (!writeBuf) writeBuf = Session->BufManager->SendPool->PopBuffer(); // TODO: Receiver에서 처리하듯이 대기 시간 늘려가면서 버퍼 가져올때까지 스레드 대기하도록 만들기
	writeBuf->Write(sendTestData, sizeof(sendTestData));
	writeBuf->Write(sendTestData2, sizeof(sendTestData2));
	FillPacketSenderTypeHeader(writeBuf);
	((PacketHeader*)(writeBuf->GetBuf()))->id = PacketId::CHAT_GLOBAL;
	Session->PushSendQueue(writeBuf);

	//// 수신
	// 상대방이 DeltaTime 이상의 시간을 주기로 패킷을 보내야 틱에서 데이터를 제대로 처리할 수 있다
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
	if (!Session->IsRecvQueueEmpty())
	{
		while (!Session->Receiver->Lock.TryLock());
		Session->Receiver->RecvQueue.Dequeue(RecvPending);
		Session->Receiver->Lock.Unlock();
	}
}

void ANetHandler::Tick_UEServer()
{
	//// 발송
	// 테스트용 패킷 생성
	T_BYTE sendTestData[10] = { T_BYTE(65), T_BYTE(65), T_BYTE(65), T_BYTE(65), T_BYTE(65), T_BYTE(65), T_BYTE(65), T_BYTE(65), T_BYTE(65), T_BYTE(65) }; // AAAAAAAAAA
	T_BYTE sendTestData2[2] = { T_BYTE(66), T_BYTE(66) }; // BB
	TSharedPtr<SendBuffer> writeBuf;
	while (!writeBuf) writeBuf = Session->BufManager->SendPool->PopBuffer(); // TODO: Receiver에서 처리하듯이 대기 시간 늘려가면서 버퍼 가져올때까지 스레드 대기하도록 만들기
	writeBuf->Write(sendTestData, sizeof(sendTestData));
	writeBuf->Write(sendTestData2, sizeof(sendTestData2));
	FillPacketSenderTypeHeader(writeBuf);
	((PacketHeader*)(writeBuf->GetBuf()))->id = PacketId::CHAT_GLOBAL;
	Session->PushSendQueue(writeBuf);

	//// 수신
	// 상대방이 DeltaTime 이상의 시간을 주기로 패킷을 보내야 틱에서 데이터를 제대로 처리할 수 있다
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
	if (!Session->IsRecvQueueEmpty())
	{
		while (!Session->Receiver->Lock.TryLock());
		Session->Receiver->RecvQueue.Dequeue(RecvPending);
		Session->Receiver->Lock.Unlock();
	}
}
