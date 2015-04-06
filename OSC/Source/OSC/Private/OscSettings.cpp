#include "OscPrivatePCH.h"

#include "OscSettings.h"
#include "OscDispatcher.h"


UOscSettings::UOscSettings( const class FPostConstructInitializeProperties& PCIP )
 :  Super(PCIP),
    ReceiveFrom("8000"),
    _sendSocket(FUdpSocketBuilder(TEXT("OscSender")).Build())
{
    SendTargets.Add(TEXT("127.0.0.1:8000"));
}

void UOscSettings::UpdateSendAddresses()
{
    FString addressStr, portStr;
    for(int32 i=0, n=SendTargets.Num(); i!=n; ++i)
    {
        if(i >= _sendAddresses.Num())
        {
            _sendAddresses.Emplace(FString(), ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr());
        }

        if(_sendAddresses[i].first != SendTargets[i])
        {
            FIPv4Address address(0);
            uint32_t port;
            if(Parse(SendTargets[i], &address, &port) && address != FIPv4Address::Any)
            {
                _sendAddresses[i].second->SetIp(address.GetValue());
                _sendAddresses[i].second->SetPort(port);

                _sendAddresses[i].first = SendTargets[i];
            }
            else
            {
                UE_LOG(LogOSC, Error, TEXT("Fail to parse or invalid send address: %s"), *SendTargets[i]);
            }
        }
    }
}

static void SendImpl(FSocket *socket, const uint8 *buffer, int32 length, const FInternetAddr & target)
{
    int32 bytesSent = 0;
    while(length > 0)
    {
        socket->SendTo(buffer, length, bytesSent, target);
        length -= bytesSent;
        buffer += bytesSent;
    }
}

void UOscSettings::Send(const uint8 *buffer, int32 length, int32 targetIndex)
{
    if(targetIndex == -1)
    {
        for(const auto & address : _sendAddresses)
        {
            SendImpl(&_sendSocket.Get(), buffer, length, *address.second);
        }

        // Log sent packet
        if(!LogOSC.IsSuppressed(ELogVerbosity::Verbose))
        {
            TArray<uint8> tmp;
            tmp.Append(buffer, length);
            const auto encoded = FBase64::Encode(tmp);
            UE_LOG(LogOSC, Verbose, TEXT("SentAll: %s"), *encoded);
        }
    }
    else if(targetIndex < _sendAddresses.Num())
    {
        SendImpl(&_sendSocket.Get(), buffer, length, *_sendAddresses[targetIndex].second);

        // Log sent packet
        if(!LogOSC.IsSuppressed(ELogVerbosity::Verbose))
        {
            TArray<uint8> tmp;
            tmp.Append(buffer, length);
            const auto encoded = FBase64::Encode(tmp);
            const auto target  = _sendAddresses[targetIndex].second->ToString(true);
            UE_LOG(LogOSC, Verbose, TEXT("SentTo %s: %s"), *target, *encoded);
        }
    }
    else
    {
        UE_LOG(LogOSC, Error, TEXT("Cannot send OSC: invalid targetIndex %d"), targetIndex);
    }
}

bool UOscSettings::Parse(const FString & address_port, FIPv4Address * address, uint32_t * port)
{
    if(address_port.IsEmpty())
    {
        return false;
    }

    FIPv4Address addressResult(0);
    uint32_t portResult;

    int32 sep = -1;
    if(address_port.FindChar(TEXT(':'), sep))
    {
        const auto ip = address_port.Left(sep).Trim();
        if(!FIPv4Address::Parse(ip, addressResult))
        {
            return false;
        }
    }
    
    portResult = FCString::Atoi(&address_port.GetCharArray()[sep+1]);
    if(portResult == 0)
    {
        return false;
    }

    *address = addressResult;
    *port = portResult;
    return true;
}

void UOscSettings::ClearKeyInputs(UOscDispatcher & dispatcher)
{
    for(auto & receiver : _keyReceivers)
    {
        // Unregister here, not in the OscReceiverInputKey destructor
        // because it would crash at the application exit.
        dispatcher.UnregisterReceiver(&receiver);
    }
    _keyReceivers.Reset(0);
}

void UOscSettings::UpdateKeyInputs(UOscDispatcher & dispatcher)
{
    ClearKeyInputs(dispatcher);
    for(const auto & address : Inputs)
    {
        _keyReceivers.Emplace(address);
        dispatcher.RegisterReceiver(&_keyReceivers.Last());
    }
}
