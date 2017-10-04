#include "OscPrivatePCH.h"
#include "OscSettings.h"
#include "OscFunctionLibrary.h"
#include "oscpack/osc/OscOutboundPacketStream.h"


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

void UOscFunctionLibrary::PopBlob(const TArray<FOscDataElemStruct> & input, TArray<FOscDataElemStruct> & output, TArray<uint8> & Value)
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

void UOscFunctionLibrary::PushBlob(const TArray<FOscDataElemStruct> & input, const TArray<uint8> & Value, TArray<FOscDataElemStruct> & output)
{
    output = input;
    FOscDataElemStruct elem;
    elem.SetBlob(Value);
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

TArray<uint8> UOscFunctionLibrary::AsBlob(const FOscDataElemStruct & input)
{
    return input.GetValue<TArray<uint8>>();
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

FOscDataElemStruct UOscFunctionLibrary::FromBlob(const TArray<uint8> & input)
{
    FOscDataElemStruct result;
    result.SetBlob(input);
    return result;
}


namespace
{
    bool isValidAddress(const FName & Address)
    {
        if(!Address.IsValid())
        {
            UE_LOG(LogOSC, Error, TEXT("Empty OSC address"));
            return false;
        }

        if(Address.GetDisplayNameEntry()->IsWide())
        {
            const auto tmp = Address.GetPlainNameString();
            UE_LOG(LogOSC, Error, TEXT("Invalid OSC address \"%s\": ASCII only"), *tmp);
            return false;
        }

        if(Address.GetPlainANSIString()[0] != '/')
        {
            const auto tmp = Address.GetPlainNameString();
            UE_LOG(LogOSC, Error, TEXT("Invalid OSC address \"%s\": must start with '/'"), *tmp);
            return false;
        }

        return true;
    }

    void appendMessage(osc::OutboundPacketStream & output, FName Address, const TArray<FOscDataElemStruct> & Data)
    {
        output << osc::BeginMessage(Address.GetPlainANSIString());
        if(output.State() != osc::SUCCESS)
        {
            return;
        }

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
            else if(elem.IsBlob())
            {
                const TArray<uint8> & value = elem.AsBlobValue();
                output << osc::Blob(value.GetData(), value.Num());
            }

            if(output.State() != osc::SUCCESS)
            {
                return;
            }
        }
        output << osc::EndMessage;
    }

    TArray<uint8> GlobalBuffer(TArray<uint8>(), 1024);
}

void UOscFunctionLibrary::SendOsc(FName Address, const TArray<FOscDataElemStruct> & Data, int32 TargetIndex)
{
    if(!isValidAddress(Address))
    {
        return;
    }

    static_assert(sizeof(uint8) == sizeof(char), "Cannot cast uint8 to char");

    osc::OutboundPacketStream output((char *)GlobalBuffer.GetData(), GlobalBuffer.Max());
    check(reinterpret_cast<const void *>(GlobalBuffer.GetData()) == reinterpret_cast<const void *>(output.Data()));

    appendMessage(output, Address, Data);

    if(output.State() == osc::OUT_OF_BUFFER_MEMORY_ERROR)
    {
        GlobalBuffer.Reserve(GlobalBuffer.Max() * 2);  // not enough memory: double the size
        SendOsc(Address, Data, TargetIndex);  // try again
        return;
    }

    if(output.State() == osc::SUCCESS)
    {
        GetMutableDefault<UOscSettings>()->Send(GlobalBuffer.GetData(), output.Size(), TargetIndex);
    }
    else
    {
        UE_LOG(LogOSC, Error, TEXT("OSC Send Message Error: %s"), osc::errorString(output.State()));
    }
}

void UOscFunctionLibrary::SendOscBundle(const TArray<FOscMessageStruct> & Messages, int32 TargetIndex)
{
    static_assert(sizeof(uint8) == sizeof(char), "Cannot cast uint8 to char");

    osc::OutboundPacketStream output((char *)GlobalBuffer.GetData(), GlobalBuffer.Max());
    check(reinterpret_cast<const void *>(GlobalBuffer.GetData()) == reinterpret_cast<const void *>(output.Data()));

    output << osc::BeginBundle();
    for(const auto & message : Messages)
    {
        if(!isValidAddress(message.Address))
        {
            return;
        }

        appendMessage(output, message.Address, message.Data);

        if(output.State() == osc::OUT_OF_BUFFER_MEMORY_ERROR)
        {
            GlobalBuffer.Reserve(GlobalBuffer.Max() * 2);  // not enough memory: double the size
            SendOscBundle(Messages, TargetIndex);  // try again
            return;
        }
        if(output.State() != osc::SUCCESS)
        {
            UE_LOG(LogOSC, Error, TEXT("OSC Send Bundle Error: %s"), osc::errorString(output.State()));
            return;
        }
    }
    output << osc::EndBundle;

    if(output.State() == osc::OUT_OF_BUFFER_MEMORY_ERROR)
    {
        GlobalBuffer.Reserve(GlobalBuffer.Max() * 2);  // not enough memory: double the size
        SendOscBundle(Messages, TargetIndex);  // try again
        return;
    }

    if(output.State() == osc::SUCCESS)
    {
        GetMutableDefault<UOscSettings>()->Send(GlobalBuffer.GetData(), output.Size(), TargetIndex);
    }
    else
    {
        UE_LOG(LogOSC, Error, TEXT("OSC Send Bundle Error: %s"), osc::errorString(output.State()));
    }
}

int32 UOscFunctionLibrary::AddSendOscTarget(FString IpPort)
{
    return GetMutableDefault<UOscSettings>()->GetOrAddSendTarget(IpPort);
}
