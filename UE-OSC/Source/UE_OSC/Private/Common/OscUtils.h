#pragma once
#include "UObject/NameTypes.h"

namespace Osc
{
    template <class CharType>
    void NameToLower(CharType(&Str)[NAME_SIZE])
    {
        for (int32 i = 0; i < NAME_SIZE; ++i)
        {
            if (Str[i] == '\0')
            {
                return;
            }
            Str[i] = TChar<CharType>::ToLower(Str[i]);
        }
    }
}
