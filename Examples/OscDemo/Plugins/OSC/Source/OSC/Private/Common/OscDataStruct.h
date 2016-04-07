#pragma once

#include "OscDataElemStruct.h"
#include "OscDataStruct.generated.h"


USTRUCT()
struct FOscDataStruct
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY()
    int32 Index;

    UPROPERTY()
    TArray<FOscDataElemStruct> Queue;

    FOscDataStruct() : Index(0)
    { }

    FOscDataElemStruct Pop()
    {
        const auto idx = std::max(Index, 0);
        if(idx < Queue.Num())
        {
            ++Index;
            return Queue[idx];
        }
        return FOscDataElemStruct();
    }
};
