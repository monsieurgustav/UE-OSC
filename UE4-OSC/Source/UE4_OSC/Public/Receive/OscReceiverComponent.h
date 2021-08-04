#pragma once

#include "Components/ActorComponent.h"
#include "Receive/OscReceiverInterface.h"
#include "Common/OscDataElemStruct.h"
#include "OscReceiverComponent.generated.h"


// declare the OnOscReceived event type
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FComponentOscReceivedSignature, const FName &, Address, const TArray<FOscDataElemStruct> &, Data, const FString &, SenderIp);


UCLASS(ClassGroup=OSC, meta = (BlueprintSpawnableComponent))
class UE4_OSC_API UOscReceiverComponent : public UActorComponent
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, Category=OSC)
    FString AddressFilter;

    UPROPERTY(BlueprintAssignable, Category=OSC)
    FComponentOscReceivedSignature OnOscReceived;

public:

    UOscReceiverComponent();

    /// Hot reload constructor
    UOscReceiverComponent(FVTableHelper & helper);

    const FString & GetAddressFilter() const
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
