#include "OscPrivatePCH.h"


UOscSettings::UOscSettings( const class FPostConstructInitializeProperties& PCIP )
 :  Super(PCIP),
    ReceivePort(8000),
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
            int32 sep = 0;
            bool isValid = false;
            if(SendTargets[i].FindChar(TEXT(':'), sep))
            {
                const FString ip = SendTargets[i].Left(sep).Trim();
                _sendAddresses[i].second->SetIp(&ip.GetCharArray()[0], isValid);
                if(isValid)
                {
                    const int32 port = FCString::Atoi(&SendTargets[i].GetCharArray()[sep+1]);
                    _sendAddresses[i].second->SetPort(port);
                    _sendAddresses[i].first = SendTargets[i];
                }
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
