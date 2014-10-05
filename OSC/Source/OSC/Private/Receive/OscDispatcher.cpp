

#include "OscPrivatePCH.h"
#include "OscDispatcher.h"
#include "OscReceiverInterface.h"

#include "oscpack/osc/OscReceivedElements.h"

#include <assert.h>


UOscDispatcher::UOscDispatcher(const class FPostConstructInitializeProperties& PCIP)
: Super(PCIP),
  _currentPort(0),
  _socket(nullptr),
  _socketReceiver(nullptr),
  _pendingMessages(64)
{
}

UOscDispatcher * UOscDispatcher::Get()
{
    return UOscDispatcher::StaticClass()->GetDefaultObject<UOscDispatcher>();
}

void UOscDispatcher::Listen(uint32_t port)
{
    if(_currentPort != port)
    {
        delete _socket;
        delete _socketReceiver;

        _socket = FUdpSocketBuilder(TEXT("OscListener")).BoundToAddress(FIPv4Address::Any).BoundToPort(port).Build();
        if(_socket)
        {
            _socketReceiver = new FUdpSocketReceiver(_socket, FTimespan(0, 0, 0, 10), TEXT("OSCListener"));
            _socketReceiver->OnDataReceived().BindUObject(this, &UOscDispatcher::Callback);

            _currentPort = port;
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
    if(_socket)
    {
        _socket->Close();
    }
    if(_socketReceiver)
    {
        _socketReceiver->OnDataReceived().Unbind();
        _socketReceiver->Stop();

        // wait longer than the socket read wait time, or it will crash.
        FPlatformProcess::Sleep(0.1f);
    }

    delete _socket;
    delete _socketReceiver;
    _socket = nullptr;
    _socketReceiver = nullptr;

    _currentPort = 0;
}

void UOscDispatcher::RegisterReceiver(IOscReceiverInterface * receiver)
{
    _receivers.AddUnique(receiver);
}

void UOscDispatcher::UnregisterReceiver(IOscReceiverInterface * receiver)
{
    _receivers.Remove(receiver);
}

static void SendMessage(TCircularQueue<std::pair<FName, FOscDataStruct>> & _pendingMessages, const osc::ReceivedMessage & message)
{
    const FName address(message.AddressPattern());

    FOscDataStruct data;
    
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
        data.Queue.Add(elem);
    }

    // save it in pending messages
    _pendingMessages.Enqueue(std::make_pair(address, data));
}

static void SendBundle(TCircularQueue<std::pair<FName, FOscDataStruct>> & _pendingMessages, const osc::ReceivedBundle & bundle)
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
    UE_LOG(LogOSC, Verbose, TEXT("OSC Received"));
    const osc::ReceivedPacket packet((const char *)data->GetData(), data->Num());
    if(packet.IsBundle())
    {
        SendBundle(_pendingMessages, osc::ReceivedBundle(packet));
    }
    else
    {
        SendMessage(_pendingMessages, osc::ReceivedMessage(packet));
    }
    
    if(!_pendingMessages.IsEmpty())
    {
        FSimpleDelegateGraphTask::CreateAndDispatchWhenReady(
            FSimpleDelegateGraphTask::FDelegate::CreateUObject(this, &UOscDispatcher::CallbackMainThread),
            TEXT("OscDispatcherProcessMessages"),
            nullptr,
            ENamedThreads::GameThread);
    }
}

void UOscDispatcher::CallbackMainThread()
{
    std::pair<FName, FOscDataStruct> message;
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
