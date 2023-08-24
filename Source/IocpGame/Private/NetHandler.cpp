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
	InitSession();
}

// Called every frame
void ANetHandler::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//// 발송
	// 테스트용 패킷 생성
	T_BYTE sendTestData[10] = { T_BYTE(65), T_BYTE(65), T_BYTE(65), T_BYTE(65), T_BYTE(65), T_BYTE(65), T_BYTE(65), T_BYTE(65), T_BYTE(65), T_BYTE(65) }; // AAAAAAAAAA
	T_BYTE sendTestData2[2] = { T_BYTE(66), T_BYTE(66) }; // BB
	TSharedPtr<SendBuffer> writeBuf;
	while (!writeBuf) writeBuf = Session->BufManager->SendPool->PopBuffer();
	writeBuf->Write(sendTestData, sizeof(sendTestData));
	writeBuf->Write(sendTestData2, sizeof(sendTestData2));
	Session->PushSendQueue(writeBuf);

	//// 수신
	// 상대방이 DeltaTime 이상의 시간을 주기로 패킷을 보내야 틱에서 데이터를 제대로 처리할 수 있다
	if (RecvPending)
	{
		// 버퍼 처리
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("틱 초: %f"), DeltaTime));
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("수신 패킷 크기: %d"), (int32)RecvPending->GetSize()));
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("수신 패킷: %s"), *BytesToString(RecvPending->GetData(), (int32)RecvPending->GetSize() - sizeof(PacketHeader))));
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

void ANetHandler::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Session->Kill();
	Super::EndPlay(EndPlayReason);
}

void ANetHandler::InitSession()
{
	Session = MakeUnique<NetSession>();
	Session->Init();
	NetAddress serverAddr(TEXT("127.0.0.1"), 7777);
	bool connected = Session->TryConnect(serverAddr, 0, 1); // TODO: 만약 일정시간동안 시도를 반복하는 방식을 사용할 경우, BeginPlay에서 이를 처리하는 것은 좋지 못함, 비동기적 처리가 필요
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("커넥션 상태: %d"), (int)connected));
	if (Session->Start()) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("세션이 정상적으로 작동중입니다.")));
}
