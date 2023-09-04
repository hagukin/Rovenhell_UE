// Fill out your copyright notice in the Description page of Project Settings.


#include "NetSyncPawn.h"

// Sets default values
ANetSyncPawn::ANetSyncPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ANetSyncPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ANetSyncPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ANetSyncPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	APawn::SetupPlayerInputComponent(PlayerInputComponent);

}

