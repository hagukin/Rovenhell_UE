// Fill out your copyright notice in the Description page of Project Settings.


#include "NetPlayerMovementComponent.h"
#include "PlayerPawn.h"
#include "GameUtilities.h"

void UNetPlayerMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    BeginTick();

    APlayerPawn* Player = Cast<APlayerPawn>(PawnOwner);
    if (!Player || !UpdatedComponent) return;

    // 인풋 처리
    // 서버의 경우 단일 폰에 대해 1틱에 여러 인풋을 처리할 수 있는데,
    // 각 인풋별로 발송받은 델타 타임을 적용해 연산한다.
    FVector DesiredMovementThisFrame(0, 0, 0);
    for (const MoveInputData& DataPerInput : MoveDatas)
    {
        FVector CurrentDirection = Player->GetRootComponent()->GetComponentTransform().GetRotation().Vector();

        // 현재 바라보고 있는 방향으로 이동 처리
        DesiredMovementThisFrame = CurrentDirection.GetClampedToMaxSize(1.0f) * 300.0f * DataPerInput.DeltaTime; // 델타 타임 반영
        Player->SetActorLocation(Player->GetActorLocation() + DesiredMovementThisFrame, false, nullptr, ETeleportType::None);

        // 회전 처리
        DesiredDirection = DataPerInput.MoveVector;

        float rotateDirection = GameUtilities::IsVectorCCW2D(CurrentDirection, DesiredDirection);
        if (rotateDirection > 0) // 언리얼의 xy 평면은 x가 세로방향이기 때문에 주의 필요
        {
            Player->AddActorLocalRotation(FRotator(0, TurnRightYaw * DataPerInput.DeltaTime, 0));
            // 지나갔을 경우 Desired로 세팅
            if (GameUtilities::IsVectorCCW2D(Player->GetRootComponent()->GetComponentTransform().GetRotation().Vector(), DesiredDirection) < 0)
            {
                Player->SetActorRotation(DesiredDirection.Rotation());
            }
        }
        else if (rotateDirection == 0)
        {
            CurrentDirection.Normalize();
            DesiredDirection.Normalize();
            if (!CurrentDirection.Equals(DesiredDirection))
            {
                // 반대 방향일 경우 좌우 지향 방향에 따라 회전방향 결정
                // 좌우 입력이 0일 경우 좌회전
                if (DesiredDirection.Y > 0)
                    Player->AddActorLocalRotation(FRotator(0, TurnRightYaw * DataPerInput.DeltaTime, 0));
                else
                    Player->AddActorLocalRotation(FRotator(0, TurnLeftYaw * DataPerInput.DeltaTime, 0));
            }
        }
        else
        {
            Player->AddActorLocalRotation(FRotator(0, TurnLeftYaw * DataPerInput.DeltaTime, 0));
            // 지나갔을 경우 Desired로 세팅
            if (GameUtilities::IsVectorCCW2D(Player->GetRootComponent()->GetComponentTransform().GetRotation().Vector(), DesiredDirection) > 0)
            {
                Player->SetActorRotation(DesiredDirection.Rotation());
            }
        }
    }

    EndTick();
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

void UNetPlayerMovementComponent::EndTick()
{
    MoveDatas.Empty();
}

void UNetPlayerMovementComponent::AddMovementData(FVector MoveVector, float DeltaTime)
{
    MoveDatas.Add({ MoveVector, DeltaTime });
}
