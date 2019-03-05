// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include <chrono>
#include <string>

#include "spdlog/common.h"

namespace ib {
namespace mw {
namespace logging {

struct SourceLoc
{
    std::string filename;
    uint32_t line;
    std::string funcname;
};
    
struct LogMsg
{
    std::string logger_name;
    spdlog::level::level_enum level{spdlog::level::off};
    spdlog::log_clock::time_point time;
    SourceLoc source;
    std::string payload;
};

} // namespace logging
} // namespace mw
} // namespace ib
