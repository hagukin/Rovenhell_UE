// Fill out your copyright notice in the Description page of Project Settings.


#include "InputApplier.h"
#include "PlayerPawn.h"

InputApplier::InputApplier()
{
}

InputApplier::~InputApplier()
{
}

bool InputApplier::Init(TSharedPtr<NetSession> session, UGameInstance* gameInstance)
{
	if (!PacketApplier::Init(session, gameInstance)) return false;
	return true;
}

bool InputApplier::ApplyPacket(TSharedPtr<RecvBuffer> packet, class ANetHandler* netHandler)
{
	URovenhellGameInstance* gameInstance = Cast<URovenhellGameInstance>(GameInstance);
	if (!gameInstance) return false;

	bool applied = true;
	switch (gameInstance->GetExecType()->GetHostType())
	{
	case HostTypeEnum::CLIENT:
	case HostTypeEnum::CLIENT_HEADLESS:
	{
		applied &= ApplyPacket_UEClient(packet, netHandler);
		break;
	}
	case HostTypeEnum::LOGIC_SERVER:
	case HostTypeEnum::LOGIC_SERVER_HEADLESS:
	{
		applied &= ApplyPacket_UEServer(packet, netHandler);
		break;
	}
	}
	return applied;
}

bool InputApplier::ApplyPacket_UEClient(TSharedPtr<RecvBuffer> packet, class ANetHandler* netHandler)
{
	return false; // 클라이언트는 Input 패킷을 처리하지 않음
}

bool InputApplier::ApplyPacket_UEServer(TSharedPtr<RecvBuffer> packet, class ANetHandler* netHandler)
{
	netHandler->GetDeserializerShared()->Clear();
	SD_GameInputHistory* inputData = new SD_GameInputHistory();
	netHandler->GetDeserializerShared()->ReadDataFromBuffer(packet);
	netHandler->GetDeserializerShared()->Deserialize((SD_Data*)inputData);
	uint64 sessionId = ((PacketHeader*)packet->GetBuf())->senderId;

	// 모든 플레이어들의 인풋 처리
	for (const SD_GameInput& input : inputData->GameInputs)
	{
		if (input.ActionType == (uint32)ActionTypeEnum::MOVE)
		{
			APlayerController* controller = netHandler->GetRovenhellGameInstance()->GetPlayerControllerOfOwner(sessionId);
			if (!controller)
			{ 
				UE_LOG(LogTemp, Error, TEXT("세션 아이디 %i가 등록되어있지 않아 인풋을 처리할 수 없습니다."), sessionId);
				return false;
			}
			APlayerPawn* playerPawn = Cast<APlayerPawn>(controller->GetPawn());
			if (!playerPawn)
			{
				UE_LOG(LogTemp, Error, TEXT("인풋 처리의 대상을 플레이어 폰으로 캐스팅 할 수 없습니다."), sessionId);
				return false;
			}
			playerPawn->Move_UEServer(FInputActionValue(FVector(input.X, input.Y, input.Z)), input.DeltaTime);

			// 더 나중 틱으로 업데이트
			netHandler->UpdateLastProcessedInputTickForSession(sessionId, FMath::Max<uint32>(netHandler->GetLastProcessedInputTickForSession(sessionId), input.Tick));
		}
	}
	return true;
}
