// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

#include <utility>
#include "OscSettings.generated.h"


UCLASS(config=Engine)
class UOscSettings : public UObject
{
    GENERATED_UCLASS_BODY()

public:

    UPROPERTY(Config, EditAnywhere, Category=Receive)
    int32 ReceivePort;

    UPROPERTY(Config, EditAnywhere, Category=Send)
    TArray<FString> SendTargets;

public:
    void UpdateSendAddresses();

    void Send(const uint8 *buffer, int32 length, int32 targetIndex);

private:
    TSharedRef<FSocket> _sendSocket;
    TArray<std::pair<FString, TSharedRef<FInternetAddr>>> _sendAddresses;
};
