#pragma once

#include "Osc.h"
#include "Receive/OscReceiverInterface.h"
#include "Common/OscDataElemStruct.h"

#include "InputCoreTypes.h"


class OscReceiverInputKey : public IOscReceiverInterface
{
public:
    OscReceiverInputKey(FName address);

    OscReceiverInputKey(const OscReceiverInputKey &) = delete;
    OscReceiverInputKey& operator=(const OscReceiverInputKey &) = delete;

private:

    /// Adds the custom input key to the Slate application.
    void RegisterKey() const;

    FName GetAddressFilter() const override
    {
        return _address;
    }

    void SendEvent(const FName & Address, const TArray<FOscDataElemStruct> & Data, const FString & SenderIp) override;

private:
    FName _address;
    FKey _key;
};
