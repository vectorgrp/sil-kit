/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

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