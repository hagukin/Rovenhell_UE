// Copyright 2023 Haguk Kim
// Author: Haguk Kim

#include "NetPawnMovementComponent.h"

UNetPawnMovementComponent::UNetPawnMovementComponent()
{
}

void UNetPawnMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    ApplyNetPhysics(DeltaTime);
}

void UNetPawnMovementComponent::ApplyNetPhysics(float DeltaTime)
{
    ApplyGravity(DeltaTime);
}

FVector UNetPawnMovementComponent::GetNewFallVelocity(const FVector& InitialVelocity, const FVector& Gravity, float DeltaTime) const
{
    FVector Result = InitialVelocity;
    if (DeltaTime > 0.f)
    {
        // 중력 연산의 처리
        Result += Gravity * DeltaTime;

        // Terminal Limit 아래로 capping
        const float TerminalLimit = FMath::Abs(Cast<UMovementComponent>(this)->GetPhysicsVolume()->TerminalVelocity);
        if (Result.SizeSquared() > FMath::Square(TerminalLimit))
        {
            const FVector GravityDir = Gravity.GetSafeNormal();
            if ((Result | GravityDir) > TerminalLimit)
            {
                Result = FVector::PointPlaneProject(Result, FVector::ZeroVector, GravityDir) + GravityDir * TerminalLimit;
            }
        }
    }
    return Result;
}

ANetSyncPawn* UNetPawnMovementComponent::GetNetSyncPawn()
{
    return Cast<ANetSyncPawn>(PawnOwner);
}

void UNetPawnMovementComponent::ApplyGravity(float DeltaTime)
{
    ANetSyncPawn* Pawn = GetNetSyncPawn();
    FHitResult gravityHit;
    const FVector netPawnGravity = GetNetPawnGravity();
    FVector fallVelocity = GetNewFallVelocity(Velocity, netPawnGravity, DeltaTime);
    FVector fallDelta = fallVelocity * DeltaTime;
    SafeMoveUpdatedComponent(fallDelta, Pawn->GetTransform().GetRotation(), true, gravityHit); // 실제 낙하 연산
}
