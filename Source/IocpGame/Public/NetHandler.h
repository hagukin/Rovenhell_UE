// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../NetCodes/NetSession.h"
#include "../NetCodes/NetBufferManager.h"
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

private:
	TUniquePtr<NetSession> Session;
	TSharedPtr<RecvBuffer> RecvPending = nullptr; // 처리를 대기중인 패킷
};
