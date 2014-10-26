// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

#include <utility>
#include "OscSettings.generated.h"


UCLASS(config=Engine)
class UOscSettings : public UObject
{
    GENERATED_UCLASS_BODY()

public:

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

    UPROPERTY(Config, EditAnywhere, Category=Send)
    TArray<FString> SendTargets;

public:
    void UpdateSendAddresses();

    void Send(const uint8 *buffer, int32 length, int32 targetIndex);

public:

    /**
     *  @brief Parse "8000" and "192.168.0.12" to IP and port values.
     *  @return true if succeed
     */
    static bool Parse(const FString & address_port, FIPv4Address * address, uint32_t * port);

private:
    TSharedRef<FSocket> _sendSocket;
    TArray<std::pair<FString, TSharedRef<FInternetAddr>>> _sendAddresses;
};
