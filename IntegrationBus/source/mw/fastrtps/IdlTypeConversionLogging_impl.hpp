// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IdlTypeConversionLogging.hpp"

#include "ib/exception.hpp"
#include "ib/mw/logging/LoggingDatatypes.hpp"
#include "idl/LoggingTopics.h"

#include "spdlog/details/log_msg.h"


namespace spdlog {
namespace level {
auto to_idl(level_enum level) -> ib::mw::logging::idl::level::level_enum
{
    namespace idl = ib::mw::logging::idl::level;
    switch (level)
    {
    case trace:
        return idl::trace;
    case debug:
        return idl::debug;
    case info:
        return idl::info;
    case warn:
        return idl::warn;
    case err:
        return idl::err;
    case critical:
        return idl::critical;
    case off:
        return idl::off;
    }
    throw ib::type_conversion_error{};
}
} // namespace spdlog
} // namespace level


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

namespace idl {
namespace level {
    auto from_idl(level_enum level) -> spdlog::level::level_enum
    {
        switch (level)
        {
        case trace:
            return spdlog::level::trace;
        case debug:
            return spdlog::level::debug;
        case info:
            return spdlog::level::info;
        case warn:
            return spdlog::level::warn;
        case err:
            return spdlog::level::err;
        case critical:
            return spdlog::level::critical;
        case off:
            return spdlog::level::off;
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
    msg.time = spdlog::log_clock::time_point(std::chrono::microseconds{idl.timeUs()});
    msg.source = from_idl(std::move(idl.source()));
    msg.payload = std::move(idl.payload());

    return msg;
}
        
} // namespace logging
} // namespace mw
} // namespace ib
