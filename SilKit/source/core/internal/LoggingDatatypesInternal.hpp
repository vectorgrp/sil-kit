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

#include <chrono>
#include <string>
#include <sstream>
#include <ostream>

#include "silkit/services/logging/LoggingDatatypes.hpp"
#include "silkit/services/logging/string_utils.hpp"

namespace SilKit {
namespace Services {
namespace Logging {

using log_clock = std::chrono::system_clock;

/*! \brief The source location that a log entry refers to
 */
struct SourceLoc
{
    std::string filename;
    uint32_t line;
    std::string funcname;
};

/*! \brief A log entry
 */
struct LogMsg
{
    std::string logger_name;
    Level level{Level::Off};
    log_clock::time_point time;
    SourceLoc source;
    std::string payload;
};

inline bool operator==(const SourceLoc& lhs, const SourceLoc& rhs);
inline bool operator==(const LogMsg& lhs, const LogMsg& rhs);

inline std::string to_string(const SourceLoc& sourceLoc);
inline std::ostream& operator<<(std::ostream& out, const SourceLoc& sourceLoc);

inline std::string to_string(const LogMsg& msg);
inline std::ostream& operator<<(std::ostream& out, const LogMsg& msg);

// ================================================================================
//  Inline Implementations
// ================================================================================

bool operator==(const SourceLoc& lhs, const SourceLoc& rhs)
{
    return lhs.filename == rhs.filename && lhs.line == rhs.line && lhs.funcname == rhs.funcname;
}

inline bool operator==(const LogMsg& lhs, const LogMsg& rhs)
{
    return lhs.logger_name == rhs.logger_name && lhs.level == rhs.level && lhs.time == rhs.time
           && lhs.source == rhs.source && lhs.payload == rhs.payload;
}

std::string to_string(const SourceLoc& sourceLoc)
{
    std::stringstream outStream;
    outStream << sourceLoc;
    return outStream.str();
}

std::ostream& operator<<(std::ostream& out, const SourceLoc& sourceLoc)
{
    return out << "SourceLoc{filename=\"" << sourceLoc.filename << "\","
               << "line=" << sourceLoc.line << ", funcname={\"" << sourceLoc.funcname << "\"}";
}

std::string to_string(const LogMsg& msg)
{
    std::stringstream outStream;
    outStream << msg;
    return outStream.str();
}

std::ostream& operator<<(std::ostream& out, const LogMsg& msg)
{
    out << "LogMsg{logger=" << msg.logger_name << ", level=" << msg.level
        << ", time=" << msg.time.time_since_epoch().count() << ", source=" << msg.source << ", payload=\""
        << msg.payload << "\""
        << "}";
    return out;
}

} // namespace Logging
} // namespace Services
} // namespace SilKit
