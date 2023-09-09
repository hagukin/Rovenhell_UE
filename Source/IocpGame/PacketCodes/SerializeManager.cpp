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

bool SerializeManager::SerializeTransform(SD_Transform* inData)
{
	FMemoryWriter Writer(*Array);
	Writer << *inData;
	return true;
}

bool SerializeManager::DeserializeTransform(SD_Transform* outData)
{
	FMemoryReader Reader(*Array);
	Reader << *outData;
	return true;
}

bool SerializeManager::WriteDataToBuffer(TSharedPtr<SendBuffer> writeBuffer)
{
	writeBuffer->Write(Array->GetData(), (SIZE_T)Array->Num());
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