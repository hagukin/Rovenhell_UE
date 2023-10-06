// Fill out your copyright notice in the Description page of Project Settings.


#include "NetActorSpawner.h"
#include "PlayerPawn.h"

ANetActorSpawner::ANetActorSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

}

void ANetActorSpawner::BeginPlay()
{
	Super::BeginPlay();
	Cast<URovenhellGameInstance>(GetGameInstance())->GetNetActorSpawner(); // 최초 캐싱
}

AActor* ANetActorSpawner::Spawn(TSubclassOf<AActor> actorToSpawn)
{
	return GetWorld()->SpawnActor<AActor>(actorToSpawn, GetPlayerSpawnTransform()); // TODO: Notification
}

bool ANetActorSpawner::Remove(AActor* actor)
{
	return actor->Destroy(); // TODO: Notification
}

APlayerPawn* ANetActorSpawner::SpawnNewPlayerPawn()
{
	APlayerPawn* player = Cast<APlayerPawn>(Spawn(PlayerPawnToSpawn));
	player->SpawnDefaultController(); // TODO: Custom controller
	return player;
}

FTransform ANetActorSpawner::GetPlayerSpawnTransform()
{
	return GetActorTransform(); // TODO
}

