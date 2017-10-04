#pragma once

#include "OscDataElemStruct.h"
#include "OscMessageStruct.h"
#include "OscFunctionLibrary.generated.h"


UCLASS()
class UOscFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    /// Get the next value from an OSC message as a boolean.
    UFUNCTION(BlueprintPure, Category=OSC)
    static void PopBool(const TArray<FOscDataElemStruct> & input, TArray<FOscDataElemStruct> & output, bool & Value);

    /// Get the next value from an OSC message as a floating point.
    UFUNCTION(BlueprintPure, Category=OSC)
    static void PopFloat(const TArray<FOscDataElemStruct> & input, TArray<FOscDataElemStruct> & output, float & Value);

    /// Get the next value from an OSC message as a integer.
    UFUNCTION(BlueprintPure, Category=OSC)
    static void PopInt(const TArray<FOscDataElemStruct> & input, TArray<FOscDataElemStruct> & output, int32 & Value);

    /// Get the next value from an OSC message as a string.
    UFUNCTION(BlueprintPure, Category=OSC)
    static void PopString(const TArray<FOscDataElemStruct> & input, TArray<FOscDataElemStruct> & output, FName & Value);

    /// Get the next value from an OSC message as a blob.
    UFUNCTION(BlueprintPure, Category=OSC)
    static void PopBlob(const TArray<FOscDataElemStruct> & input, TArray<FOscDataElemStruct> & output, TArray<uint8> & Value);


    /// Add a boolean value to an OSC message.
    UFUNCTION(BlueprintPure, Category=OSC, meta=(AutoCreateRefTerm = "input"))
    static void PushBool(const TArray<FOscDataElemStruct> & input, bool Value, TArray<FOscDataElemStruct> & output);

    /// Add a floating point value to an OSC message.
    UFUNCTION(BlueprintPure, Category=OSC, meta=(AutoCreateRefTerm = "input"))
    static void PushFloat(const TArray<FOscDataElemStruct> & input, float Value, TArray<FOscDataElemStruct> & output);

    /// Add a integer value to an OSC message.
    UFUNCTION(BlueprintPure, Category=OSC, meta=(AutoCreateRefTerm = "input"))
    static void PushInt(const TArray<FOscDataElemStruct> & input, int32 Value, TArray<FOscDataElemStruct> & output);

    /// Add a string value to an OSC message.
    UFUNCTION(BlueprintPure, Category=OSC, meta=(AutoCreateRefTerm = "input"))
    static void PushString(const TArray<FOscDataElemStruct> & input, FName Value, TArray<FOscDataElemStruct> & output);

    /// Add a blob to an OSC message.
    UFUNCTION(BlueprintPure, Category=OSC, meta=(AutoCreateRefTerm = "input"))
    static void PushBlob(const TArray<FOscDataElemStruct> & input, const TArray<uint8> & Value, TArray<FOscDataElemStruct> & output);


    /// Interpret an OSC argument as a boolean
    UFUNCTION(BlueprintPure, Category=OSC)
    static bool AsBool(const FOscDataElemStruct & input);

    /// Interpret an OSC argument as a floating point.
    UFUNCTION(BlueprintPure, Category=OSC)
    static float AsFloat(const FOscDataElemStruct & input);

    /// Interpret an OSC argument as a integer.
    UFUNCTION(BlueprintPure, Category=OSC)
    static int32 AsInt(const FOscDataElemStruct & input);

    /// Interpret an OSC argument as a string.
    UFUNCTION(BlueprintPure, Category=OSC)
    static FName AsString(const FOscDataElemStruct & input);

    /// Interpret an OSC argument as a blob.
    UFUNCTION(BlueprintPure, Category=OSC)
    static TArray<uint8> AsBlob(const FOscDataElemStruct & input);


    /// Create an OSC argument from a boolean
    UFUNCTION(BlueprintPure, Category=OSC)
    static FOscDataElemStruct FromBool(bool input);

    /// Create an OSC argument from a floating point.
    UFUNCTION(BlueprintPure, Category=OSC)
    static FOscDataElemStruct FromFloat(float input);

    /// Create an OSC argument from a integer.
    UFUNCTION(BlueprintPure, Category=OSC)
    static FOscDataElemStruct FromInt(int32 input);

    /// Create an OSC argument from a string.
    UFUNCTION(BlueprintPure, Category=OSC)
    static FOscDataElemStruct FromString(FName input);

    /// Create an OSC argument from a blob.
    UFUNCTION(BlueprintPure, Category=OSC)
    static FOscDataElemStruct FromBlob(const TArray<uint8> & input);


    /**
     *  @brief Send an OSC message.
     *  @param Address OSC address.
     *  @param Data result of successive PushFloat/Int/String/etc.
     *  @param TargetIndex index of the destination, -1 for all destinations. (SendTarget list of the plugin settings)
     */
    UFUNCTION(BlueprintCallable, Category=OSC, meta=(AutoCreateRefTerm = "Data"))
    static void SendOsc(FName Address, const TArray<FOscDataElemStruct> & Data, int32 TargetIndex);

    /**
    *  @brief Send several OSC messages in an OSC bundle.
    *  @param Messages of the bundle.
    *  @param TargetIndex index of the destination, -1 for all destinations. (SendTarget list of the plugin settings)
    */
    UFUNCTION(BlueprintCallable, Category=OSC)
    static void SendOscBundle(const TArray<FOscMessageStruct> & Messages, int32 TargetIndex);

    /**
     *  @brief Add Ip:Port to the available OSC send targets.
     *  @param IpPort "ip:port". e.g. "192.168.0.1:7777"
     *  @return The created TargetIndex to pass to the SendOsc function.
     *
     *  Use this function to add target at runtime. Generally, it is best
     *  to define your targets in the OSC plugin settings.
     *
     *  @see SendOsc
     */
    UFUNCTION(BlueprintCallable, Category=OSC)
    static int32 AddSendOscTarget(FString IpPort);
};
