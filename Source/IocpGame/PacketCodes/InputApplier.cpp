// Fill out your copyright notice in the Description page of Project Settings.


#include "InputApplier.h"
#include "../IocpGameCharacter.h"

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
	SD_GameInput* inputData = new SD_GameInput();
	deserializer->ReadDataFromBuffer(packet);
	deserializer->Deserialize((SD_Data*)inputData);
	FVector2D vec(inputData->X, inputData->Y);

	UE_LOG(LogTemp, Warning, TEXT("%i: %f %f"), inputData->ActionType, inputData->X, inputData->Y);

	//////////// TESTING
	for (TActorIterator<AIocpGameCharacter> iter(GameInstance->GetWorld()); iter; ++iter)
	{
		if (inputData->ActionType == (uint32)ActionTypeEnum::MOVE)
		{
			(*iter)->Move_UEServer(FInputActionValue(vec));
		}
	}
	return true;
}
