// Fill out your copyright notice in the Description page of Project Settings.

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
	void UpdateLastProcessedInputTickForSession(uint64 sessionId, uint32 tick);
	uint32 GetLastProcessedInputTickForSession(uint64 sessionId);
	HostTypeEnum GetHostType() { return HostType; }
	void FillPacketSenderTypeHeader(TSharedPtr<SendBuffer> buffer);

private:
	void Init();
	void InitGameHostType();
	void PacketDebug(float DeltaTime);
	bool DistributePendingPacket(); // RecvPending 패킷을 적절한 Applier에게 전달하고, 처리가 완료되면 버퍼 풀에 버퍼를 반환한다
	
	void Tick_UEClient(float DeltaTime);

	void StartingNewGameTick_UEServer();
	void Tick_UEServer(float DeltaTime);
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
	TMap<uint64, bool> HasProcessedOncePerTickPacket; // 이번 틱에 어떤 세션의 ONCE_PER_TICK 프로토콜 패킷이 처리되었는가; 매 틱마다 각 항목이 false로 초기화된다
	TMap<uint64, uint32> LastProcessedInputTick; // 각 클라이언트 별로 몇 로컬 틱까지 서버에서 처리가 완료되었는가; 이 정보는 클라이언트로 Broadcast되어, 클라이언트가 수신받은 서버 정보를 적용한 후 몇 로컬 틱까지 인풋을 재연산해야 하는지 구할 때 사용된다
};
