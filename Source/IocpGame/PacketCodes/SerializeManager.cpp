// Fill out your copyright notice in the Description page of Project Settings.


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

bool SerializeManager::WriteDataToBuffer(TSharedPtr<SendBuffer> writeBuffer) const
{
	if (!writeBuffer->Write(Array->GetData(), (SIZE_T)Array->Num()))
	{
		// TODO: Scatter/Gather -> 데이터를 버퍼 여러 개로 쪼개 발송, 수신후 재조합
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
	if (!Array->IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("비어있지 않은 Array에 버퍼를 읽어들이려 하고 있습니다."));
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