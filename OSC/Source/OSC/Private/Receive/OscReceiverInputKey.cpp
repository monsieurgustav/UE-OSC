#include "OscPrivatePCH.h"
#include "OscReceiverInputKey.h"
#include "OscDispatcher.h"

#include "SlateBasics.h"


OscReceiverInputKey::OscReceiverInputKey(const FString &address)
  : _address(address),
    _addressName(*address)
{
    char buffer[512];  // address truncated after 500 chars.
    const auto length = sprintf(buffer, "OSC%.500s", StringCast<ANSICHAR>(*address).Get());

    // replace '/' by '_' because '/' is not a valid FKey character.
    for(int i=0; i!=length; ++i)
    {
        if(buffer[i] == '/')
        {
            buffer[i] = '_';
        }
    }

    _key = FName(buffer);

    RegisterKey();
}

void OscReceiverInputKey::RegisterKey() const
{
    if(!_address.IsEmpty() && !EKeys::GetKeyDetails(_key).IsValid())
    {
        EKeys::AddKey(FKeyDetails(_key,
                                  FText::FromString(_key.ToString()),
                                  FKeyDetails::FloatAxis));
    }
}

void OscReceiverInputKey::SendEvent(const FName & Address, const TArray<FOscDataElemStruct> & Data, const FString & SenderIp)
{
    if(Address != _addressName)
    {
        return;
    }

    auto & app = FSlateApplication::Get();
    
    const auto value = Data.Num() ? Data[0].GetValue<float>() : 0.f;
     
    FKeyEvent keyEvent(_key, app.GetModifierKeys(), 0, false, 0, 0);
    if(value < 0.5f)
    {
        app.ProcessKeyDownEvent(keyEvent);
    }
    else
    {
        app.ProcessKeyUpEvent(keyEvent);
    }

    FAnalogInputEvent axisEvent(_key, app.GetModifierKeys(), 0, false, 0, 0, value);
    app.ProcessAnalogInputEvent(axisEvent);
}
