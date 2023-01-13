#pragma once

#include "Osc.h"
#include "Receive/OscReceiverInterface.h"
#include "Common/OscDataElemStruct.h"

#include "Components/ActorComponent.h"
#include "OscReceiverComponent.generated.h"


// declare the OnOscReceived event type
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FComponentOscReceivedSignature, const FName &, Address, const TArray<FOscDataElemStruct> &, Data, const FString &, SenderIp);


UCLASS(ClassGroup=OSC, meta = (BlueprintSpawnableComponent))
class UE_OSC_API UOscReceiverComponent : public UActorComponent
{
    GENERATED_BODY()

public:

    /**
     *  Receive a message only if its address starts with the filter.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=OSC)
    FName AddressFilter;

    UPROPERTY(BlueprintAssignable, Category=OSC)
    FComponentOscReceivedSignature OnOscReceived;

public:

    UOscReceiverComponent();

    /// Hot reload constructor
    UOscReceiverComponent(FVTableHelper & helper);

    FName GetAddressFilter() const
    {
        return AddressFilter;
    }

    void SendEvent(const FName & Address, const TArray<FOscDataElemStruct> & Data, const FString & SenderIp)
    {
        OnOscReceived.Broadcast(Address, Data, SenderIp);
    }

private:
    void BeginPlay() override;

    void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    BasicOscReceiver<UOscReceiverComponent> _listener;
};
