#include "OscPrivatePCH.h"
#include "OscReceiverComponent.h"
#include "OscDispatcher.h"


UOscReceiverComponent::UOscReceiverComponent(const class FPostConstructInitializeProperties& PCIP)
    : Super(PCIP)
{

}

void UOscReceiverComponent::OnRegister()
{
    Super::OnRegister();
    
    auto instance = UOscDispatcher::Get();
    if (instance && !HasAnyFlags(RF_ClassDefaultObject))
    {
        instance->RegisterReceiver(this);
    }
}

void UOscReceiverComponent::OnUnregister()
{
    auto instance = UOscDispatcher::Get();
    if (instance && !HasAnyFlags(RF_ClassDefaultObject))
    {
        instance->UnregisterReceiver(this);
    }

    Super::OnUnregister();
}
