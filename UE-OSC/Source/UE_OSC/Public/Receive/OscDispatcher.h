#pragma once

#include "Osc.h"
#include "Common/OscDataElemStruct.h"
#include "Common/UdpSocketReceiver.h"

#include "UObject/Object.h"
#include "Containers/Queue.h"
#include "Interfaces/IPv4/IPv4Address.h"

#include <utility>
#include <tuple>

#include "OscDispatcher.generated.h"

struct IOscReceiverInterface;
class FSocket;
class FUdpSocketReceiver;
struct FIPv4Endpoint;

/// Singleton that dispatches the OSC messages to listeners.
UCLASS()
class UE_OSC_API UOscDispatcher : public UObject
{
    GENERATED_BODY()

    friend class FOscModule;

    void Listen(FIPv4Address address, uint32_t port, FIPv4Address multicastAddress, bool multicastLoopback);
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

    /// Dispatch an OSC message from an external source.
    /// 
    /// The dispatcher generally uses its own UDP socket to receive OSC packets.
    /// This function is useful to also receive OSC packets from another source,
    /// like an TCP socket, etc.
    void DispatchPacket(const TArrayView<const uint8>& data, const FIPv4Endpoint&);

private:
    void Callback(const FArrayReaderPtr& data, const FIPv4Endpoint&);

    void CallbackMainThread();

    void BeginDestroy() override;
    
private:
    TArray<IOscReceiverInterface *> _receivers;
    struct SocketReceiver
    {
        FSocket * Socket;
        FUdpSocketReceiver * Receiver;
    };
    TArray<SocketReceiver> _socketReceivers;
    TQueue<std::tuple<FName, TArray<FOscDataElemStruct>, FIPv4Address>, EQueueMode::Mpsc> _pendingMessages;  // supports multiple-producers
    int32 _taskSpawned;

    /// Protects _receivers
    FCriticalSection _receiversMutex;
    
    /// Prevent new receivers to be added/removed while iterating over existing receivers
    bool _sendingMessages = false;
    TArray<TPair<IOscReceiverInterface*, bool>> _deferredReceivers;
};
