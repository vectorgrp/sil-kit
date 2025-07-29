// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

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
