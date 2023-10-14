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

	bool WriteDataToBuffer(TSharedPtr<SendBuffer> writeBuffer, uint8* dataCursor, int32 dataSize) const;
	bool ReadDataFromBuffer(TSharedPtr<RecvBuffer> readBuffer);
	void ResetPacketInfo();
	bool IsCorrectPacket(const PacketHeader* packetHeader); // 반환값: 현재 이 패킷을 수신하는 것이 순서상 올바른가; false일 경우 기존에 역직렬화한 데이터들에 대한 조치가 필요
	bool SetPacketInfo(const PacketHeader* packetHeader); // 반환값: 모든 패킷 fragment를 다 수신했는가

public:
	TUniquePtr<TArray<uint8>> Array;

private:
	uint8 ReceivedPacketId; // 현재 수신 중인 패킷의 UniqueId
	TMap<uint8, bool> ReceivedFragments; // 수신한 패킷 조각들
};
