// Copyright 2023 Haguk Kim
// Author: Haguk Kim

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class IOCPGAME_API NetMemory
{
public:
	NetMemory();
	~NetMemory();

	template<typename Type, typename... Args>
	static Type* Allocate(Args&&... args);
};