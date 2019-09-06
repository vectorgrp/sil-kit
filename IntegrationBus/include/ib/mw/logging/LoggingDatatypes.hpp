// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <chrono>
#include <string>

namespace ib {
namespace mw {
namespace logging {

using log_clock = std::chrono::system_clock;

enum class Level : uint32_t
{
    trace,
    debug,
    info,
    warn,
    error,
    critical,
    off
};

struct SourceLoc
{
    std::string filename;
    uint32_t line;
    std::string funcname;
};
    
struct LogMsg
{
    std::string logger_name;
    Level level{Level::off};
    log_clock::time_point time;
    SourceLoc source;
    std::string payload;
};

// ================================================================================
//  Inline Implementations
// ================================================================================
inline bool operator==(const SourceLoc& lhs, const SourceLoc& rhs)
{
    return lhs.filename == rhs.filename
        && lhs.line == rhs.line
        && lhs.funcname == rhs.funcname;
}

inline bool operator==(const LogMsg& lhs, const LogMsg& rhs)
{
    return lhs.logger_name == rhs.logger_name
        && lhs.level == rhs.level
        && lhs.time == rhs.time
        && lhs.source == rhs.source
        && lhs.payload == rhs.payload;
}


} // namespace logging
} // namespace mw
} // namespace ib
