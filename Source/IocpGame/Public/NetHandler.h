// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../NetCodes/NetSession.h"
#include "../NetCodes/NetBufferManager.h"
#include "../PacketCodes/PacketHeader.h"
#include "../PacketCodes/ChatPacketApplier.h"
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

private:
	void InitSession();
	void FillPacketSenderTypeHeader(TSharedPtr<SendBuffer> buffer);
	void PacketDebug(float DeltaTime);
	bool DistributePendingPacket(); // RecvPending 패킷을 적절한 Applier에게 전달하고, 처리가 완료되면 버퍼 풀에 버퍼를 반환한다
	void GetGameHostType(uint8& hostType);
	

private:
	TUniquePtr<NetSession> Session;
	TSharedPtr<RecvBuffer> RecvPending = nullptr; // 처리를 대기중인 패킷

	TUniquePtr<ChatPacketApplier> ChatApplier;
};
