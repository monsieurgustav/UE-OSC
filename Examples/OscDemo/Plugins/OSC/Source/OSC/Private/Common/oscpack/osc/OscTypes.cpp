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

#include "OscPrivatePCH.h"

#include "OscTypes.h"

namespace osc{

BundleInitiator BeginBundleImmediate(1);
BundleTerminator EndBundle;
MessageTerminator EndMessage;
NilType OscNil;
#ifndef _OBJC_OBJC_H_
NilType Nil; // Objective-C defines Nil. so our Nil is deprecated. use OscNil instead
#endif
InfinitumType Infinitum;
ArrayInitiator BeginArray;
ArrayTerminator EndArray;

const TCHAR * errorString(Errors value)
{
    static const TCHAR * ERROR_STR[] = {
        TEXT("No Error"),

        TEXT("Malformed packet"),
        TEXT("Malformed packet: invalid size"),
        TEXT("Malformed packet: zero size"),
        TEXT("Malformed packet: not multiple of 4"),

        TEXT("Malformed message"),
        TEXT("Malformed message: invalid size"),
        TEXT("Malformed message: zero size"),
        TEXT("Malformed message: not multiple of 4"),
        TEXT("Malformed message: invalid blob size"),
        TEXT("Malformed message: unterminated address pattern"),
        TEXT("Malformed message: unterminated string"),
        TEXT("Malformed message: unterminated array"),
        TEXT("Malformed message: no type tags"),
        TEXT("Malformed message: unterminated type tags"),
        TEXT("Malformed message: argument exceed msg size"),
        TEXT("Malformed message: unknown type tag"),

        TEXT("Malformed bundle"),
        TEXT("Malformed bundle: invalid size"),
        TEXT("Malformed bundle: invalid address pattern"),
        TEXT("Malformed bundle: too short"),
        TEXT("Malformed bundle: too short for element size"),
        TEXT("Malformed bundle: too short for element"),
        TEXT("Malformed bundle: not multiple of 4"),
        TEXT("Malformed bundle: element not multiple of 4"),
        TEXT("Malformed bundle: content"),

        TEXT("Wrong argument type"),
        TEXT("Missing argument"),
        TEXT("Excess argument"),
        TEXT("Out of buffer memory"),
        };

    static_assert((sizeof(ERROR_STR) / sizeof(*ERROR_STR) == ERRORS_COUNT), "Error messages mismatch");
    if(value < 0 || value >= ERRORS_COUNT)
    {
        check(false);
        return TEXT("Unknown error code");
    }
    return ERROR_STR[value];
}

} // namespace osc
