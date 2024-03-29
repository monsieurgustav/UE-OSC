#pragma once

#include "Osc.h"
#include "Common/OscDataElemStruct.h"


/// Base class to receive OSC messages.
struct UE_OSC_API IOscReceiverInterface
{
    virtual ~IOscReceiverInterface()  {}
    virtual FName GetAddressFilter() const = 0;
    virtual void SendEvent(const FName & Address, const TArray<FOscDataElemStruct> & Data, const FString & SenderIp) = 0;
};


/// Forward calls to an impl object.
template <class T>
struct UE_OSC_API BasicOscReceiver : IOscReceiverInterface
{
    T * const _impl;

    BasicOscReceiver(T * impl) : _impl(impl)
    {
    }

    FName GetAddressFilter() const final
    {
        return _impl->GetAddressFilter();
    }

    void SendEvent(const FName & Address, const TArray<FOscDataElemStruct> & Data, const FString & SenderIp) final
    {
        _impl->SendEvent(Address, Data, SenderIp);
    }
};
