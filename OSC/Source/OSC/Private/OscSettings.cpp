#include "OscPrivatePCH.h"


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
                UE_LOG(LogOSC, Warning, TEXT("Fail to parse or invalid send address: %s"), *SendTargets[i]);
            }
        }
    }
}

void UOscSettings::Send(const uint8 *buffer, int32 length, int32 targetIndex)
{
    if(targetIndex < _sendAddresses.Num())
    {
        int32 bytesSent = 0;
        while(length > 0)
        {
            _sendSocket->SendTo(buffer, length, bytesSent, *_sendAddresses[targetIndex].second);
            length -= bytesSent;
            buffer += bytesSent;
        }
        UE_LOG(LogOSC, Verbose, TEXT("OSC sent"));
    }
    else
    {
        UE_LOG(LogOSC, Warning, TEXT("Cannot send OSC: invalid targetIndex %d"), targetIndex);
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
