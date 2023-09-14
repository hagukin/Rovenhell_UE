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

bool InputApplier::ApplyPacket(TSharedPtr<RecvBuffer> packet, TSharedPtr<SerializeManager> deserializer)
{
	URovenhellGameInstance* gameInstance = Cast<URovenhellGameInstance>(GameInstance);
	if (!gameInstance) return false;

	bool applied = true;
	switch (gameInstance->GetExecType()->GetHostType())
	{
	case HostTypeEnum::CLIENT:
	case HostTypeEnum::CLIENT_HEADLESS:
	{
		applied &= ApplyPacket_UEClient(packet, deserializer);
		break;
	}
	case HostTypeEnum::LOGIC_SERVER:
	case HostTypeEnum::LOGIC_SERVER_HEADLESS:
	{
		applied &= ApplyPacket_UEServer(packet, deserializer);
		break;
	}
	}
	return applied;
}

bool InputApplier::ApplyPacket_UEClient(TSharedPtr<RecvBuffer> packet, TSharedPtr<SerializeManager> deserializer)
{
	return false; // 클라이언트는 Input 패킷을 처리하지 않음
}

bool InputApplier::ApplyPacket_UEServer(TSharedPtr<RecvBuffer> packet, TSharedPtr<SerializeManager> deserializer)
{
	deserializer->Clear();
	SD_GameInputHistory* inputData = new SD_GameInputHistory();
	deserializer->ReadDataFromBuffer(packet);
	deserializer->Deserialize((SD_Data*)inputData);

	// 모든 플레이어들의 인풋 처리
	// TODO
	for (TActorIterator<APlayerPawn> iter(GameInstance->GetWorld()); iter; ++iter)
	{
		// TODO: 세션 Id 값으로 어떤 플레이어인지 찾아내어야 한다
		for (const SD_GameInput& input : inputData->GameInputs)
		{
			if (input.ActionType == (uint32)ActionTypeEnum::MOVE)
			{
				(*iter)->Move_UEServer(FInputActionValue(FVector(input.X, input.Y, input.Z)), input.DeltaTime);
			}
		}
	}
	return true;
}
