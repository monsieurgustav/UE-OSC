#include "OscPrivatePCH.h"

#include "OscSettings.h"
#include "OscDispatcher.h"


UOscSettings::UOscSettings()
 :  ReceiveFrom("8000"),
    MulticastLoopback(true),
    _sendSocket(FUdpSocketBuilder(TEXT("OscSender")).Build())
{
    SendTargets.Add(TEXT("127.0.0.1:8000"));
}

UOscSettings::UOscSettings(FVTableHelper & helper)
 :  _sendSocket(FUdpSocketBuilder(TEXT("OscSender")).Build())
{
    // Does not need to be a valid object.
}

void UOscSettings::InitSendTargets()
{
    UE_LOG(LogOSC, Display, TEXT("Send targets cleared"));

    FString addressStr, portStr;

    _sendAddresses.Empty();
    _sendAddresses.Reserve(SendTargets.Num());

    _sendAddressesIndex.Empty();
    _sendAddressesIndex.Reserve(SendTargets.Num());

    for(int32 i=0, n=SendTargets.Num(); i!=n; ++i)
    {
        const auto result = AddSendTarget(SendTargets[i]);
        check(result == i);
    }
}

int32 UOscSettings::GetOrAddSendTarget(const FString & ip_port)
{
    const int32 * result = _sendAddressesIndex.Find(ip_port);
    if(result)
    {
        return *result;
    }

    return AddSendTarget(ip_port);
}

int32 UOscSettings::AddSendTarget(const FString & ip_port)
{
    auto target = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

    FIPv4Address address(0);
    uint32_t port;
    if(Parse(ip_port, &address, &port, ParseOption::Strict))
    {
        target->SetIp(address.Value);
        target->SetPort(port);
        UE_LOG(LogOSC, Display, TEXT("Send target added: %s"), *ip_port);
    }
    else
    {
        UE_LOG(LogOSC, Error, TEXT("Fail to parse or invalid send target: %s"), *ip_port);
    }

    const auto result = _sendAddresses.Num();
    _sendAddressesIndex.Emplace(ip_port, result);
    _sendAddresses.Emplace(target);
    return result;
}

static bool SendImpl(FSocket *socket, const uint8 *buffer, int32 length, const FInternetAddr & target)
{
    int32 bytesSent = 0;
    while(length > 0)
    {
        socket->SendTo(buffer, length, bytesSent, target);
        if( bytesSent < 0 )
        {
            return false;
        }
        length -= bytesSent;
        buffer += bytesSent;
    }

    return true;
}

void UOscSettings::Send(const uint8 *buffer, int32 length, int32 targetIndex)
{
    if(targetIndex == -1)
    {
        bool error = false;
        for(const auto & address : _sendAddresses)
        {
            if(!SendImpl(&_sendSocket.Get(), buffer, length, *address))
            {
                const auto target = address->ToString(true);
                UE_LOG(LogOSC, Error, TEXT("Cannot send OSC: %s : socket cannot send data"), *target);
                error = true;
            }
        }

#if !NO_LOGGING
        // Log sent packet
        if(!error && !LogOSC.IsSuppressed(ELogVerbosity::Verbose))
        {
            TArray<uint8> tmp;
            tmp.Append(buffer, length);
            const auto encoded = FBase64::Encode(tmp);
            UE_LOG(LogOSC, Verbose, TEXT("SentAll: %s"), *encoded);
        }
#endif
    }
    else if(targetIndex < _sendAddresses.Num())
    {
        bool error = false;
        if(!SendImpl(&_sendSocket.Get(), buffer, length, *_sendAddresses[targetIndex]))
        {
            const auto target = _sendAddresses[targetIndex]->ToString(true);
            UE_LOG(LogOSC, Error, TEXT("Cannot send OSC: %s : socket cannot send data"), *target);
            error = true;
        }

#if !NO_LOGGING
        // Log sent packet
        if(!error && !LogOSC.IsSuppressed(ELogVerbosity::Verbose))
        {
            TArray<uint8> tmp;
            tmp.Append(buffer, length);
            const auto encoded = FBase64::Encode(tmp);
            const auto target  = _sendAddresses[targetIndex]->ToString(true);
            UE_LOG(LogOSC, Verbose, TEXT("SentTo %s: %s"), *target, *encoded);
        }
#endif
    }
    else
    {
        UE_LOG(LogOSC, Error, TEXT("Cannot send OSC: invalid targetIndex %d"), targetIndex);
    }
}

bool UOscSettings::Parse(const FString & ip_port, FIPv4Address * address, uint32_t * port, ParseOption option)
{
    if(ip_port.IsEmpty())
    {
        return false;
    }

    FIPv4Address addressResult = FIPv4Address::Any;
    uint32_t portResult = 0;

    int32 sep = -1;
    const bool hasSep = ip_port.FindChar(TEXT(':'), sep);

    if(hasSep)
    {
        portResult = FCString::Atoi(&ip_port.GetCharArray()[sep+1]);
        if(portResult == 0)
        {
            return false;
        }

        const auto ip = ip_port.Left(sep).Trim();
        if(!FIPv4Address::Parse(ip, addressResult))
        {
            return false;
        }
    }
    else
    {
        if(option == ParseOption::Strict)
        {
            return false;
        }
        else if(option == ParseOption::OptionalAddress)
        {
            portResult = FCString::Atoi(ip_port.GetCharArray().GetData());
            if(portResult == 0)
            {
                return false;
            }
        }
        else if(option == ParseOption::OptionalPort)
        {
            if(!FIPv4Address::Parse(ip_port, addressResult))
            {
                return false;
            }
        }
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
        dispatcher.UnregisterReceiver(receiver.get());
    }
    _keyReceivers.Reset(0);
}

void UOscSettings::UpdateKeyInputs(UOscDispatcher & dispatcher)
{
    ClearKeyInputs(dispatcher);
    for(const auto & address : Inputs)
    {
        auto receiver = std::make_unique<OscReceiverInputKey>(address);
        dispatcher.RegisterReceiver(receiver.get());
        _keyReceivers.Add(std::move(receiver));
    }
}

#if WITH_EDITOR
void UOscSettings::PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent)
{
    static const FName SendTargetsName("SendTargets");

    if( PropertyChangedEvent.GetPropertyName() == SendTargetsName )
    {
        for(auto & target : SendTargets)
        {
            FIPv4Address address;
            uint32_t port;
            if( !Parse(target, &address, &port, ParseOption::OptionalPort) || address == FIPv4Address::Any )
            {
                target = "127.0.0.1:8000";
            }
            else if( port == 0 )
            {
                target = address.ToString() + ":8000";
            }
        }
    }

    Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif
