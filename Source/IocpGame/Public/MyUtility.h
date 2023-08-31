// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"

// 언리얼 BytesToString 에러 해결 - UTF16 인코딩 관련해서 문제가 발생한다
inline FString MyBytesToString(const uint8* In, int32 Count)
{
    FString Result;
    Result.Empty(Count);

    while (Count)
    {
        int16 Value = *In;
        Result += TCHAR(Value);
        ++In;
        Count--;
    }
    return Result;
}