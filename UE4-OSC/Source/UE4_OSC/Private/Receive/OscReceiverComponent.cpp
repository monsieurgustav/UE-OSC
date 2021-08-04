#include "Receive/OscReceiverComponent.h"
#include "Receive/OscDispatcher.h"


UOscReceiverComponent::UOscReceiverComponent()
    : _listener(this)
{

}

UOscReceiverComponent::UOscReceiverComponent(FVTableHelper & helper)
    : _listener(this)
{
}

void UOscReceiverComponent::BeginPlay()
{
    Super::BeginPlay();

    auto instance = UOscDispatcher::Get();
    if (instance &&
        !HasAnyFlags(RF_ClassDefaultObject) &&
        (!GetOwner() || !GetOwner()->HasAnyFlags(RF_ClassDefaultObject)))
    {
        instance->RegisterReceiver(&_listener);

        if(GetOwner())
        {
            UE_LOG(LogUE4_OSC, Verbose, TEXT("Registering component %s of %s"),  *GetName(), *GetOwner()->GetName());
        }
        else
        {
            UE_LOG(LogUE4_OSC, Verbose, TEXT("Registering component %s"), *GetName());
        }
    }
}

void UOscReceiverComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    auto instance = UOscDispatcher::Get();
    if (instance &&
        !HasAnyFlags(RF_ClassDefaultObject) &&
        (!GetOwner() || !GetOwner()->HasAnyFlags(RF_ClassDefaultObject)))
    {
        instance->UnregisterReceiver(&_listener);

        if(GetOwner())
        {
            UE_LOG(LogUE4_OSC, Verbose, TEXT("Unregistering component %s of %s"),  *GetName(), *GetOwner()->GetName());
        }
        else
        {
            UE_LOG(LogUE4_OSC, Verbose, TEXT("Unregistering component %s"), *GetName());
        }
    }

    Super::EndPlay(EndPlayReason);
}
