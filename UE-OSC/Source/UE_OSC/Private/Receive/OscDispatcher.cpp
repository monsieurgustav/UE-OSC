#include "Receive/OscDispatcher.h"
#include "Receive/OscReceiverInterface.h"
#include "Common/OscUtils.h"
#include "Common/oscpack/osc/OscReceivedElements.h"

#include "Serialization/ArrayReader.h"
#include "Common/UdpSocketBuilder.h"
#include "Common/UdpSocketReceiver.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Async/TaskGraphInterfaces.h"
#include "Misc/Base64.h"

#include "oscpkt.hh"  // only for pattern matching (oscpack is preferred because it makes no memory allocation)


UOscDispatcher::UOscDispatcher()
: _taskSpawned(0)
{
}

UOscDispatcher::UOscDispatcher(FVTableHelper & helper)
{
    // Does not need to be a valid object.
}

UOscDispatcher * UOscDispatcher::Get()
{
    return UOscDispatcher::StaticClass()->GetDefaultObject<UOscDispatcher>();
}

void UOscDispatcher::Listen(FIPv4Address address, uint32_t port, FIPv4Address multicastAddress, bool multicastLoopback)
{
    FUdpSocketBuilder builder(TEXT("OscListener"));
    builder.BoundToPort(port);
    if(multicastAddress.IsMulticastAddress())
    {
        builder.JoinedToGroup(multicastAddress, address);
        if(multicastLoopback)
        {
            builder.WithMulticastLoopback();
        }
    }
    else
    {
        builder.BoundToAddress(address);
    }
        
    auto socket = builder.Build();
    if(socket)
    {
        auto receiver = new FUdpSocketReceiver(socket, FTimespan::FromMilliseconds(100), TEXT("OSCListener"));
        receiver->OnDataReceived().BindUObject(this, &UOscDispatcher::Callback);
        receiver->Start();
        _socketReceivers.Add({ socket, receiver });

        UE_LOG(LogUE_OSC, Display, TEXT("Listen to port %d"), port);
    }
    else
    {
        UE_LOG(LogUE_OSC, Warning, TEXT("Cannot listen port %d"), port);
    }
}

void UOscDispatcher::Stop()
{
    UE_LOG(LogUE_OSC, Display, TEXT("Stop listening"));

    for (auto& socketReceiver : _socketReceivers)
    {
        delete socketReceiver.Receiver;
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(socketReceiver.Socket);
    }
    _socketReceivers.Reset();
}

void UOscDispatcher::RegisterReceiver(IOscReceiverInterface * receiver)
{
    FScopeLock ScopeLock(&_receiversMutex);
    if (_sendingMessages)
    {
        _deferredReceivers.Add({ receiver, true });
    }
    else
    {
        _receivers.AddUnique(receiver);
    }
}

void UOscDispatcher::UnregisterReceiver(IOscReceiverInterface * receiver)
{
    FScopeLock ScopeLock(&_receiversMutex);
    if (_sendingMessages)  // defer remove
    {
        _deferredReceivers.Add({ receiver, false });
    }
    else
    {
        _receivers.Remove(receiver);
    }
}

static void SendMessage(TQueue<std::tuple<FName, TArray<FOscDataElemStruct>, FIPv4Address>, EQueueMode::Mpsc> & _pendingMessages,
                        const osc::ReceivedMessage & message,
                        const FIPv4Address & senderIp)
{
    if(message.State() != osc::SUCCESS)
    {
        UE_LOG(LogUE_OSC, Warning, TEXT("OSC Received Message Error: %s"), osc::errorString(message.State()));
        return;
    }
    const FName address(message.AddressPattern());
    if(address.GetDisplayNameEntry()->IsWide())
    {
        UE_LOG(LogUE_OSC, Warning, TEXT("OSC Received Message Error: address contains non ASCII characters"));
        return;
    }

    TArray<FOscDataElemStruct> data;
    
    const auto argBegin = message.ArgumentsBegin();
    const auto argEnd = message.ArgumentsEnd();
    for(auto it = argBegin; it != argEnd; ++it)
    {
        FOscDataElemStruct elem;
        if(it->IsFloat())
        {
            elem.SetFloat(it->AsFloatUnchecked());
        }
        else if(it->IsDouble())
        {
            elem.SetFloat(it->AsDoubleUnchecked());
        }
        else if(it->IsInt32())
        {
            elem.SetInt(it->AsInt32Unchecked());
        }
        else if(it->IsInt64())
        {
            elem.SetInt(it->AsInt64Unchecked());
        }
        else if(it->IsBool())
        {
            osc::Errors error = osc::SUCCESS;
            elem.SetBool(it->AsBoolUnchecked(error));
            check(error == osc::SUCCESS);
        }
        else if(it->IsString())
        {
            elem.SetString(FName(reinterpret_cast<const UTF8CHAR*>(it->AsStringUnchecked())));
        }
        else if(it->IsBlob())
        {
            const void* buffer;
            osc::osc_bundle_element_size_t size;
            osc::Errors error = osc::SUCCESS;
            it->AsBlobUnchecked(buffer, size, error);

            TArray<uint8> blob;
            if(size && !error)
            {
                blob.SetNumUninitialized(size);
                FMemory::Memcpy(blob.GetData(), buffer, size);
            }
            else if(error)
            {
                UE_LOG(LogUE_OSC, Warning, TEXT("OSC Received Message Error: %s"), osc::errorString(error));
            }
            elem.SetBlob(std::move(blob));
        }
        else if(it->IsArrayBegin())
        {
            elem.SetArrayBegin();
        }
        else if (it->IsArrayEnd())
        {
            elem.SetArrayEnd();
        }
        data.Add(elem);
    }

    // save it in pending messages
    const auto added = _pendingMessages.Enqueue(std::make_tuple(address, data, senderIp));

    // the circular buffer may be full.
    if(!added)
    {
        UE_LOG(LogUE_OSC, Warning, TEXT("Circular Buffer Full: Message Ignored"));
    }
}

