// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <ostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <cctype>

#include "LoggingDatatypes.hpp"
#include "StringHelpers.hpp"

namespace SilKit {
namespace Services {
namespace Logging {

inline std::string to_string(const Level& level);
inline Level from_string(const std::string& levelStr);
inline std::ostream& operator<<(std::ostream& out, const Level& level);

// ================================================================================
//  Inline Implementations
// ================================================================================

std::string to_string(const Level& level)
{
    std::stringstream outStream;
    outStream << level;
    return outStream.str();
}

std::ostream& operator<<(std::ostream& outStream, const Level& level)
{
    switch (level)
    {
    case Level::Trace:
        outStream << "Trace";
        break;
    case Level::Debug:
        outStream << "Debug";
        break;
    case Level::Info:
        outStream << "Info";
        break;
    case Level::Warn:
        outStream << "Warn";
        break;
    case Level::Error:
        outStream << "Error";
        break;
    case Level::Critical:
        outStream << "Critical";
        break;
    case Level::Off:
        outStream << "Off";
        break;
    default:
        outStream << "Invalid Logging::Level";
    }
    return outStream;
}

inline Level from_string(const std::string& levelStr)
{
    auto logLevel = SilKit::Util::LowerCase(levelStr);
    if (logLevel == "trace")
        return Level::Trace;
    if (logLevel == "debug")
        return Level::Debug;
    if (logLevel == "warn")
        return Level::Warn;
    if (logLevel == "info")
        return Level::Info;
    if (logLevel == "error")
        return Level::Error;
    if (logLevel == "critical")
        return Level::Critical;
    if (logLevel == "off")
        return Level::Off;
    // default to Off
    return Level::Off;
}

} // namespace Logging
} // namespace Services
} // namespace SilKit