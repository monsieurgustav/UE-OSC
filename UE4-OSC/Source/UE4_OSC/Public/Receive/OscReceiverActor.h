#pragma once

#include "GameFramework/Actor.h"
#include "Receive/OscReceiverInterface.h"
#include "Common/OscDataElemStruct.h"
#include "OscReceiverActor.generated.h"


UCLASS(ClassGroup=OSC)
class UE4_OSC_API AOscReceiverActor : public AActor
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, Category=OSC)
    FString AddressFilter;

    UFUNCTION(BlueprintImplementableEvent, Category=OSC)
    void OnOscReceived(const FName & Address, const TArray<FOscDataElemStruct> & Data, const FString & SenderIp);

public:

    AOscReceiverActor();

    /// Hot reload constructor
    AOscReceiverActor(FVTableHelper & helper);

    const FString & GetAddressFilter() const
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
