#include "OscSettings.h"
#include "Receive/OscDispatcher.h"

#include "Runtime/Core/Public/Misc/Base64.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "Common/UdpSocketBuilder.h"

#include "Sockets.h"


UOscSettings::UOscSettings()
 :  MulticastLoopback(true)
{
    FIPv4Endpoint::Initialize();
    _defaultSendSocket = MakeShareable(FUdpSocketBuilder(TEXT("OscSender")).Build());
    _defaultSocketSender = MakeShared<FUdpSocketSender>(_defaultSendSocket.Get(), TEXT("OSCSender"));
    ReceiveFrom.Add(TEXT("8000"));
    SendTargets.Add(TEXT("127.0.0.1:8000"));
}

UOscSettings::UOscSettings(FVTableHelper & helper)
{
    // Does not need to be a valid object.
}

void UOscSettings::InitSendTargets()
{
    UE_LOG(LogUE_OSC, Display, TEXT("Send targets cleared"));

    _sendAddressAndSenders.Empty();
    _sendAddressAndSenders.Reserve(SendTargets.Num());

    _sendAddressesIndex.Empty();
    _sendAddressesIndex.Reserve(SendTargets.Num());

    _multicastInterfaceSenders.Empty();

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
    FIPv4Endpoint target;
    FUdpSocketSender* sender = _defaultSocketSender.Get();
    FIPv4Address address(0);
    uint32_t port;
    FIPv4Address multicastAddress(0);
    if(Parse(ip_port, &address, &port, &multicastAddress, ParseOption::Strict))
    {
        if(multicastAddress.IsMulticastAddress())
        {
            target.Address = multicastAddress;
            sender = GetOrAddMulticastInterfaceSender(address);
        }
        else
        {
            target.Address = address;
        }
        target.Port = port;
        UE_LOG(LogUE_OSC, Display, TEXT("Send target added: %s"), *ip_port);
    }
    else
    {
        UE_LOG(LogUE_OSC, Error, TEXT("Fail to parse or invalid send target: %s"), *ip_port);
    }

    const auto result = _sendAddressAndSenders.Num();
    _sendAddressesIndex.Emplace(ip_port, result);
    _sendAddressAndSenders.Emplace(target, sender);
    return result;
}

FUdpSocketSender* UOscSettings::GetOrAddMulticastInterfaceSender(const FIPv4Address& multicastInterface)
{
    if(multicastInterface == FIPv4Address::Any)
    {
        return _defaultSocketSender.Get();
    }
    else
    {
        for(const auto & item : _multicastInterfaceSenders)
        {
            if(item.Interface == multicastInterface)
            {
                return item.Sender.Get();
            }
        }

        // add
        TSharedPtr<FSocket> socket = MakeShareable(FUdpSocketBuilder(TEXT("OscSenderMulticast")).WithMulticastInterface(multicastInterface).Build());
        TSharedPtr<FUdpSocketSender> sender = MakeShared<FUdpSocketSender>(socket.Get(), TEXT("OSCSenderMulticast"));
        _multicastInterfaceSenders.Add({ multicastInterface, socket, sender });
        return sender.Get();
    }
}

void UOscSettings::Send(const uint8 *buffer, int32 length, int32 targetIndex)
{
    auto data = MakeShared<TArray<uint8>, ESPMode::ThreadSafe>(buffer, length);

    if (targetIndex == -1)
    {
        bool error = false;
        for(const auto& item : _sendAddressAndSenders)
        {
            if(!item.Value->Send(data, item.Key))
            {
                const auto target = item.Key.ToString();
                UE_LOG(LogUE_OSC, Error, TEXT("Cannot send OSC: %s : socket cannot send data"), *target);
                error = true;
            }
        }

#if !NO_LOGGING
        // Log sent packet
        if(!error && !LogUE_OSC.IsSuppressed(ELogVerbosity::Verbose))
        {
            const auto encoded = FBase64::Encode(buffer, length);
            UE_LOG(LogUE_OSC, Verbose, TEXT("SentAll: %s"), *encoded);
        }
#endif
    }
    else if(targetIndex < _sendAddressAndSenders.Num())
    {
        bool error = false;
        const auto& item = _sendAddressAndSenders[targetIndex];
        if(!item.Value->Send(data, item.Key))
        {
            const auto target = item.Key.ToString();
            UE_LOG(LogUE_OSC, Error, TEXT("Cannot send OSC: %s : socket cannot send data"), *target);
            error = true;
        }

#if !NO_LOGGING
        // Log sent packet
        if(!error && !LogUE_OSC.IsSuppressed(ELogVerbosity::Verbose))
        {
            const auto encoded = FBase64::Encode(buffer, length);
            const auto target = item.Key.ToString();
            UE_LOG(LogUE_OSC, Verbose, TEXT("SentTo %s: %s"), *target, *encoded);
        }
#endif
    }
    else
    {
        UE_LOG(LogUE_OSC, Error, TEXT("Cannot send OSC: invalid targetIndex %d"), targetIndex);
    }
}

