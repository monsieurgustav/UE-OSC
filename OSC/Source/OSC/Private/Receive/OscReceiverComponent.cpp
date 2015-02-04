#include "OscPrivatePCH.h"
#include "OscReceiverComponent.h"
#include "OscDispatcher.h"


UOscReceiverComponent::UOscReceiverComponent(const class FPostConstructInitializeProperties& PCIP)
    : Super(PCIP),
      _listener(this)
{

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
