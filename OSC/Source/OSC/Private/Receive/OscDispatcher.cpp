#include "OscPrivatePCH.h"
#include "OscDispatcher.h"
#include "OscReceiverInterface.h"

#include "oscpack/osc/OscReceivedElements.h"


UOscDispatcher::UOscDispatcher()
: _listening(FIPv4Address(0), 0),
  _socket(nullptr),
  _socketReceiver(nullptr),
  _pendingMessages(1024),  // arbitrary max message count per frame
  _taskSpawned(0)
{
}

UOscDispatcher::UOscDispatcher(FVTableHelper & helper)
: _pendingMessages(0)
{
    // Does not need to be a valid object.
}

UOscDispatcher * UOscDispatcher::Get()
{
    return UOscDispatcher::StaticClass()->GetDefaultObject<UOscDispatcher>();
}

void UOscDispatcher::Listen(FIPv4Address address, uint32_t port, bool multicastLoopback)
{
    if(_listening != std::make_pair(address, port))
    {
        Stop();

        FUdpSocketBuilder builder(TEXT("OscListener"));
        builder.BoundToPort(port);
        if(address.IsMulticastAddress())
        {
            builder.JoinedToGroup(address);
            if(multicastLoopback)
            {
                builder.WithMulticastLoopback();
            }
        }
        else
        {
            builder.BoundToAddress(address);
        }
        
        _socket = builder.Build();
        if(_socket)
        {
            _socketReceiver = new FUdpSocketReceiver(_socket, FTimespan::FromMilliseconds(100), TEXT("OSCListener"));
            _socketReceiver->OnDataReceived().BindUObject(this, &UOscDispatcher::Callback);
            _socketReceiver->Start();

            _listening = std::make_pair(address, port);
            UE_LOG(LogOSC, Display, TEXT("Listen to port %d"), port);
        }
        else
        {
            UE_LOG(LogOSC, Warning, TEXT("Cannot listen port %d"), port);
        }
    }
}

void UOscDispatcher::Stop()
{
    UE_LOG(LogOSC, Display, TEXT("Stop listening"));

    delete _socketReceiver;
    _socketReceiver = nullptr;

    if(_socket)
    {
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(_socket);
        _socket = nullptr;
    }

    _listening = std::make_pair(FIPv4Address(0), 0);
}

void UOscDispatcher::RegisterReceiver(IOscReceiverInterface * receiver)
{
    FScopeLock ScopeLock(&_receiversMutex);
    _receivers.AddUnique(receiver);
}

void UOscDispatcher::UnregisterReceiver(IOscReceiverInterface * receiver)
{
    FScopeLock ScopeLock(&_receiversMutex);
    _receivers.Remove(receiver);
}

static void SendMessage(TCircularQueue<std::tuple<FName, TArray<FOscDataElemStruct>, FIPv4Address>> & _pendingMessages,
                        const osc::ReceivedMessage & message,
                        const FIPv4Address & senderIp)
{
    if(message.State() != osc::SUCCESS)
    {
        UE_LOG(LogOSC, Warning, TEXT("OSC Received Message Error: %s"), osc::errorString(message.State()));
        return;
    }
    const FName address(message.AddressPattern());

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
            elem.SetString(FName(it->AsStringUnchecked()));
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
                UE_LOG(LogOSC, Warning, TEXT("OSC Received Message Error: %s"), osc::errorString(error));
            }
            elem.SetBlob(std::move(blob));
        }
        data.Add(elem);
    }

    // save it in pending messages
    const auto added = _pendingMessages.Enqueue(std::make_tuple(address, data, senderIp));

    // the circular buffer may be full.
    if(!added)
    {
        UE_LOG(LogOSC, Warning, TEXT("Circular Buffer Full: Message Ignored"));
    }
}

static void SendBundle(TCircularQueue<std::tuple<FName, TArray<FOscDataElemStruct>, FIPv4Address>> & _pendingMessages,
                       const osc::ReceivedBundle & bundle,
                       const FIPv4Address & senderIp)
{
    if(bundle.State() != osc::SUCCESS)
    {
        UE_LOG(LogOSC, Warning, TEXT("OSC Received Bundle Error: %s"), osc::errorString(bundle.State()));
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
    const osc::ReceivedPacket packet((const char *)data->GetData(), data->Num());
    if(packet.State() != osc::SUCCESS)
    {
        UE_LOG(LogOSC, Warning, TEXT("OSC Received Packet Error: %s"), osc::errorString(packet.State()));
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
    if(!LogOSC.IsSuppressed(ELogVerbosity::Verbose))
    {
        const auto encoded = FBase64::Encode(*data);
        UE_LOG(LogOSC, Verbose, TEXT("Received: %s"), *encoded);
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

    std::tuple<FName, TArray<FOscDataElemStruct>, FIPv4Address> message;
    while(_pendingMessages.Dequeue(message))
    {
        const FIPv4Address & senderIp = std::get<2>(message);
        FString senderIpStr = FString::Printf(TEXT("%i.%i.%i.%i"), senderIp.A, senderIp.B, senderIp.C, senderIp.D);
        for(auto receiver : _receivers)
        {
            receiver->SendEvent(std::get<0>(message), std::get<1>(message), senderIpStr);
        }
    }
}

void UOscDispatcher::BeginDestroy()
{
    Stop();
    Super::BeginDestroy();
}
