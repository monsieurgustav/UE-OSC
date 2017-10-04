#pragma once

#include "OscDataElemStruct.generated.h"


USTRUCT(BlueprintType)
struct FOscDataElemStruct
{
    GENERATED_USTRUCT_BODY()

    enum
    {
        FLOAT = 0,
        INT = 1,
        BOOL = 2,
        STRING = 3,
        BLOB = 4,
    };

public:

    FOscDataElemStruct() : Type(0), Data(0)
    {
    }

    void SetFloat(double value)
    {
        static_assert(sizeof(value) <= sizeof(Data), "Too short to hold the float value.");
        Type = FLOAT;
        Data = *reinterpret_cast<int64 *>(&value);
        check(value == AsFloatValue());
    }

    double AsFloatValue() const
    {
        check(IsFloat());
        return *reinterpret_cast<const double *>(&Data);
    }

    bool IsFloat() const
    {
        return Type == FLOAT;
    }

    //-------------------------------------------------------------------------

    void SetInt(int64 value)
    {
        static_assert(sizeof(value) <= sizeof(Data), "Too short to hold the integer value.");
        Type = INT;
        Data = value;
        check(value == AsIntValue());
    }

    int64 AsIntValue() const
    {
        check(IsInt());
        return Data;
    }

    bool IsInt() const
    {
        return Type == INT;
    }

    //-------------------------------------------------------------------------

    void SetBool(bool value)
    {
        static_assert(sizeof(value) <= sizeof(Data), "Too short to hold the bool value.");
        Type = BOOL;
        Data = value;
        check(value == AsBoolValue());
    }

    bool AsBoolValue() const
    {
        check(IsBool());
        return Data != 0;
    }

    bool IsBool() const
    {
        return Type == BOOL;
    }

    //-------------------------------------------------------------------------

    void SetString(FName value)
    {
        static_assert(sizeof(FMinimalName) <= sizeof(Data), "Too short to hold the string value.");
        const auto mininal = NameToMinimalName(value);
        Data = *reinterpret_cast<const int64*>(&mininal);
        Type = STRING;
        check(value == AsStringValue());
    }

    FName AsStringValue() const
    {
        check(IsString());
        return MinimalNameToName(*reinterpret_cast<const FMinimalName *>(&Data));
    }

    bool IsString() const
    {
        return Type == STRING;
    }

    //-------------------------------------------------------------------------

    void SetBlob(TArray<uint8> value)
    {
        Data = 0;
        Type = BLOB;
        Blob = MakeShared<TArray<uint8>>(std::move(value));
    }

    const TArray<uint8> & AsBlobValue() const
    {
        check(IsBlob());
        static const TArray<uint8> empty;
        return Blob.IsValid() ? *Blob : empty;
    }

    bool IsBlob() const
    {
        return Type == BLOB;
    }

    //-------------------------------------------------------------------------

    /// Cast the value to T.
    template <class T>
    T GetValue() const
    {
        static const bool isNumber = (std::is_integral<T>::value || std::is_floating_point<T>::value);
        if(IsFloat())
        {
            if(isNumber)
            {
                const auto value = AsFloatValue();
                return Cast<T, isNumber>::eval(value);
            }
        }
        else if(IsInt())
        {
            if(isNumber)
            {
                const auto value = AsIntValue();
                return Cast<T, isNumber>::eval(value);
            }
        }
        else if(IsBool())
        {
            if(isNumber)
            {
                const auto value = AsBoolValue();
                return Cast<T, isNumber>::eval(value);
            }
        }
        else if(IsString())
        {
            const bool isString = std::is_same<T, FName>::value;
            if(isString)
            {
                const auto value = AsStringValue();
                return Cast<T, isString>::eval(value);
            }
        }
        else if(IsBlob())
        {
            const bool isBlob = std::is_same<T, TArray<uint8>>::value;
            if(isBlob)
            {
                return Cast<T, isBlob>::eval(AsBlobValue());
            }
        }
        return T();
    }

private:
    template <class T, bool>
    struct Cast
    {
        template <class U>
        static T eval(const U &)
        {
            return T();
        }
    };

private:
    int32 Type;

    int64 Data;

    TSharedPtr<TArray<uint8>> Blob;
};


template <class T>
struct FOscDataElemStruct::Cast<T, true>
{
    static T eval(double value)
    {
        if(std::is_integral<T>::value)
        {
            value += 0.4999999;
        }
        return static_cast<T>(value);
    }

    template <class U>
    static T eval(const U & value)
    {
        return static_cast<T>(value);
    }
};

template <>
struct FOscDataElemStruct::Cast<bool, true>
{
    template <class U>
    static bool eval(const U & value)
    {
        return value != 0;
    }
};