static void SendBundle(TQueue<std::tuple<FName, TArray<FOscDataElemStruct>, FIPv4Address>, EQueueMode::Mpsc> & _pendingMessages,
                       const osc::ReceivedBundle & bundle,
                       const FIPv4Address & senderIp)
{
    if(bundle.State() != osc::SUCCESS)
    {
        UE_LOG(LogUE_OSC, Warning, TEXT("OSC Received Bundle Error: %s"), osc::errorString(bundle.State()));
        return;
    }

    const auto begin = bundle.ElementsBegin();
    const auto end = bundle.ElementsEnd();
    for(auto it = begin; it != end; ++it)
    {
        if(it->IsBundle())
        {
            SendBundle(_pendingMessages, osc::ReceivedBundle(*it), senderIp);
        }
        else
        {
            SendMessage(_pendingMessages, osc::ReceivedMessage(*it), senderIp);
        }
    }
}

void UOscDispatcher::Callback(const FArrayReaderPtr& data, const FIPv4Endpoint& endpoint)
{
    DispatchPacket(MakeArrayView(data->GetData(), data->Num()), endpoint);
}

void UOscDispatcher::DispatchPacket(const TArrayView<const uint8>& data, const FIPv4Endpoint& endpoint)
{
    const osc::ReceivedPacket packet((const char *)data.GetData(), data.Num());
    if(packet.State() != osc::SUCCESS)
    {
        UE_LOG(LogUE_OSC, Warning, TEXT("OSC Received Packet Error: %s"), osc::errorString(packet.State()));
        return;
    }

    if(packet.IsBundle())
    {
        SendBundle(_pendingMessages, osc::ReceivedBundle(packet), endpoint.Address);
    }
    else
    {
        SendMessage(_pendingMessages, osc::ReceivedMessage(packet), endpoint.Address);
    }

    // Set a single callback in the main thread per frame.
    if(!_pendingMessages.IsEmpty() && !FPlatformAtomics::InterlockedCompareExchange(&_taskSpawned, 1, 0))
    {
        check(_taskSpawned == 1);
        FSimpleDelegateGraphTask::CreateAndDispatchWhenReady(
            FSimpleDelegateGraphTask::FDelegate::CreateUObject(this, &UOscDispatcher::CallbackMainThread),
            TStatId(),
            nullptr,
            ENamedThreads::GameThread);
    }

#if !NO_LOGGING
    // Log received packet
    if(!LogUE_OSC.IsSuppressed(ELogVerbosity::Verbose))
    {
        const auto encoded = FBase64::Encode(data.GetData(), data.Num());
        UE_LOG(LogUE_OSC, Verbose, TEXT("Received: %s"), *encoded);
    }
#endif
}

void UOscDispatcher::CallbackMainThread()
{
    // Release before dequeue.
    // If it was released after dequeue, when a message arrives after the while
    // loop and before the release, it would not be processed.
    check(_taskSpawned == 1);
    FPlatformAtomics::InterlockedCompareExchange(&_taskSpawned, 0, 1);

    FScopeLock ScopeLock(&_receiversMutex);
    _sendingMessages = true;

    FString addressStr;
    ANSICHAR addressPatternANSI[NAME_SIZE];
    ANSICHAR addressFilterANSI[NAME_SIZE];
    std::tuple<FName, TArray<FOscDataElemStruct>, FIPv4Address> message;
    while(_pendingMessages.Dequeue(message))
    {
        const FIPv4Address & senderIp = std::get<2>(message);
        const FString senderIpStr = FString::Printf(TEXT("%i.%i.%i.%i"), senderIp.A, senderIp.B, senderIp.C, senderIp.D);

        FName address = std::get<0>(message);
        address.GetPlainANSIString(addressPatternANSI);  // it is check as ANSI string when the messae is received
        Osc::NameToLower(addressPatternANSI);

        for(auto receiver : _receivers)
        {
            const FName addressFilter = receiver->GetAddressFilter();
            if (!addressFilter.IsNone())
            {
                if (addressFilter.GetDisplayNameEntry()->IsWide())
                {
                    // only ANSI characters are supported as address pattern, it would necessarly fail.
                    continue;
                }
                else
                {
                    addressFilter.GetPlainANSIString(addressFilterANSI);
                    Osc::NameToLower(addressFilterANSI);
                    const ANSICHAR* result = oscpkt::internalPatternMatch(addressPatternANSI, addressFilterANSI);
                    if (!result)
                    {
                        continue;
                    }

                    address = FName{ result };
                }
            }
            receiver->SendEvent(address, std::get<1>(message), senderIpStr);
        }
    }

    // merge registered receivers
    for (const auto& deferredReceiver : _deferredReceivers)
    {
        if (deferredReceiver.Value)
        {
            _receivers.AddUnique(deferredReceiver.Key);
        }
        else
        {
            _receivers.Remove(deferredReceiver.Key);
        }
    }
    _deferredReceivers.Empty();
    _sendingMessages = false;
}

void UOscDispatcher::BeginDestroy()
{
    Stop();
    Super::BeginDestroy();
}
