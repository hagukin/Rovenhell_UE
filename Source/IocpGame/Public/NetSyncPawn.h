// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/SceneComponent.h"
#include "ActorPhysicsSyncComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NetSyncPawn.generated.h"

class ANetHandler;

UCLASS()
class IOCPGAME_API ANetSyncPawn : public APawn
{
	GENERATED_BODY()

public:
	ANetSyncPawn();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UActorPhysicsSyncComponent* GetPhysicsSyncComp() const { return PhysicsSyncComp; }
	FORCEINLINE class ANetHandler* GetNetHandler() const { return NetHandler; }
	void SetNetHandler(class ANetHandler* handler) { NetHandler = handler; }
	bool SetOwnerSessionId(uint64 id);
	uint64 GetOwnerSessionId(uint64 id);

public:
	//// 컴포넌트
	// 루트
	UPROPERTY(EditAnywhere)
	USceneComponent* SceneComp;

	// 네트워크 싱크
	UActorPhysicsSyncComponent* PhysicsSyncComp;

protected:
	ANetHandler* NetHandler = nullptr;

private:
	bool SessionIdInitialized = false;
	uint64 OwnerSessionId = 0;
};