namespace
{
    bool ParseIpAndMulticast(const FString& addressStr, FIPv4Address* address, FIPv4Address* multicastAddress)
    {
        FIPv4Address addressResult = FIPv4Address::Any;
        FIPv4Address multicastAddressResult = FIPv4Address::Any;

        int32 multicastSep = -1;
        const bool hasMulticastSep = addressStr.FindChar(TEXT('/'), multicastSep);
        if (hasMulticastSep)
        {
            const auto multicastIp = addressStr.Left(multicastSep).TrimStartAndEnd();
            const auto interfaceIp = addressStr.RightChop(multicastSep+1).TrimStartAndEnd();
            if (!FIPv4Address::Parse(multicastIp, multicastAddressResult))
            {
                return false;
            }

            if (interfaceIp.EndsWith(".255"))
            {
                // look up actual network interface using a mask
                FIPv4SubnetMask mask;
                if (FIPv4SubnetMask::Parse(interfaceIp, mask))
                {
                    TArray<TSharedPtr<FInternetAddr>> localAdapterAddresses;
                    if (ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLocalAdapterAddresses(localAdapterAddresses))
                    {
                        for (const auto& localAdapterAddress : localAdapterAddresses)
                        {
                            uint32 localAdapterIp = 0;
                            localAdapterAddress->GetIp(localAdapterIp);

                            const FIPv4Address localAdapterIpv4(localAdapterIp);
                            if ((localAdapterIpv4 & mask) == localAdapterIpv4)
                            {
                                addressResult = localAdapterIpv4;
                                break;
                            }
                        }
                    }
                }

                if (addressResult == FIPv4Address::Any)
                {
                    UE_LOG(LogUE_OSC, Error, TEXT("Cannot parse multicast address: no network interface matches the provided mask %s."), *interfaceIp);
                    return false;
                }
            }
            else
            {
                if (!FIPv4Address::Parse(interfaceIp, addressResult))
                {
                    return false;
                }
            }
        }
        else
        {
            if (!FIPv4Address::Parse(addressStr, addressResult))
            {
                return false;
            }

            if (addressResult.IsMulticastAddress())
            {
                using std::swap;
                swap(addressResult, multicastAddressResult);
            }
        }

        *address = addressResult;
        *multicastAddress = multicastAddressResult;
        return true;
    }
}

bool UOscSettings::Parse(const FString & ip_port, FIPv4Address * address, uint32_t * port, FIPv4Address* multicastAddress, ParseOption option)
{
    if(ip_port.IsEmpty())
    {
        return false;
    }

    FIPv4Address addressResult = FIPv4Address::Any;
    uint32_t portResult = 0;
    FIPv4Address multicastAddressResult = FIPv4Address::Any;

    int32 portSep = -1;
    const bool hasPortSep = ip_port.FindChar(TEXT(':'), portSep);

    if(hasPortSep)
    {
        portResult = FCString::Atoi(&ip_port.GetCharArray()[portSep+1]);
        if(portResult == 0)
        {
            return false;
        }

        const auto ip = ip_port.Left(portSep).TrimStartAndEnd();
        if(!ParseIpAndMulticast(ip, &addressResult, &multicastAddressResult))
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
            if (!ParseIpAndMulticast(ip_port, &addressResult, &multicastAddressResult))
            {
                return false;
            }
        }
    }

    *address = addressResult;
    *port = portResult;
    *multicastAddress = multicastAddressResult;
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
    static const FName ReceiveFromName("ReceiveFrom");
    static const FName SendTargetsName("SendTargets");

    if( PropertyChangedEvent.GetPropertyName() == ReceiveFromName )
    {
        for(auto & target : ReceiveFrom)
        {
            FIPv4Address address;
            uint32_t port;
            FIPv4Address multicastAddress;
            if( !Parse(target, &address, &port, &multicastAddress, ParseOption::OptionalAddress) )
            {
                target = "8000";
                continue;
            }
        }
    }

    if( PropertyChangedEvent.GetPropertyName() == SendTargetsName )
    {
        for(auto & target : SendTargets)
        {
            FIPv4Address address;
            uint32_t port;
            FIPv4Address multicastAddress;
            if( !Parse(target, &address, &port, &multicastAddress, ParseOption::OptionalPort) || (address == FIPv4Address::Any && !multicastAddress.IsMulticastAddress()) )
            {
                target = "127.0.0.1:8000";
                continue;
            }

            const FIPv4Address& targetAddress = multicastAddress.IsMulticastAddress() ? multicastAddress : address;
            if( port == 0 )
            {
                target = targetAddress.ToString() + ":8000";
            }
        }
    }

    Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif
