// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IdlTypeConversionLogging.hpp"

#include "ib/exception.hpp"
#include "ib/mw/logging/LoggingDatatypes.hpp"
#include "idl/LoggingTopics.h"

namespace ib {
namespace mw {
namespace logging {

auto to_idl(const SourceLoc& msg) -> idl::SourceLoc
{
    idl::SourceLoc idl;
    idl.filename(msg.filename);
    idl.line(msg.line);
    idl.funcname(msg.funcname);
    return idl;
}

auto to_idl(SourceLoc&& msg) -> idl::SourceLoc
{
    idl::SourceLoc idl;
    idl.filename(std::move(msg.filename));
    idl.line(std::move(msg.line));
    idl.funcname(std::move(msg.funcname));
    return idl;
}

auto to_idl(const LogMsg& msg) -> idl::LogMsg
{
    idl::LogMsg idl;

    idl.logger_name(msg.logger_name);
    idl.level(to_idl(msg.level));
    idl.timeUs(std::chrono::duration_cast<std::chrono::microseconds>(msg.time.time_since_epoch()).count());
    idl.source(to_idl(msg.source));
    idl.payload(msg.payload);

    return idl;
}

auto to_idl(LogMsg&& msg) -> idl::LogMsg
{
    idl::LogMsg idl;

    idl.logger_name(std::move(msg.logger_name));
    idl.level(to_idl(msg.level));
    idl.timeUs(std::chrono::duration_cast<std::chrono::microseconds>(msg.time.time_since_epoch()).count());
    idl.source(to_idl(std::move(msg.source)));
    idl.payload(std::move(msg.payload));

    return idl;
}

inline auto to_idl(Level level) -> idl::level::level_enum
{
    namespace idl = idl::level;
    switch (level)
    {
    case Level::Trace:
        return idl::trace;
    case Level::Debug:
        return idl::debug;
    case Level::Info:
        return idl::info;
    case Level::Warn:
        return idl::warn;
    case Level::Error:
        return idl::err;
    case Level::Critical:
        return idl::critical;
    case Level::Off:
        return idl::off;
    }
    throw ib::type_conversion_error{};
}

namespace idl {
namespace level {
inline auto from_idl(level_enum level) -> Level
{
    switch (level)
    {
    case trace:
        return Level::Trace;
    case debug:
        return Level::Debug;
    case info:
        return Level::Info;
    case warn:
        return Level::Warn;
    case err:
        return Level::Error;
    case critical:
        return Level::Critical;
    case off:
        return Level::Off;
    }
    throw ib::type_conversion_error{};
}
}
}

auto idl::from_idl(idl::SourceLoc&& idl) -> logging::SourceLoc
{
    logging::SourceLoc source;
    source.filename = std::move(idl.filename());
    source.line = idl.line();
    source.funcname = idl.funcname();
    return source;
}

auto idl::from_idl(idl::LogMsg&& idl) -> logging::LogMsg
{
    logging::LogMsg msg;

    msg.logger_name = std::move(idl.logger_name());
    msg.level = from_idl(idl.level());
    msg.time = log_clock::time_point(std::chrono::microseconds{idl.timeUs()});
    msg.source = from_idl(std::move(idl.source()));
    msg.payload = std::move(idl.payload());

    return msg;
}
        
} // namespace logging
} // namespace mw
} // namespace ib
