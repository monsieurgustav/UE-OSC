#pragma once

#include "OscDataStruct.h"
#include "OscFunctionLibrary.generated.h"


UCLASS()
class UOscFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_UCLASS_BODY()

    /// Get the next value from an OSC message as a boolean.
    UFUNCTION(BlueprintPure, Category=OSC)
    static void PopBool(const FOscDataStruct & input, FOscDataStruct & output, bool & Value);

    /// Get the next value from an OSC message as a floating point.
    UFUNCTION(BlueprintPure, Category=OSC)
    static void PopFloat(const FOscDataStruct & input, FOscDataStruct & output, float & Value);

    /// Get the next value from an OSC message as a integer.
    UFUNCTION(BlueprintPure, Category=OSC)
    static void PopInt(const FOscDataStruct & input, FOscDataStruct & output, int32 & Value);

    /// Get the next value from an OSC message as a string.
    UFUNCTION(BlueprintPure, Category=OSC)
    static void PopString(const FOscDataStruct & input, FOscDataStruct & output, FName & Value);


    /// Add a boolean value to an OSC message.
    UFUNCTION(BlueprintPure, Category=OSC)
    static void PushBool(FOscDataStruct input, bool Value, FOscDataStruct & output);

    /// Add a floating point value to an OSC message.
    UFUNCTION(BlueprintPure, Category=OSC)
    static void PushFloat(FOscDataStruct input, float Value, FOscDataStruct & output);

    /// Add a integer value to an OSC message.
    UFUNCTION(BlueprintPure, Category=OSC)
    static void PushInt(FOscDataStruct input, int32 Value, FOscDataStruct & output);

    /// Add a string value to an OSC message.
    UFUNCTION(BlueprintPure, Category=OSC)
    static void PushString(FOscDataStruct input, FName Value, FOscDataStruct & output);


    /**
     *  @brief Send an OSC message.
     *  @param Address OSC address.
     *  @param Data result of successive PushFloat/Int/String/etc.
     *  @param Index index of the destination, -1 for all destinations. (SendTarget list of the plugin settings)
     */
    UFUNCTION(BlueprintCallable, Category=OSC)
    static void SendOsc(FName Address, const FOscDataStruct & Data, int32 TargetIndex);
};
