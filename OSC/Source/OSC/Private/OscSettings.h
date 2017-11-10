#pragma once

#include <utility>
#include "OscReceiverInputKey.h"

#include "OscSettings.generated.h"

class UOscDispatcher;


UCLASS(Config=Engine, DefaultConfig)
class UOscSettings : public UObject
{
    GENERATED_BODY()

public:

    /// Default constructor
    UOscSettings();

    /// Hot reload constructor
    UOscSettings(FVTableHelper & helper);

    /**
     *  Specify the [address:]port to listen to.
     *
     *  e.g.
     *  - "8000" listen to messages from any sender on port 8000.  [default]
     *  - "224.0.0.100:8000" listen multi-cast messages of group 224.0.0.100 on port 8000.
     *  - "192.168.0.1:8000" listen messages addressed specifically to 192.168.0.1 on port 8000, useful if there are several addresses for this machine.
     */
    UPROPERTY(Config, EditAnywhere, Category=Receive)
    FString ReceiveFrom;

    /**
     *  Specify the addresses (ip:port) to send messages to.
     *
     *  Addresses can also be added at runtime with the AddSendOscTarget function.
     */
    UPROPERTY(Config, EditAnywhere, Category=Send)
    TArray<FString> SendTargets;

    /**
     *  List of the messages treated as inputs.
     *
     *  Every entry adds a key in the input mapping project setting.
     *  e.g. "/position/x" -> "OSC_position_x"
     */
    UPROPERTY(Config, EditAnywhere, Category=Input)
    TArray<FString> Inputs;

    UPROPERTY(Config, EditAnywhere, Category=Network)
    bool MulticastLoopback;

public:
    void InitSendTargets();

    int32 GetOrAddSendTarget(const FString & ip_port);

    void Send(const uint8 *buffer, int32 length, int32 targetIndex);

    void ClearKeyInputs(UOscDispatcher & dispatcher);

    void UpdateKeyInputs(UOscDispatcher & dispatcher);
#if WITH_EDITOR
    void PostEditChangeProperty( struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
public:

    enum class ParseOption
    {
        Strict,  /// "192.168.0.12:8000"
        OptionalPort,  ///  "192.168.0.12" or "192.168.0.12:8000"
        OptionalAddress,  ///  "8000" or "192.168.0.12:8000"
    };

    /**
     *  @brief Parse "8000" and "192.168.0.12" to IP and port values.
     *  @return true if succeed
     */
    static bool Parse(const FString & ip_port, FIPv4Address * address, uint32_t * port, ParseOption option);

private:
    int32 AddSendTarget(const FString & ip_port);

private:
    TSharedRef<FSocket> _sendSocket;
    TArray<TSharedRef<FInternetAddr>> _sendAddresses;
    TMap<FString, int32> _sendAddressesIndex;
    TArray<std::unique_ptr<OscReceiverInputKey>> _keyReceivers;
};
