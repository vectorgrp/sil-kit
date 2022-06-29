// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "spdlog/details/log_msg.h"

#include "ib/exception.hpp"
#include "ib/mw/logging/LoggingDatatypes.hpp"

namespace ib {
namespace mw {
namespace logging {

inline auto to_spdlog(Level level) -> spdlog::level::level_enum
{
    switch (level)
    {
    case Level::Trace:
        return spdlog::level::trace;
    case Level::Debug:
        return spdlog::level::debug;
    case Level::Info:
        return spdlog::level::info;
    case Level::Warn:
        return spdlog::level::warn;
    case Level::Error:
        return spdlog::level::err;
    case Level::Critical:
        return spdlog::level::critical;
    case Level::Off:
        return spdlog::level::off;
    }
    throw ib::TypeConversionError{};
}

inline auto from_spdlog(spdlog::level::level_enum level) -> Level
{
    switch (level)
    {
    case spdlog::level::trace:
        return Level::Trace;
    case spdlog::level::debug:
        return Level::Debug;
    case spdlog::level::info:
        return Level::Info;
    case spdlog::level::warn:
        return Level::Warn;
    case spdlog::level::err:
        return Level::Error;
    case spdlog::level::critical:
        return Level::Critical;
    case spdlog::level::off:
        return Level::Off;
    }
    throw ib::TypeConversionError{};
}

inline auto from_spdlog(const spdlog::source_loc& spdLoc) -> SourceLoc
{
    SourceLoc loc;
    loc.filename = spdLoc.filename;
    loc.line = spdLoc.line;
    loc.funcname = spdLoc.funcname;

    return loc;
}

inline auto from_spdlog(const spdlog::details::log_msg& spdMsg) -> LogMsg
{
    LogMsg msg;
    if (spdMsg.logger_name)
        msg.logger_name = *spdMsg.logger_name;
    msg.level = from_spdlog(spdMsg.level);
    msg.time = spdMsg.time;
    msg.source = from_spdlog(spdMsg.source);
    msg.payload = std::string{spdMsg.payload.begin(), spdMsg.payload.end()};

    return msg;
}

inline auto to_spdlog(const SourceLoc& loc) -> spdlog::source_loc
{
    return spdlog::source_loc{loc.filename.c_str(), static_cast<int>(loc.line), loc.funcname.c_str()};
}

inline auto to_spdlog(const LogMsg& msg) -> spdlog::details::log_msg
{
    return spdlog::details::log_msg{
        to_spdlog(msg.source),
        &msg.logger_name,
        to_spdlog(msg.level),
        msg.payload
    };
}

} // namespace logging
} // namespace mw
} // namespace ib
