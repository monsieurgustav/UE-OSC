#pragma once

#include "InputCoreTypes.h"

#include "OscReceiverInterface.h"
#include "OscDataElemStruct.h"


class OscReceiverInputKey : public IOscReceiverInterface
{
public:
    OscReceiverInputKey(const FString & address);

    OscReceiverInputKey(const OscReceiverInputKey &) = delete;
    OscReceiverInputKey& operator=(const OscReceiverInputKey &) = delete;

private:

    /// Adds the custom input key to the Slate application.
    void RegisterKey() const;

    const FString & GetAddressFilter() const override
    {
        return _address;
    }

    void SendEvent(const FName & Address, const TArray<FOscDataElemStruct> & Data, const FString & SenderIp) override;

private:
    FString _address;
    FName _addressName;
    FKey _key;
};
