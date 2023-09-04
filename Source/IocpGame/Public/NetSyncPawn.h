// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "NetSyncPawn.generated.h"


/*

TODO
현재는 ACharacter를 상속한 IocpGameCharacter를 쓰고 있으나
추후 클라이언트에서 플레이어 인풋 컨트롤이 가능한 Pawn을 만들 경우 ANetSyncPawn을 상속해서 사용하도록 제작
ANetSyncPawn에서는 Session 등에 대한 레퍼런스를 가져오는 등
네트워크부와 소통하기 위해 클라이언트의 Pawn들이 가져야 할 공통 부분들을 정의

*/

UCLASS()
class IOCPGAME_API ANetSyncPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ANetSyncPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
