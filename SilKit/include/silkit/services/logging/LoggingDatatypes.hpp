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
    //! Detailed debug-level messages
    Trace = 0,
    //! Normal debug-level messages
    Debug = 1,
    //! Informational content
    Info = 2,
    //! Warnings
    Warn = 3,
    //! Non-critical errors
    Error = 4,
    //! Critical errors
    Critical = 5,
    //! Logging is disabled
    Off = 0xffffffff
};

} // namespace Logging
} // namespace Services
} // namespace SilKit
