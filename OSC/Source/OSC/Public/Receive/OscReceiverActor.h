#pragma once

#include "GameFramework/Actor.h"
#include "OscReceiverInterface.h"
#include "OscDataElemStruct.h"
#include "OscReceiverActor.generated.h"


UCLASS(ClassGroup=OSC)
class OSC_API AOscReceiverActor : public AActor
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
    void BeginDestroy() override;
    
private:
    BasicOscReceiver<AOscReceiverActor> _listener;
};
