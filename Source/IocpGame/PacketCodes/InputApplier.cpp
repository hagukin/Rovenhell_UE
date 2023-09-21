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

	// 모든 플레이어들의 인풋 처리
	for (TActorIterator<APlayerPawn> iter(GameInstance->GetWorld()); iter; ++iter)
	{
		// TODO: 세션 Id 값으로 어떤 플레이어인지 찾아내어야 한다
		for (const SD_GameInput& input : inputData->GameInputs)
		{
			if (input.ActionType == (uint32)ActionTypeEnum::MOVE)
			{
				(*iter)->Move_UEServer(FInputActionValue(FVector(input.X, input.Y, input.Z)), input.DeltaTime);

				uint64 sessionId = 1; ////////// TODO FIXME: 현재는 플레이어 한 명이라 무조건 1 전달, 수정 필요

				// 더 나중 틱으로 업데이트
				netHandler->UpdateLastProcessedInputTickForSession(sessionId, FMath::Max<uint32>(netHandler->GetLastProcessedInputTickForSession(sessionId), input.Tick));
			}
		}
	}
	return true;
}
