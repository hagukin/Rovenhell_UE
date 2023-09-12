// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/SceneComponent.h"
#include "ActorSyncComponent.h"
#include "NetHandler.h"
#include "NetSyncPawn.generated.h"


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

	FORCEINLINE class UActorSyncComponent* GetSyncComp() const { return SyncComp; }
	FORCEINLINE class ANetHandler* GetNetHandler() const { return NetHandler; }
	void SetNetHandler(ANetHandler* handler) { NetHandler = handler; }

public:
	//// 컴포넌트
	// 루트
	UPROPERTY(EditAnywhere)
	USceneComponent* SceneComp;

	// NOTE: 
	// 루트에 붙는 컴포넌트가 추가되야 할 필요가 생기면 NetSyncPawn 내에서 정의하지 말고
	// NetSyncPawn을 상속받아서 해당 클래스 내에서 정의할 것.
	// 이는 상속받은 클래스 내부에서 루트가 바뀔 수 있는데, 그 과정에서 코드가 심각하게 꼬이는 일을 방지하기 위함임.

	// 네트워크 싱크
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Network, meta = (AllowPrivateAccess = "true"))
	UActorSyncComponent* SyncComp;

protected:
	ANetHandler* NetHandler = nullptr;
};
