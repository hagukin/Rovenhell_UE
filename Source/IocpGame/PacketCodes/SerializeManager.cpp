// Copyright 2023 Haguk Kim
// Author: Haguk Kim

#include "SerializeManager.h"

SerializeManager::SerializeManager()
{
}

SerializeManager::~SerializeManager()
{
}

bool SerializeManager::Init()
{
	Array = MakeUnique<TArray<uint8>>();
	return true;
}

void SerializeManager::Clear()
{
	Array->Empty();
}

bool SerializeManager::Serialize(SD_Data* inData)
{
	FMemoryWriter Writer(*Array);
	inData->Serialize(Writer);
	return true;
}

bool SerializeManager::Deserialize(SD_Data* outData)
{
	FMemoryReader Reader(*Array);
	outData->Deserialize(Reader);
	return true;
}

bool SerializeManager::WriteDataToBuffer(TSharedPtr<SendBuffer> writeBuffer, uint8* dataCursor, int32 dataSize) const
{
	if (!writeBuffer->Write(dataCursor, (SIZE_T)dataSize))
	{
		UE_LOG(LogTemp, Error, TEXT("직렬화된 데이터가 버퍼 크기를 초과해 Write 할 수 없습니다. 버퍼 크기: %i바이트, 데이터 크기: %i바이트"), writeBuffer->GetCapacity(), Array->Num());
		return false;
	};
	return true;
}

bool SerializeManager::ReadDataFromBuffer(TSharedPtr<RecvBuffer> readBuffer)
{
	if (!readBuffer)
	{
		UE_LOG(LogTemp, Warning, TEXT("빈 버퍼를 입력 받았습니다."));
		return false;
	}
	uint32 size = readBuffer->GetSize() - (uint32)sizeof(PacketHeader);
	Array->Reserve((int32)size);
	for (uint32 i = 0; i < size; ++i)
	{
		Array->Add(*(readBuffer->GetData() + i));
	}
	// FMemory::Memcpy(Array->GetData(), readBuffer->GetData(), static_cast<SIZE_T>(size)); // 알 수 없는 이유로 Memcpy로는 메모리 복사가 안됨; 
	// 1000만 바이트 이하에서는 퍼포먼스 차이가 미미하므로 일단 for 사용; TODO FIXME
	return true;
}

void SerializeManager::ResetPacketInfo()
{
	ReceivedPacketId = 0;
	ReceivedFragments.Reset();
}

bool SerializeManager::IsCorrectPacket(const PacketHeader* packetHeader)
{
	if (ReceivedPacketId != 0 && ReceivedPacketId != packetHeader->uniqueId)
	{
		UE_LOG(LogTemp, Error, TEXT("패킷 로스가 발생했거나, 혹은 알 수 없는 이유로 패킷 순서가 뒤바뀌었습니다! 기존 수신하던 패킷 fragment들은 삭제됩니다."));
		ResetPacketInfo();
		return false;
	}
	return true;
}

bool SerializeManager::SetPacketInfo(const PacketHeader* packetHeader)
{
	ReceivedPacketId = packetHeader->uniqueId;
	ReceivedFragments.Add(packetHeader->packetOrder, true);

	// TCP 패킷이므로 순서 보장
	if (packetHeader->packetOrder == packetHeader->fragmentCount - 1) // 인덱스 == size - 1
	{
		// 마지막 패킷 수신 완료
		ResetPacketInfo();
		return true;
	}
	return false;
}
