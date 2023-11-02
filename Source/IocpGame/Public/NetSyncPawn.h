// Copyright 2023 Haguk Kim
// Author: Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/SceneComponent.h"
#include "ActorPhysicsSyncComponent.h"
#include "NetPawnInterpComponent.h"
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
	
	virtual void Puppetfy(); // 이 호스트가 조작하지 않는 폰의 경우 Puppetfy를 호출해주어야 한다

	UActorPhysicsSyncComponent* GetPhysicsSyncComp() const { return PhysicsSyncComp; }
	UNetPawnInterpComponent* GetInterpComp() const { return InterpComp; }
	FORCEINLINE class ANetHandler* GetNetHandler() const { return NetHandler; }
	void SetNetHandler(class ANetHandler* handler) { NetHandler = handler; }
	bool SetOwnerSessionId(uint16 id);
	uint64 GetOwnerSessionId(uint16 id);
	bool IsPuppet() { return bIsPuppet; }

public:
	//// 컴포넌트
	// 루트
	UPROPERTY(EditAnywhere)
	USceneComponent* SceneComp;

	// 네트워크 싱크
	// 호스트 폰일 경우 PhysicsSyncComp 사용
	UActorPhysicsSyncComponent* PhysicsSyncComp;
	// Puppet일 경우 InterpComp 사용
	UNetPawnInterpComponent* InterpComp;

protected:
	ANetHandler* NetHandler = nullptr;

private:
	bool bIsPuppet = false;
	bool SessionIdInitialized = false;
	uint16 OwnerSessionId = 0;
};
