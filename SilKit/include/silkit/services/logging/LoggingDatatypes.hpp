// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>

namespace SilKit {
namespace Services {
namespace Logging {

/*! \brief Information level of a log message
 */
enum class Level : uint32_t
{
    Trace = 0,       //!< Detailed debug-level messages
    Debug = 1,       //!< Normal debug-level messages
    Info = 2,        //!< Informational content
    Warn = 3,        //!< Warnings
    Error = 4,       //!< Non-critical errors
    Critical = 5,    //!< Critical errors
    Off = 0xffffffff //!< Logging is disabled
};

} // namespace Logging
} // namespace Services
} // namespace SilKit
