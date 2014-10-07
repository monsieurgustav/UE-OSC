#include "OscPrivatePCH.h"
#include "OscReceiverActor.h"
#include "OscDispatcher.h"


AOscReceiverActor::AOscReceiverActor(const class FPostConstructInitializeProperties& PCIP)
    : Super(PCIP)
{
    auto instance = UOscDispatcher::Get();
    if (instance && !HasAnyFlags(RF_ClassDefaultObject))
    {
        instance->RegisterReceiver(this);
    }
}

void AOscReceiverActor::BeginDestroy()
{
    auto instance = UOscDispatcher::Get();
    if (instance && !HasAnyFlags(RF_ClassDefaultObject))
    {
        instance->UnregisterReceiver(this);
    }

    Super::BeginDestroy();
}
