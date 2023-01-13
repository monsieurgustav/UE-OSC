#pragma once

#include "Osc.h"
#include "Receive/OscReceiverInterface.h"
#include "Common/OscDataElemStruct.h"

#include "GameFramework/Actor.h"
#include "OscReceiverActor.generated.h"


UCLASS(ClassGroup=OSC)
class UE_OSC_API AOscReceiverActor : public AActor
{
    GENERATED_BODY()

public:

    /**
     *  Receive a message only if its address starts with the filter.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=OSC)
    FName AddressFilter;

    UFUNCTION(BlueprintImplementableEvent, Category=OSC)
    void OnOscReceived(const FName & Address, const TArray<FOscDataElemStruct> & Data, const FString & SenderIp);

public:

    AOscReceiverActor();

    /// Hot reload constructor
    AOscReceiverActor(FVTableHelper & helper);

    FName GetAddressFilter() const
    {
        return AddressFilter;
    }

    void SendEvent(const FName & Address, const TArray<FOscDataElemStruct> & Data, const FString & SenderIp)
    {
        OnOscReceived(Address, Data, SenderIp);
    }

private:
    void BeginPlay() override;

    void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    BasicOscReceiver<AOscReceiverActor> _listener;
};
