#pragma once

#include "Osc.h"
#include "OscDataElemStruct.h"
#include "OscMessageStruct.generated.h"


USTRUCT(BlueprintType)
struct UE_OSC_API FOscMessageStruct
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=OSC)
    FName Address;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=OSC)
    TArray<FOscDataElemStruct> Data;
};
