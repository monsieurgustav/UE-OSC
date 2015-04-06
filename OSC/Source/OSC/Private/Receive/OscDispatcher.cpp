#include "OscPrivatePCH.h"
#include "OscDispatcher.h"
#include "OscReceiverInterface.h"

#include "oscpack/osc/OscReceivedElements.h"


UOscDispatcher::UOscDispatcher(const class FPostConstructInitializeProperties& PCIP)
: Super(PCIP),
  _listening(FIPv4Address(0), 0),
  _socket(nullptr),
  _socketReceiver(nullptr),
  _pendingMessages(64)
{
}

UOscDispatcher * UOscDispatcher::Get()
{
    return UOscDispatcher::StaticClass()->GetDefaultObject<UOscDispatcher>();
}

void UOscDispatcher::Listen(FIPv4Address address, uint32_t port)
{
    if(_listening != std::make_pair(address, port))
    {
        Stop();

        FUdpSocketBuilder builder(TEXT("OscListener"));
        builder.BoundToPort(port);
        if(address.IsMulticastAddress())
        {
            builder.JoinedToGroup(address);
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

static void SendMessage(TCircularQueue<std::pair<FName, TArray<FOscDataElemStruct>>> & _pendingMessages, const osc::ReceivedMessage & message)
{
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
            elem.SetBool(it->AsBoolUnchecked());
        }
        else if(it->IsString())
        {
            elem.SetString(FName(it->AsStringUnchecked()));
        }
        data.Add(elem);
    }

    // save it in pending messages
    _pendingMessages.Enqueue(std::make_pair(address, data));
}

static void SendBundle(TCircularQueue<std::pair<FName, TArray<FOscDataElemStruct>>> & _pendingMessages, const osc::ReceivedBundle & bundle)
{
    const auto begin = bundle.ElementsBegin();
    const auto end = bundle.ElementsEnd();
    for(auto it = begin; it != end; ++it)
    {
        if(it->IsBundle())
        {
            SendBundle(_pendingMessages, osc::ReceivedBundle(*it));
        }
        else
        {
            SendMessage(_pendingMessages, osc::ReceivedMessage(*it));
        }
    }
}

void UOscDispatcher::Callback(const FArrayReaderPtr& data, const FIPv4Endpoint&)
{
    try
    {
        const osc::ReceivedPacket packet((const char *)data->GetData(), data->Num());
        if(packet.IsBundle())
        {
            SendBundle(_pendingMessages, osc::ReceivedBundle(packet));
        }
        else
        {
            SendMessage(_pendingMessages, osc::ReceivedMessage(packet));
        }
    }
    catch(osc::Exception &e)
    {
        // Exceptions are disabled by default, so destructors are not called.
        // We don't care: there is no acquired resource to release.
        const FString wide(e.what());
        UE_LOG(LogOSC, Warning, TEXT("OSC Message Error: %s"), *wide);
    }
    
    if(!_pendingMessages.IsEmpty())
    {
        FSimpleDelegateGraphTask::CreateAndDispatchWhenReady(
            FSimpleDelegateGraphTask::FDelegate::CreateUObject(this, &UOscDispatcher::CallbackMainThread),
#if OSC_ENGINE_VERSION < 40500
            TEXT("OscDispatcherProcessMessages"),
#else
            TStatId(),
#endif
            nullptr,
            ENamedThreads::GameThread);
    }

    // Log received packet
    if(!LogOSC.IsSuppressed(ELogVerbosity::Verbose))
    {
        const auto encoded = FBase64::Encode(*data);
        UE_LOG(LogOSC, Verbose, TEXT("Received: %s"), *encoded);
    }
}

void UOscDispatcher::CallbackMainThread()
{
    FScopeLock ScopeLock(&_receiversMutex);

    std::pair<FName, TArray<FOscDataElemStruct>> message;
    while(_pendingMessages.Dequeue(message))
    {
        for(auto receiver : _receivers)
        {
            receiver->SendEvent(message.first, message.second);
        }
    }
}

void UOscDispatcher::BeginDestroy()
{
    Stop();
    Super::BeginDestroy();
}
