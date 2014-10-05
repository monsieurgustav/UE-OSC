#pragma once

#include "Components/ActorComponent.h"
#include "OscReceiverInterface.h"
#include "OscDataStruct.h"
#include "OscReceiverComponent.generated.h"


// declare the OnOscReceived event type
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FComponentOscReceivedSignature, const FName &, Address, const FOscDataStruct &, Data);


UCLASS(meta = (BlueprintSpawnableComponent))
class UOscReceiverComponent : public UActorComponent, public IOscReceiverInterface
{
    GENERATED_UCLASS_BODY()

    UPROPERTY(EditAnywhere, Category=OSC)
    FString AddressFilter;

    UPROPERTY(BlueprintAssignable, Category=OSC)
    FComponentOscReceivedSignature OnOscReceived;

protected:
    virtual void OnRegister() override;

    virtual void OnUnregister() override;
    
    virtual const FString & GetAddressFilter() override
    {
        return AddressFilter;
    }

    virtual void SendEvent(const FName & Address, const FOscDataStruct & Data) override
    {
        OnOscReceived.Broadcast(Address, Data);
    }
};
