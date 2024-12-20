// Copyright (c) 2023 Vector Informatik GmbH
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once

#include "silkit/capi/SilKit.h"

#include "silkit/detail/macros.hpp"


namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Impl {

inline void ThrowOnError(SilKit_ReturnCode returnCode);

} // namespace Impl
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit


// ================================================================================
//  Inline Implementations
// ================================================================================

#include <sstream>

#include "silkit/participant/exception.hpp"

namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Impl {

void ThrowOnError(SilKit_ReturnCode returnCode)
{
    if (returnCode != SilKit_ReturnCode_SUCCESS)
    {
        const char* lastErrorCstr = SilKit_GetLastErrorString();
        if (lastErrorCstr == nullptr)
        {
            lastErrorCstr = "(no error message available)";
        }

        const char* returnCodeCstr = nullptr;
        if (SilKit_ReturnCodeToString(&returnCodeCstr, returnCode) != SilKit_ReturnCode_SUCCESS
            || returnCodeCstr == nullptr)
        {
            returnCodeCstr = "Unknown";
        }

        std::ostringstream os;
        os << "SIL Kit: " << returnCodeCstr << " (" << returnCode << "):\n" << lastErrorCstr;

        switch (returnCode)
        {
        case SilKit_ReturnCode_WRONGSTATE:
            throw StateError{os.str()};
        case SilKit_ReturnCode_TYPECONVERSIONERROR:
            throw TypeConversionError{os.str()};
        case SilKit_ReturnCode_CONFIGURATIONERROR:
            throw ConfigurationError{os.str()};
        case SilKit_ReturnCode_PROTOCOLERROR:
            throw ProtocolError{os.str()};
        case SilKit_ReturnCode_ASSERTIONERROR:
            throw AssertionError{os.str()};
        case SilKit_ReturnCode_EXTENSIONERROR:
            throw ExtensionError{os.str()};
        case SilKit_ReturnCode_LOGICERROR:
            throw LogicError{os.str()};
        case SilKit_ReturnCode_LENGTHERROR:
            throw LengthError{os.str()};
        case SilKit_ReturnCode_OUTOFRANGEERROR:
            throw OutOfRangeError{os.str()};
        default:
            throw SilKitError{os.str()};
        }
    }
}

} // namespace Impl
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit
