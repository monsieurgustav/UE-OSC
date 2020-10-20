/*
    oscpack -- Open Sound Control (OSC) packet manipulation library
    http://www.rossbencina.com/code/oscpack

    Copyright (c) 2004-2013 Ross Bencina <rossb@audiomulch.com>

    Permission is hereby granted, free of charge, to any person obtaining
    a copy of this software and associated documentation files
    (the "Software"), to deal in the Software without restriction,
    including without limitation the rights to use, copy, modify, merge,
    publish, distribute, sublicense, and/or sell copies of the Software,
    and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
    ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
    CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*
    The text above constitutes the entire oscpack license; however, 
    the oscpack developer(s) also make the following non-binding requests:

    Any person wishing to distribute modifications to the Software is
    requested to send the modifications to the original developer so that
    they can be incorporated into the canonical version. It is also 
    requested that these non-binding requests be included whenever the
    above license is reproduced.
*/
#ifndef INCLUDED_OSCPACK_OSCRECEIVEDELEMENTS_H
#define INCLUDED_OSCPACK_OSCRECEIVEDELEMENTS_H

#include <cassert>
#include <cstddef>
#include <cstring> // size_t

#include "OscTypes.h"


namespace osc{


class ReceivedPacket{
public:
    // Although the OSC spec is not entirely clear on this, we only support
    // packets up to 0x7FFFFFFC bytes long (the maximum 4-byte aligned value
    // representable by an int32). An exception will be raised if you pass a 
    // larger value to the ReceivedPacket() constructor.

    ReceivedPacket( const char *contents, osc_bundle_element_size_t size )
        : contents_( contents )
        , size_( ValidateSize(size, &state_) ) {}

    ReceivedPacket( const char *contents, std::size_t size )
        : contents_( contents )
        , size_( ValidateSize( (osc_bundle_element_size_t)size, &state_ ) ) {}

#if !(defined(__x86_64__) || defined(_M_X64))
    ReceivedPacket( const char *contents, int size )
        : contents_( contents )
        , size_( ValidateSize( (osc_bundle_element_size_t)size, &state_ ) ) {}
#endif

    bool IsMessage() const { return !IsBundle(); }
    bool IsBundle() const;

    osc_bundle_element_size_t Size() const { return size_; }
    const char *Contents() const { return contents_; }
    Errors State() const { return state_; }

private:
    const char *contents_;
    osc_bundle_element_size_t size_;
    Errors state_;

    static osc_bundle_element_size_t ValidateSize( osc_bundle_element_size_t size, Errors * state )
    {
        // sanity check integer types declared in OscTypes.h 
        // you'll need to fix OscTypes.h if any of these asserts fail
        assert( sizeof(osc::int32) == 4 );
        assert( sizeof(osc::uint32) == 4 );
        assert( sizeof(osc::int64) == 8 );
        assert( sizeof(osc::uint64) == 8 );

        if( !IsValidElementSizeValue(size) )
        {
            *state = MALFORMED_PACKET_INVALID_SIZE_ERROR;
            return 0;
        }

        if( size == 0 )
        {
            *state = MALFORMED_PACKET_ZERO_SIZE_ERROR;
            return 0;
        }

        if( !IsMultipleOf4(size) )
        {
            *state = MALFORMED_PACKET_NOT_MULTIPLE_OF_4_ERROR;
            return 0;
        }

        *state = SUCCESS;
        return size;
    }
};


class ReceivedBundleElement{
public:
    ReceivedBundleElement( const char *sizePtr )
        : sizePtr_( sizePtr ) {}

    friend class ReceivedBundleElementIterator;

    bool IsMessage() const { return !IsBundle(); }
    bool IsBundle() const;

    osc_bundle_element_size_t Size() const;
    const char *Contents() const { return sizePtr_ + osc::OSC_SIZEOF_INT32; }

private:
    const char *sizePtr_;
};


class ReceivedBundleElementIterator{
public:
    ReceivedBundleElementIterator( const char *sizePtr )
        : value_( sizePtr ) {}

    ReceivedBundleElementIterator operator++()
    {
        Advance();
        return *this;
    }

    ReceivedBundleElementIterator operator++(int)
    {
        ReceivedBundleElementIterator old( *this );
        Advance();
        return old;
    }

