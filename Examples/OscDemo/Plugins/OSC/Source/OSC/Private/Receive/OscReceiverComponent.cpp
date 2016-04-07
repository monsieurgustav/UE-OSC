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
    if (instance &&
        !HasAnyFlags(RF_ClassDefaultObject) &&
        (!GetOwner() || !GetOwner()->HasAnyFlags(RF_ClassDefaultObject)))
    {
        instance->RegisterReceiver(&_listener);

        if(GetOwner())
        {
            UE_LOG(LogOSC, Verbose, TEXT("Registering component %s of %s"),  *GetName(), *GetOwner()->GetName());
        }
        else
        {
            UE_LOG(LogOSC, Verbose, TEXT("Registering component %s"), *GetName());
        }
    }
}

void UOscReceiverComponent::OnUnregister()
{
    auto instance = UOscDispatcher::Get();
    if (instance &&
        !HasAnyFlags(RF_ClassDefaultObject) &&
        (!GetOwner() || !GetOwner()->HasAnyFlags(RF_ClassDefaultObject)))
    {
        instance->UnregisterReceiver(&_listener);

        if(GetOwner())
        {
            UE_LOG(LogOSC, Verbose, TEXT("Unregistering component %s of %s"),  *GetName(), *GetOwner()->GetName());
        }
        else
        {
            UE_LOG(LogOSC, Verbose, TEXT("Unregistering component %s"), *GetName());
        }
    }

    Super::OnUnregister();
}
