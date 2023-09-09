// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SerializableData.h"
#include "HAL/UnrealMemory.h"
#include "../NetCodes/NetBuffer.h"

/**
 * 
 */
class IOCPGAME_API SerializeManager
{
private:

public:
	SerializeManager();
	~SerializeManager();
	bool Init();
	void Clear();

	bool SerializeActor(SD_Actor* inData); // TEST
	bool DeserializeActor(SD_Actor* outData); // TEST
	bool WriteDataToBuffer(TSharedPtr<SendBuffer> writeBuffer); // TEST
	bool ReadDataFromBuffer(TSharedPtr<RecvBuffer> readBuffer); // TEST

private:
	TUniquePtr<TArray<uint8>> Array;
};
