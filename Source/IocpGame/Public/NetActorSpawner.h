// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetActorSpawner.generated.h"

class APlayerPawn;

/*
* 싱크를 맞춰야 하는 모든 Global 액터는 이 객체에 의해 생성과 삭제가 처리되어야 한다.
* Local 액터는 일반적인 생성과 삭제 방식으로 처리해도 상관 없다.
*/
UCLASS()
class IOCPGAME_API ANetActorSpawner : public AActor
{
	GENERATED_BODY()
	
public:
	ANetActorSpawner();

protected:
	virtual void BeginPlay() override;

public:
	AActor* Spawn(TSubclassOf<AActor> actorToSpawn);
	bool Remove(AActor* actor);
	APlayerPawn* SpawnNewPlayerPawn();
	FTransform GetPlayerSpawnTransform();

public:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<APawn> PlayerPawnToSpawn;
};
