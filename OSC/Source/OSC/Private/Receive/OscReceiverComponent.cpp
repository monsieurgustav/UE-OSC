#include "OscPrivatePCH.h"
#include "OscReceiverComponent.h"
#include "OscDispatcher.h"


UOscReceiverComponent::UOscReceiverComponent()
    : _listener(this)
{

}

UOscReceiverComponent::UOscReceiverComponent(FVTableHelper & helper)
    : _listener(this)
{
    // Does not need to be a valid object.
}

void UOscReceiverComponent::OnRegister()
{
    Super::OnRegister();
    
    auto instance = UOscDispatcher::Get();
    if (instance && !HasAnyFlags(RF_ClassDefaultObject))
    {
        instance->RegisterReceiver(&_listener);
    }
}

void UOscReceiverComponent::OnUnregister()
{
    auto instance = UOscDispatcher::Get();
    if (instance && !HasAnyFlags(RF_ClassDefaultObject))
    {
        instance->UnregisterReceiver(&_listener);
    }

    Super::OnUnregister();
}
