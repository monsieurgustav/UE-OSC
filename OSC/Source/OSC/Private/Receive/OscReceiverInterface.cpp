#include "OscPrivatePCH.h"
#include "OscReceiverInterface.h"


UOscReceiverInterface::UOscReceiverInterface(const class FPostConstructInitializeProperties& PCIP)
: Super(PCIP)
{

}

const FString & IOscReceiverInterface::GetAddressFilter()
{
    static FString defaultValue;
    return defaultValue;
}

void IOscReceiverInterface::SendEvent(const FName & Address, const TArray<FOscDataElemStruct> & Data)
{ }
