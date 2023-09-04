// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Misc/SpinLock.h"
#include "../NetCodes/NetSession.h"
#include "../NetCodes/NetBufferManager.h"
#include "../PacketCodes/PacketHeader.h"
#include "../PacketCodes/ChatPacketApplier.h"
#include "../PacketCodes/ClientEventApplier.h"
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
	HostTypeEnum GetHostType() { return HostType; }
	void FillPacketSenderTypeHeader(TSharedPtr<SendBuffer> buffer);

private:
	void Init();
	void InitGameHostType();
	void PacketDebug(float DeltaTime);
	bool DistributePendingPacket(); // RecvPending 패킷을 적절한 Applier에게 전달하고, 처리가 완료되면 버퍼 풀에 버퍼를 반환한다
	
	void Tick_UEClient(float DeltaTime);
	void Tick_UEServer(float DeltaTime);
private:
	TSharedPtr<NetSession> Session;
	TSharedPtr<RecvBuffer> RecvPending = nullptr; // 처리를 대기중인 패킷
	TQueue<TSharedPtr<RecvBuffer>> SortedRecvPendings; // 틱 순서대로 정렬된, 처리를 대기중인 패킷들
	HostTypeEnum HostType = HostTypeEnum::NONE;

	TUniquePtr<ChatPacketApplier> ChatApplier = nullptr;
	TUniquePtr<ClientEventApplier> ClEventApplier = nullptr;

	uint32 ignoreTickLowerThan = 0; // 해당 틱 번호 이하 수신 패킷은 무시한다; 이는 Contradiction 방지를 위함 (2틱에서 b가 a를 킬한 패킷을 처리한 이후 1틱에서 a가 b를 킬한 패킷 수신 시의 상황)
	// 기존 GameState와 그전의 인풋 정보들을 기반으로 과거 GameState들을 재구성하는 방식도 사용 가능하지만,
	// 언리얼 물리엔진의 특성 상 자체 물리 연산을 구현하지 않는 이상 적용 불가능하다고 판단해 본 방식을 고안해 냄.
	// High latency client의 패킷이 무시되는 경우가 발생할 수 있음.

private:
	/* UE_SERVER */
	const float SERVER_TICK_INTERVAL = 0.1f; // ms
	float AccumulatedTickTime = 0.0f; // ms
};
