// Copyright 2023 Haguk Kim
// Author: Haguk Kim

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

	bool Serialize(SD_Data* inData);
	bool Deserialize(SD_Data* outData);

	bool WriteDataToBuffer(TSharedPtr<SendBuffer> writeBuffer) const;
	bool ReadDataFromBuffer(TSharedPtr<RecvBuffer> readBuffer);

private:
	
	TUniquePtr<TArray<uint8>> Array;
};
