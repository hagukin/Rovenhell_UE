// Fill out your copyright notice in the Description page of Project Settings.


#include "NetPlayerMovementComponent.h"
#include "PlayerPawn.h"

void UNetPlayerMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // 모든 것이 아직 유효한지, 이동 가능한지 확인합니다.
    if (!PawnOwner || !UpdatedComponent || ShouldSkipUpdate(DeltaTime))
    {
        return;
    }

    //FVector DesiredMovementThisFrame = ConsumeInputVector().GetClampedToMaxSize(1.0f) * DeltaTime * 200.0f;
    //if (!DesiredMovementThisFrame.IsNearlyZero())
    //{
    //    FHitResult Hit;
    //    SafeMoveUpdatedComponent(DesiredMovementThisFrame, UpdatedComponent->GetComponentRotation(), true, Hit);

    //    // 무언가에 부딛혔으면, 돌아가 봅니다.
    //    if (Hit.IsValidBlockingHit())
    //    {
    //        SlideAlongSurface(DesiredMovementThisFrame, 1.f - Hit.Time, Hit.Normal, Hit);
    //    }
    //}
    //////////// TESTING
    FVector DesiredMovementThisFrame = ConsumeInputVector().GetClampedToMaxSize(1.0f) * 10.0f;
    PawnOwner->SetActorLocation(PawnOwner->GetActorLocation() + DesiredMovementThisFrame); ///// 임시 테스트 코드
}

APlayerPawn* UNetPlayerMovementComponent::GetPlayerOwner()
{
    return (APlayerPawn*)(GetOwner()); // risky...
}

ANetHandler* UNetPlayerMovementComponent::GetNetHandler()
{
	if (APlayerPawn* playerOwner = GetPlayerOwner())
	{
		return playerOwner->GetNetHandler();
	}
	return nullptr;
}
