// Copyright 2023 Haguk Kim
// Author: Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "Misc/SpinLock.h"
#include "Templates/SharedPointer.h"
#include "../NetCodes/NetSession.h"
#include "../NetCodes/NetBufferManager.h"
#include "../PacketCodes/PacketHeader.h"
#include "../PacketCodes/InputApplier.h"
#include "../PacketCodes/ChatPacketApplier.h"
#include "../PacketCodes/GameStateApplier.h"
#include "../PacketCodes/MiddlemanPacketApplier.h"
#include "../PacketCodes/SerializeManager.h"
#include "GameTickCounter.h"
#include "RovenhellGameInstance.h"
#include "GameFramework/Actor.h"
#include "NetHandler.generated.h"

#define DESIRED_SERVER_SEND_CYCLE_PER_PACKET 0.125f // per fragment가 아님
#define MIN_FRAGMENTS_SEND_COUNT_PER_TICK 1
#define MAX_FRAGMENTS_SEND_COUNT_PER_TICK 10 // Congestion control

UCLASS()
class IOCPGAME_API ANetHandler : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANetHandler();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason);

	TSharedPtr<NetSession> GetSessionShared() { return Session; }
	TSharedPtr<SerializeManager> GetSerializerShared() { return Serializer; }
	TSharedPtr<SerializeManager> GetDeserializerShared() { return Deserializer; }
	HostTypeEnum GetHostType() { return HostType; }
	void FillPacketSenderTypeHeader(TSharedPtr<SendBuffer> buffer);
	AIocpGameState* GetIocpGameState() { return Cast<AIocpGameState>(GetWorld()->GetGameState()); }
	URovenhellGameInstance* GetRovenhellGameInstance() { return Cast<URovenhellGameInstance>(GetGameInstance()); }
	
private:
	void Init();
	void InitGameHostType();
	void PacketDebug(float DeltaTime);
	bool DistributePendingPacket(); // RecvPending 패킷을 적절한 Applier에게 전달하고, 처리가 완료되면 버퍼 풀에 버퍼를 반환한다
	uint8 GenerateUniquePacketId();
	uint8 GetCurrentPacketUniqueId() { return PacketUniqueId; }
	
	/*UEClient*/
	void Tick_UEClient(float DeltaTime);
	void RegisterSend_UEClient(float DeltaTime);
	void GetPendingBuffer_UEClient(float DeltaTime);
	void ProcessRecv_UEClient(float DeltaTime);
	void OnProcessRecvFinish_UEClient(float DeltaTime);

	/*UEServer*/
	void StartingNewGameTick_UEServer();
	void Tick_UEServer(float DeltaTime);
	void GetPendingBuffer_UEServer(float DeltaTime);
	void ProcessRecv_UEServer(float DeltaTime);
	void RegisterSend_UEServer(float DeltaTime);
	SD_GameState* CreateNewGameStatePacket_UEServer(float DeltaTime);
	void SendFragment_UEServer(float DeltaTime, int sendCount);
	void OnAllFragmentsSent_UEServer(float DeltaTime);

private:
	TSharedPtr<NetSession> Session;
	TSharedPtr<RecvBuffer> AddToRecvPendings = nullptr;
	TQueue<TSharedPtr<RecvBuffer>> RecvPendings; // 이번 틱에 처리할 모든 패킷들
	TSharedPtr<RecvBuffer> RecvPending = nullptr; // 처리를 대기중인 단일 패킷

	HostTypeEnum HostType = HostTypeEnum::NONE;

	TUniquePtr<InputApplier> InApplier = nullptr;
	TUniquePtr<ChatPacketApplier> ChatApplier = nullptr;
	TUniquePtr<GameStateApplier> GameApplier = nullptr;
	TUniquePtr<MiddlemanPacketApplier> MiddleApplier = nullptr;

	TSharedPtr<SerializeManager> Serializer = nullptr;
	TSharedPtr<SerializeManager> Deserializer = nullptr;

	float AccumulatedTickTime = 0.0f; // ms; 일정 주기로 tick에서 무언가를 처리하기 위해 사용; 매 틱에서의 DeltaTime 더해 누적

private:
	/*UEServer*/
	TMap<uint16, bool> HasProcessedOncePerTickPacket; // 이번 틱에 어떤 세션의 ONCE_PER_TICK 프로토콜 패킷이 처리되었는가; 매 틱마다 각 항목이 false로 초기화된다
	bool bShouldCreateNewPacket = true; // 이전 패킷의 모든 fragment들의 전송이 끝났으면 true
	uint8 PacketUniqueId = 1; // 1번부터 시작; Read 시 가급적 getter 사용
	uint8 PacketFragmentCount = 0;
	uint8 FragmentSendStart = 1; // 1번부터 시작
	float FragmentSendCycleTime = 0.0f; // fragment 1개 발송 주기 (s)
	float TimePassedSinceLastFragmentSend = 0.0f; // 현재 발송중인 패킷(fragment들의 합)에 소비된 DeltaTime 누적 
	uint32 PacketTick = 0; // 이 패킷 데이터를 추출했을 시점에서의 서버의 틱; 추후 fragment를 다른 틱에서 보내더라도 이 값은 동일하게 유지되어야 함.
};
