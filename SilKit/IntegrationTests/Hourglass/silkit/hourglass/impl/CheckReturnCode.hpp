#pragma once

#include "silkit/capi/SilKit.h"

namespace SilKit {
namespace Hourglass {
namespace Impl {

inline void ThrowOnError(SilKit_ReturnCode returnCode);

}
} // namespace Hourglass
} // namespace SilKit

// ================================================================================
//  Inline Implementations
// ================================================================================

#include "silkit/participant/exception.hpp"

namespace SilKit {
namespace Hourglass {
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
        os << "SIL Kit: " << returnCodeCstr << " (" << returnCode << "): " << lastErrorCstr;

        throw SilKitError{os.str()};
    }
}

} // namespace Impl
} // namespace Hourglass
} // namespace SilKit
