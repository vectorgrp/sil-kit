// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <chrono>
#include <string>
#include <sstream>
#include <ostream>
#include <vector>

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
    std::string loggerName;
    Level level{Level::Off};
    log_clock::time_point time;
    SourceLoc source;
    std::string payload;
    std::vector<std::pair<std::string, std::string>> keyValues;
};

inline bool operator==(const SourceLoc& lhs, const SourceLoc& rhs);
inline bool operator==(const LogMsg& lhs, const LogMsg& rhs);

inline std::string to_string(const SourceLoc& sourceLoc);
inline std::ostream& operator<<(std::ostream& out, const SourceLoc& sourceLoc);

inline std::string to_string(const LogMsg& msg);
inline std::ostream& operator<<(std::ostream& out, const LogMsg& msg);


inline std::string to_string(const std::vector<std::pair<std::string, std::string>>& kv);
inline std::ostream& operator<<(std::ostream& out, const std::vector<std::pair<std::string, std::string>>& kv);

// ================================================================================
//  Inline Implementations
// ================================================================================

bool operator==(const SourceLoc& lhs, const SourceLoc& rhs)
{
    return lhs.filename == rhs.filename && lhs.line == rhs.line && lhs.funcname == rhs.funcname;
}

inline bool operator==(const LogMsg& lhs, const LogMsg& rhs)
{
    return lhs.loggerName == rhs.loggerName && lhs.level == rhs.level && lhs.time == rhs.time
           && lhs.source == rhs.source && lhs.payload == rhs.payload && lhs.keyValues == rhs.keyValues;
}

std::string to_string(const SourceLoc& sourceLoc)
{
    std::stringstream outStream;
    outStream << sourceLoc;
    return outStream.str();
}

std::ostream& operator<<(std::ostream& out, const SourceLoc& sourceLoc)
{
    return out << "SourceLoc{filename=\"" << sourceLoc.filename << "\"," << "line=" << sourceLoc.line
               << ", funcname={\"" << sourceLoc.funcname << "\"}";
}


inline std::string to_string(const std::vector<std::pair<std::string, std::string>>& kv)
{
    std::stringstream outStream;
    outStream << kv;
    return outStream.str();
}


inline std::ostream& operator<<(std::ostream& out, const std::vector<std::pair<std::string, std::string>>& kv)
{
    std::string result;
    result.reserve(kv.size() * 2);

    if (kv.size() > 0)
    {
        result.append(", kv: ");

        std::vector<std::pair<std::string, std::string>>::const_iterator it = kv.begin();
        result.append("{");
        while (it != kv.end())
        {
            if (it != kv.begin())
            {
                result.append(",");
            }
            result.append("\"" + it->first + "\"" + ":" + "\"" + it->second + "\"");
            ++it;
        }
        result.append("}");
    }
    return out << result;
}


std::string to_string(const LogMsg& msg)
{
    std::stringstream outStream;
    outStream << msg;
    return outStream.str();
}


std::ostream& operator<<(std::ostream& out, const LogMsg& msg)
{
    out << "LogMsg{logger=" << msg.loggerName << ", level=" << msg.level
        << ", time=" << std::chrono::duration_cast<std::chrono::microseconds>(msg.time.time_since_epoch()).count()
        << ", source=" << msg.source << ", payload=\"" << msg.payload << "\"" << msg.keyValues << "}";
    return out;
}

} // namespace Logging
} // namespace Services
} // namespace SilKit