    const ReceivedBundleElement& operator*() const { return value_; }

    const ReceivedBundleElement* operator->() const { return &value_; }

    friend bool operator==(const ReceivedBundleElementIterator& lhs,
            const ReceivedBundleElementIterator& rhs );

private:
    ReceivedBundleElement value_;

    void Advance() { value_.sizePtr_ = value_.Contents() + value_.Size(); }

    bool IsEqualTo( const ReceivedBundleElementIterator& rhs ) const
    {
        return value_.sizePtr_ == rhs.value_.sizePtr_;
    }
};

inline bool operator==(const ReceivedBundleElementIterator& lhs,
        const ReceivedBundleElementIterator& rhs )
{    
    return lhs.IsEqualTo( rhs );
}

inline bool operator!=(const ReceivedBundleElementIterator& lhs,
        const ReceivedBundleElementIterator& rhs )
{
    return !( lhs == rhs );
}


class ReceivedMessageArgument{
public:
    ReceivedMessageArgument( const char *typeTagPtr, const char *argumentPtr )
        : typeTagPtr_( typeTagPtr )
        , argumentPtr_( argumentPtr ) {}

    friend class ReceivedMessageArgumentIterator;
    
    char TypeTag() const { return *typeTagPtr_; }

    // the unchecked methods below don't check whether the argument actually
    // is of the specified type. they should only be used if you've already
    // checked the type tag or the associated IsType() method.

    bool IsBool() const
        { return *typeTagPtr_ == TRUE_TYPE_TAG || *typeTagPtr_ == FALSE_TYPE_TAG; }
    bool AsBool(Errors & state) const;
    bool AsBoolUnchecked(Errors & state) const;

    bool IsNil() const { return *typeTagPtr_ == NIL_TYPE_TAG; }
    bool IsInfinitum() const { return *typeTagPtr_ == INFINITUM_TYPE_TAG; }

    bool IsInt32() const { return *typeTagPtr_ == INT32_TYPE_TAG; }
    int32 AsInt32(Errors & state) const;
    int32 AsInt32Unchecked() const;

    bool IsFloat() const { return *typeTagPtr_ == FLOAT_TYPE_TAG; }
    float AsFloat(Errors & state) const;
    float AsFloatUnchecked() const;

    bool IsChar() const { return *typeTagPtr_ == CHAR_TYPE_TAG; }
    char AsChar(Errors & state) const;
    char AsCharUnchecked() const;

    bool IsRgbaColor() const { return *typeTagPtr_ == RGBA_COLOR_TYPE_TAG; }
    uint32 AsRgbaColor(Errors & state) const;
    uint32 AsRgbaColorUnchecked() const;

    bool IsMidiMessage() const { return *typeTagPtr_ == MIDI_MESSAGE_TYPE_TAG; }
    uint32 AsMidiMessage(Errors & state) const;
    uint32 AsMidiMessageUnchecked() const;

    bool IsInt64() const { return *typeTagPtr_ == INT64_TYPE_TAG; }
    int64 AsInt64(Errors & state) const;
    int64 AsInt64Unchecked() const;

    bool IsTimeTag() const { return *typeTagPtr_ == TIME_TAG_TYPE_TAG; }
    uint64 AsTimeTag(Errors & state) const;
    uint64 AsTimeTagUnchecked() const;

    bool IsDouble() const { return *typeTagPtr_ == DOUBLE_TYPE_TAG; }
    double AsDouble(Errors & state) const;
    double AsDoubleUnchecked() const;

    bool IsString() const { return *typeTagPtr_ == STRING_TYPE_TAG; }
    const char* AsString(Errors & state) const;
    const char* AsStringUnchecked() const { return argumentPtr_; }

    bool IsSymbol() const { return *typeTagPtr_ == SYMBOL_TYPE_TAG; }
    const char* AsSymbol(Errors & state) const;
    const char* AsSymbolUnchecked() const { return argumentPtr_; }

    bool IsBlob() const { return *typeTagPtr_ == BLOB_TYPE_TAG; }
    void AsBlob( const void*& data, osc_bundle_element_size_t& size, Errors & state ) const;
    void AsBlobUnchecked( const void*& data, osc_bundle_element_size_t& size, Errors & state ) const;
    
