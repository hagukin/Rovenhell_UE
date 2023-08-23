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

	//// 테스트 발송 (TEMP)
	T_BYTE sendTestData[10] = { T_BYTE(65), T_BYTE(65), T_BYTE(65), T_BYTE(65), T_BYTE(65), T_BYTE(65), T_BYTE(65), T_BYTE(65), T_BYTE(65), T_BYTE(65) }; // AAAAAAAAAA
	T_BYTE sendTestData2[2] = { T_BYTE(66), T_BYTE(66) }; // BB
	TSharedPtr<NetBuffer> writeBuf = Session->BufManager->SendPool->PopBuffer();

	/// 테스트용 프로토콜
	writeBuf->Write(sendTestData, sizeof(sendTestData));
	writeBuf->Write(sendTestData2, sizeof(sendTestData2));

	for (uint32 i = 0; i < writeBuf->GetSize(); ++i)
	{
		FString x = FString::FromInt(writeBuf->GetBuf()[i]);
		UE_LOG(LogTemp, Error, TEXT("%s"), *x);
	}
	Session->RegisterSend(writeBuf);
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
