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

        UE_LOG(LogOSC, Verbose, TEXT("Registering actor %s"), *GetName());
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

        UE_LOG(LogOSC, Verbose, TEXT("Unregistering actor %s"), *GetName());
    }

    Super::BeginDestroy();
}