    bool IsArrayBegin() const { return *typeTagPtr_ == ARRAY_BEGIN_TYPE_TAG; }
    bool IsArrayEnd() const { return *typeTagPtr_ == ARRAY_END_TYPE_TAG; }
    // Calculate the number of top-level items in the array. Nested arrays count as one item.
    // Only valid at array start. Will throw an exception if IsArrayStart() == false.
    std::size_t ComputeArrayItemCount(Errors & state) const;

private:
    const char *typeTagPtr_;
    const char *argumentPtr_;
};


class ReceivedMessageArgumentIterator{
public:
    ReceivedMessageArgumentIterator( const char *typeTags, const char *arguments )
        : value_( typeTags, arguments ) {}

    ReceivedMessageArgumentIterator operator++()
    {
        Advance();
        return *this;
    }

    ReceivedMessageArgumentIterator operator++(int)
    {
        ReceivedMessageArgumentIterator old( *this );
        Advance();
        return old;
    }

    const ReceivedMessageArgument& operator*() const { return value_; }

    const ReceivedMessageArgument* operator->() const { return &value_; }

    friend bool operator==(const ReceivedMessageArgumentIterator& lhs,
            const ReceivedMessageArgumentIterator& rhs );

private:
    ReceivedMessageArgument value_;

    void Advance();

    bool IsEqualTo( const ReceivedMessageArgumentIterator& rhs ) const
    {
        return value_.typeTagPtr_ == rhs.value_.typeTagPtr_;
    }
};

inline bool operator==(const ReceivedMessageArgumentIterator& lhs,
        const ReceivedMessageArgumentIterator& rhs )
{    
    return lhs.IsEqualTo( rhs );
}

inline bool operator!=(const ReceivedMessageArgumentIterator& lhs,
        const ReceivedMessageArgumentIterator& rhs )
{    
    return !( lhs == rhs );
}


class ReceivedMessageArgumentStream{
    friend class ReceivedMessage;
    ReceivedMessageArgumentStream( const ReceivedMessageArgumentIterator& begin,
            const ReceivedMessageArgumentIterator& end )
        : p_( begin )
        , end_( end )
        , state_( SUCCESS ) {}

    ReceivedMessageArgumentIterator p_, end_;
    Errors state_;
    
public:

    Errors State() const { return state_; }

    // end of stream
    bool Eos() const { return p_ == end_; }

    ReceivedMessageArgumentStream& operator>>( bool& rhs )
    {
        if( Eos() )
        {
            state_ = MISSING_ARGUMENT_ERROR;
        }
        else
        {
            rhs = (*p_++).AsBool(state_);
        }
        return *this;
    }

    // not sure if it would be useful to stream Nil and Infinitum
    // for now it's not possible
    // same goes for array boundaries

    ReceivedMessageArgumentStream& operator>>( int32& rhs )
    {
        if( Eos() )
        {
            state_ = MISSING_ARGUMENT_ERROR;
        }
        else
        {
            rhs = (*p_++).AsInt32(state_);
        }
        return *this;
    }     

    ReceivedMessageArgumentStream& operator>>( float& rhs )
    {
        if( Eos() )
        {
            state_ = MISSING_ARGUMENT_ERROR;
        }
        else
        {
            rhs = (*p_++).AsFloat(state_);
        }
        return *this;
    }

    ReceivedMessageArgumentStream& operator>>( char& rhs )
    {
        if( Eos() )
        {
            state_ = MISSING_ARGUMENT_ERROR;
        }
        else
        {
            rhs = (*p_++).AsChar(state_);
        }
        return *this;
    }

    ReceivedMessageArgumentStream& operator>>( RgbaColor& rhs )
    {
        if( Eos() )
        {
            state_ = MISSING_ARGUMENT_ERROR;
        }
        else
        {
            rhs.value = (*p_++).AsRgbaColor(state_);
        }
        return *this;
    }

    ReceivedMessageArgumentStream& operator>>( MidiMessage& rhs )
    {
        if( Eos() )
        {
            state_ = MISSING_ARGUMENT_ERROR;
        }
        else
        {
            rhs.value = (*p_++).AsMidiMessage(state_);
        }
        return *this;
    }

