#include "OscPrivatePCH.h"
#include "OscReceiverActor.h"
#include "OscDispatcher.h"


AOscReceiverActor::AOscReceiverActor()
    : _listener(this)
{
    auto instance = UOscDispatcher::Get();
    if (instance && !HasAnyFlags(RF_ClassDefaultObject))
    {
        instance->RegisterReceiver(&_listener);
    }
}

AOscReceiverActor::AOscReceiverActor(FVTableHelper & helper)
    : _listener(this)
{
    // Does not need to be a valid object.
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
