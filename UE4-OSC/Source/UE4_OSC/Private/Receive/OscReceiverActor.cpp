#include "Receive/OscReceiverActor.h"
#include "Receive/OscDispatcher.h"


AOscReceiverActor::AOscReceiverActor()
    : _listener(this)
{
}

AOscReceiverActor::AOscReceiverActor(FVTableHelper& helper)
    : _listener(this)
{
}

void AOscReceiverActor::BeginPlay()
{
    Super::BeginPlay();

    auto instance = UOscDispatcher::Get();
    if (instance && !HasAnyFlags(RF_ClassDefaultObject))
    {
        instance->RegisterReceiver(&_listener);

        UE_LOG(LogUE4_OSC, Verbose, TEXT("Registering actor %s"), *GetName());
    }
}

void AOscReceiverActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    auto instance = UOscDispatcher::Get();
    if (instance && !HasAnyFlags(RF_ClassDefaultObject))
    {
        instance->UnregisterReceiver(&_listener);

        UE_LOG(LogUE4_OSC, Verbose, TEXT("Unregistering actor %s"), *GetName());
    }

    Super::EndPlay(EndPlayReason);
}
