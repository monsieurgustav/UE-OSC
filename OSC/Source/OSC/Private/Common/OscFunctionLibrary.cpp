#include "OscPrivatePCH.h"
#include "OscSettings.h"
#include "OscFunctionLibrary.h"
#include "oscpack/osc/OscOutboundPacketStream.h"


UOscFunctionLibrary::UOscFunctionLibrary(const class FPostConstructInitializeProperties& PCIP)
  : Super(PCIP)
{
}

template <class T>
static inline
void PopValueImpl(const TArray<FOscDataElemStruct> & input, TArray<FOscDataElemStruct> & output, T & Value)
{
    if(input.Num() > 0)
    {
        output.Reserve(input.Num() - 1);
        for(int32 i=1, n=input.Num(); i!=n; ++i)
        {
            output.Add(input[i]);
        }
        Value = input[0].GetValue<T>();
    }
    else
    {
        output.Empty();
        Value = FOscDataElemStruct().GetValue<T>();
    }
}

void UOscFunctionLibrary::PopBool(const TArray<FOscDataElemStruct> & input, TArray<FOscDataElemStruct> & output, bool & Value)
{
    PopValueImpl(input, output, Value);
}

void UOscFunctionLibrary::PopFloat(const TArray<FOscDataElemStruct> & input, TArray<FOscDataElemStruct> & output, float & Value)
{
    PopValueImpl(input, output, Value);
}

void UOscFunctionLibrary::PopInt(const TArray<FOscDataElemStruct> & input, TArray<FOscDataElemStruct> & output, int32 & Value)
{
    PopValueImpl(input, output, Value);
}

void UOscFunctionLibrary::PopString(const TArray<FOscDataElemStruct> & input, TArray<FOscDataElemStruct> & output, FName & Value)
{
    PopValueImpl(input, output, Value);
}


void UOscFunctionLibrary::PushBool(const TArray<FOscDataElemStruct> & input, bool Value, TArray<FOscDataElemStruct> & output)
{
    output = input;
    FOscDataElemStruct elem;
    elem.SetBool(Value);
    output.Add(elem);
}

void UOscFunctionLibrary::PushFloat(const TArray<FOscDataElemStruct> & input, float Value, TArray<FOscDataElemStruct> & output)
{
    output = input;
    FOscDataElemStruct elem;
    elem.SetFloat(Value);
    output.Add(elem);
}

void UOscFunctionLibrary::PushInt(const TArray<FOscDataElemStruct> & input, int32 Value, TArray<FOscDataElemStruct> & output)
{
    output = input;
    FOscDataElemStruct elem;
    elem.SetInt(Value);
    output.Add(elem);
}

void UOscFunctionLibrary::PushString(const TArray<FOscDataElemStruct> & input, FName Value, TArray<FOscDataElemStruct> & output)
{
    if(Value.GetDisplayNameEntry()->IsWide())
    {
        const auto tmp = Value.GetPlainNameString();
        UE_LOG(LogOSC, Error, TEXT("Invalid string argument \"%s\": ASCII only"), *tmp);
        return;
    }

    output = input;
    FOscDataElemStruct elem;
    elem.SetString(Value);
    output.Add(elem);
}


bool UOscFunctionLibrary::AsBool(const FOscDataElemStruct & input)
{
    return input.GetValue<bool>();
}

float UOscFunctionLibrary::AsFloat(const FOscDataElemStruct & input)
{
    return input.GetValue<float>();
}

int32 UOscFunctionLibrary::AsInt(const FOscDataElemStruct & input)
{
    return input.GetValue<int32>();
}

FName UOscFunctionLibrary::AsString(const FOscDataElemStruct & input)
{
    return input.GetValue<FName>();
}


FOscDataElemStruct UOscFunctionLibrary::FromBool(bool input)
{
    FOscDataElemStruct result;
    result.SetBool(input);
    return result;
}

FOscDataElemStruct UOscFunctionLibrary::FromFloat(float input)
{
    FOscDataElemStruct result;
    result.SetFloat(input);
    return result;
}

FOscDataElemStruct UOscFunctionLibrary::FromInt(int32 input)
{
    FOscDataElemStruct result;
    result.SetInt(input);
    return result;
}

FOscDataElemStruct UOscFunctionLibrary::FromString(FName input)
{
    FOscDataElemStruct result;
    result.SetString(input);
    return result;
}


void UOscFunctionLibrary::SendOsc(FName Address, const TArray<FOscDataElemStruct> & Data, int32 TargetIndex)
{
    if(Address.GetDisplayNameEntry()->IsWide())
    {
        const auto tmp = Address.GetPlainNameString();
        UE_LOG(LogOSC, Error, TEXT("Invalid OSC address \"%s\": ASCII only"), *tmp);
        return;
    }
    
    static_assert(sizeof(uint8) == sizeof(char), "Cannot cast uint8 to char");
    uint8 buffer[1024];

    osc::OutboundPacketStream output((char *)buffer, sizeof(buffer));
    output << osc::BeginMessage(Address.GetPlainANSIString());
    for(const auto & elem : Data)
    {
        if(elem.IsFloat())
        {
            output << (float)elem.AsFloatValue();
        }
        else if(elem.IsInt())
        {
            output << (int32)elem.AsIntValue();
        }
        else if(elem.IsBool())
        {
            output << elem.AsBoolValue();
        }
        else if(elem.IsString())
        {
            output << elem.AsStringValue().GetPlainANSIString();
        }
    }
    output << osc::EndMessage;

    check(reinterpret_cast<const void *>(buffer) == reinterpret_cast<const void *>(output.Data()));
    GetMutableDefault<UOscSettings>()->Send(buffer, output.Size(), TargetIndex);
}
