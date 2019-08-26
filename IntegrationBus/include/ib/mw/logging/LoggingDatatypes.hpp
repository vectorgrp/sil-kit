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

} // namespace logging
} // namespace mw
} // namespace ib
