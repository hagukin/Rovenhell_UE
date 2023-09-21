// Fill out your copyright notice in the Description page of Project Settings.

#include "NetSyncPawn.h"
#include "NetHandler.h"

// Sets default values
ANetSyncPawn::ANetSyncPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComp"));
	SetRootComponent(SceneComp);

	PhysicsSyncComp = CreateDefaultSubobject<UActorPhysicsSyncComponent>(TEXT("PhysicsSyncComp"));
	this->AddOwnedComponent(PhysicsSyncComp);

	// 레플리케이션 해제
	bReplicates = false;
}

// Called when the game starts or when spawned
void ANetSyncPawn::BeginPlay()
{
	Super::BeginPlay();
	
	// NetHandler 레퍼런스 가져오기
	AActor* temp = UGameplayStatics::GetActorOfClass(GetWorld(), ANetHandler::StaticClass());
	if (!temp)
	{
		UE_LOG(LogTemp, Warning, TEXT("ANetHandler 클래스의 액터를 찾을 수 없습니다."));
	}
	else
	{
		NetHandler = Cast<ANetHandler>(temp);
	}

	// 레플리케이션 해제
	if (AController* controller = GetController())
	{
		controller->SetReplicates(false);
	}
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

