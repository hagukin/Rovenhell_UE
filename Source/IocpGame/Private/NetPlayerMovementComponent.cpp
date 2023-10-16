// Copyright 2023 Haguk Kim
// Author: Haguk Kim

#include "NetPlayerMovementComponent.h"
#include "PlayerPawn.h"
#include "GameUtilities.h"

void UNetPlayerMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    BeginTick();

    APlayerPawn* Player = GetPlayer();
    if (!Player || !UpdatedComponent) return;

    // Movement 인풋 처리
    // 서버의 경우 단일 폰에 대해 1틱에 여러 인풋을 처리할 수 있는데,
    // 각 인풋별로 발송받은 델타 타임을 적용해 연산한다.
    for (const MoveInputData& DataPerInput : MoveDatas)
    {
        // 이동
        ApplyMovement(GetCurrentFacingDirection(), DataPerInput.DeltaTime);

        // 회전
        this->DesiredDirection = DataPerInput.MoveVector;
        ApplyRotation(GetCurrentFacingDirection(), DataPerInput.DeltaTime);

        MoveInputDeltaTimeSumThisTick += DataPerInput.DeltaTime;
        MoveInputCountThisTick++;
    }

    EndTick(DeltaTime);
}

APlayerPawn* UNetPlayerMovementComponent::GetPlayerOwner()
{
    return (APlayerPawn*)(PawnOwner.Get()); // risky...
}

ANetHandler* UNetPlayerMovementComponent::GetNetHandler()
{
	if (APlayerPawn* playerOwner = GetPlayerOwner())
	{
		return playerOwner->GetNetHandler();
	}
	return nullptr;
}

void UNetPlayerMovementComponent::BeginTick()
{
}

void UNetPlayerMovementComponent::EndTick(float HostDeltaTime)
{
    // Movement Status 싱크
    if (MoveInputCountThisTick > 0)
    {
        MovementStatus = FMath::Clamp(MovementStatus + MoveStatIncrementPerInput * MoveInputCountThisTick, 0, MaxMovementStatus);
        MoveInputCountThisTick = 0;
    }
    else
    {
        MovementStatus = FMath::Clamp(MovementStatus - MoveStatDecrementPerTick, 0, MaxMovementStatus);
    }

    // 애니메이션 재생 시간 업데이트
    if (MoveInputDeltaTimeSumThisTick > 0.0f)
    {
        MoveInputDeltaTimeSumThisTick = FMath::Max(MoveInputDeltaTimeSumThisTick - HostDeltaTime, 0.0f); // 호스트 틱만큼 줄여나간다
        GetPlayer()->SetIsMovingTo(true);
    }
    else
    {
        GetPlayer()->SetIsIdlingTo(true);
    }

    // 인풋 초기화
    MoveDatas.Empty();
}

void UNetPlayerMovementComponent::AddMovementData(FVector MoveVector, float DeltaTime)
{
    MoveDatas.Add({ MoveVector, DeltaTime });
}

APlayerPawn* UNetPlayerMovementComponent::GetPlayer()
{
    return Cast<APlayerPawn>(PawnOwner);
}

FVector UNetPlayerMovementComponent::GetCurrentFacingDirection()
{
    if (APlayerPawn* Player = GetPlayer()) 
        return Player->GetRootComponent()->GetComponentTransform().GetRotation().Vector(); 
    else 
        return FVector(0, 0, 0);
}

void UNetPlayerMovementComponent::ApplyMovement(FVector FacingDirection, float InputDeltaTime)
{
    APlayerPawn* Player = GetPlayer();
    if (!Player) return;
    FHitResult hitResult;
    FVector moveVector = FacingDirection.GetClampedToMaxSize(1.0f) * PlayerSpeed * InputDeltaTime;
    if (!moveVector.IsNearlyZero())
    {
        FHitResult Hit;
        SafeMoveUpdatedComponent(moveVector, UpdatedComponent->GetComponentRotation(), true, Hit);
        if (Hit.IsValidBlockingHit())
        {
            SlideAlongSurface(moveVector, 1.f - Hit.Time, Hit.Normal, Hit);
        }
    }
}

void UNetPlayerMovementComponent::ApplyRotation(FVector FacingDirection, float InputDeltaTime)
{
    APlayerPawn* Player = GetPlayer();
    if (!Player) return;

    float rotateDirection = GameUtilities::IsVectorCCW2D(FacingDirection, DesiredDirection);
    if (rotateDirection > 0) // 언리얼의 xy 평면은 x가 세로방향이기 때문에 주의 필요
    {
        Player->AddActorLocalRotation(FRotator(0, TurnRightYaw * InputDeltaTime, 0));
        // 지나갔을 경우 Desired로 세팅
        if (GameUtilities::IsVectorCCW2D(Player->GetRootComponent()->GetComponentTransform().GetRotation().Vector(), DesiredDirection) < 0)
        {
            Player->SetActorRotation(DesiredDirection.Rotation());
        }
    }
    else if (rotateDirection == 0)
    {
        FacingDirection.Normalize();
        DesiredDirection.Normalize();
        if (!FacingDirection.Equals(DesiredDirection))
        {
            // 반대 방향일 경우 좌우 지향 방향에 따라 회전방향 결정
            // 좌우 입력이 0일 경우 좌회전
            if (DesiredDirection.Y > 0)
                Player->AddActorLocalRotation(FRotator(0, TurnRightYaw * InputDeltaTime, 0));
            else
                Player->AddActorLocalRotation(FRotator(0, TurnLeftYaw * InputDeltaTime, 0));
        }
    }
    else
    {
        Player->AddActorLocalRotation(FRotator(0, TurnLeftYaw * InputDeltaTime, 0));
        // 지나갔을 경우 Desired로 세팅
        if (GameUtilities::IsVectorCCW2D(Player->GetRootComponent()->GetComponentTransform().GetRotation().Vector(), DesiredDirection) > 0)
        {
            Player->SetActorRotation(DesiredDirection.Rotation());
        }
    }
}
