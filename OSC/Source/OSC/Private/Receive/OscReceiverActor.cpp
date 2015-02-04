#include "OscPrivatePCH.h"
#include "OscReceiverActor.h"
#include "OscDispatcher.h"


AOscReceiverActor::AOscReceiverActor(const class FPostConstructInitializeProperties& PCIP)
    : Super(PCIP),
      _listener(this)
{
    auto instance = UOscDispatcher::Get();
    if (instance && !HasAnyFlags(RF_ClassDefaultObject))
    {
        instance->RegisterReceiver(&_listener);
    }
}

void AOscReceiverActor::BeginDestroy()
{
    auto instance = UOscDispatcher::Get();
    if (instance && !HasAnyFlags(RF_ClassDefaultObject))
    {
        instance->UnregisterReceiver(&_listener);
    }

    Super::BeginDestroy();
}