    ReceivedMessageArgumentStream& operator>>( int64& rhs )
    {
        if( Eos() )
        {
            state_ = MISSING_ARGUMENT_ERROR;
        }
        else
        {
            rhs = (*p_++).AsInt64(state_);
        }
        return *this;
    }
    
    ReceivedMessageArgumentStream& operator>>( TimeTag& rhs )
    {
        if( Eos() )
        {
            state_ = MISSING_ARGUMENT_ERROR;
        }
        else
        {
            rhs.value = (*p_++).AsTimeTag(state_);
        }
        return *this;
    }

    ReceivedMessageArgumentStream& operator>>( double& rhs )
    {
        if( Eos() )
        {
            state_ = MISSING_ARGUMENT_ERROR;
        }
        else
        {
            rhs = (*p_++).AsDouble(state_);
        }
        return *this;
    }

    ReceivedMessageArgumentStream& operator>>( Blob& rhs )
    {
        if( Eos() )
        {
            state_ = MISSING_ARGUMENT_ERROR;
        }
        else
        {
            (*p_++).AsBlob( rhs.data, rhs.size, state_ );
        }
        return *this;
    }
    
    ReceivedMessageArgumentStream& operator>>( const char*& rhs )
    {
        if( Eos() )
        {
            state_ = MISSING_ARGUMENT_ERROR;
        }
        else
        {
            rhs = (*p_++).AsString(state_);
        }
        return *this;
    }
    
    ReceivedMessageArgumentStream& operator>>( Symbol& rhs )
    {
        if( Eos() )
        {
            state_ = MISSING_ARGUMENT_ERROR;
        }
        else
        {
            rhs.value = (*p_++).AsSymbol(state_);
        }
        return *this;
    }

    ReceivedMessageArgumentStream& operator>>( MessageTerminator& rhs )
    {
        (void) rhs; // suppress unused parameter warning

        if( Eos() )
        {
            state_ = EXCESS_ARGUMENT_ERROR;
        }
        return *this;
    }
};


class ReceivedMessage{
    void Init( const char *bundle, osc_bundle_element_size_t size );
public:
    explicit ReceivedMessage( const ReceivedPacket& packet );
    explicit ReceivedMessage( const ReceivedBundleElement& bundleElement );

    Errors State() const { return state_; }

    const char *AddressPattern() const { return addressPattern_; }

    // Support for non-standard SuperCollider integer address patterns:
    bool AddressPatternIsUInt32() const;
    uint32 AddressPatternAsUInt32() const;

    uint32 ArgumentCount() const { return static_cast<uint32>(typeTagsEnd_ - typeTagsBegin_); }

    const char *TypeTags() const { return typeTagsBegin_; }


    typedef ReceivedMessageArgumentIterator const_iterator;
    
    ReceivedMessageArgumentIterator ArgumentsBegin() const
    {
        return ReceivedMessageArgumentIterator( typeTagsBegin_, arguments_ );
    }
     
    ReceivedMessageArgumentIterator ArgumentsEnd() const
    {
        return ReceivedMessageArgumentIterator( typeTagsEnd_, 0 );
    }

    ReceivedMessageArgumentStream ArgumentStream() const
    {
        return ReceivedMessageArgumentStream( ArgumentsBegin(), ArgumentsEnd() );
    }

private:
    const char *addressPattern_;
    const char *typeTagsBegin_;
    const char *typeTagsEnd_;
    const char *arguments_;
    Errors state_;
};


class ReceivedBundle{
    void Init( const char *message, osc_bundle_element_size_t size );
public:
    explicit ReceivedBundle( const ReceivedPacket& packet );
    explicit ReceivedBundle( const ReceivedBundleElement& bundleElement );

    Errors State() const { return state_; }

    uint64 TimeTag() const;

    uint32 ElementCount() const { return elementCount_; }

    typedef ReceivedBundleElementIterator const_iterator;
    
    ReceivedBundleElementIterator ElementsBegin() const
    {
        return ReceivedBundleElementIterator( timeTag_ + 8 );
    }
     
    ReceivedBundleElementIterator ElementsEnd() const
    {
        return ReceivedBundleElementIterator( end_ );
    }

private:
    const char *timeTag_;
    const char *end_;
    uint32 elementCount_;
    Errors state_;
};


} // namespace osc


#endif /* INCLUDED_OSCPACK_OSCRECEIVEDELEMENTS_H */
