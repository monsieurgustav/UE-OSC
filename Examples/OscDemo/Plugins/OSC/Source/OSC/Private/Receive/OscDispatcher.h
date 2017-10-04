#pragma once

#include "OscDataElemStruct.h"
#include "OscDispatcher.generated.h"

struct IOscReceiverInterface;
class FSocket;
class FUdpSocketReceiver;


/// Singleton that dispatches the OSC messages to listeners.
UCLASS()
class UOscDispatcher : public UObject
{
    GENERATED_BODY()

    friend class FOscModule;

    void Listen(FIPv4Address address, uint32_t port, bool multicastLoopback);
    void Stop();

public:
    /// Default constructor
    UOscDispatcher();

    /// Hot reload constructor
    UOscDispatcher(FVTableHelper & helper);

    /// Get the unique instance
    ///
    /// The instance is the UObject default instance.
    static UOscDispatcher * Get();

    /// Add the receiver in the listeners list
    void RegisterReceiver(IOscReceiverInterface * receiver);

    /// Remove the receiver in the listeners list
    void UnregisterReceiver(IOscReceiverInterface * receiver);

private:
    void Callback(const FArrayReaderPtr& data, const FIPv4Endpoint&);

    void CallbackMainThread();

    void BeginDestroy() override;
    
private:
    TArray<IOscReceiverInterface *> _receivers;
    std::pair<FIPv4Address, uint32_t> _listening;
    FSocket * _socket;
    FUdpSocketReceiver * _socketReceiver;
    TCircularQueue<std::tuple<FName, TArray<FOscDataElemStruct>, FIPv4Address>> _pendingMessages;
    int32 _taskSpawned;

    /// Protects _receivers
    FCriticalSection _receiversMutex;
};
