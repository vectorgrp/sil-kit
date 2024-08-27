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

#include "spdlog/details/log_msg.h"

#include "silkit/participant/exception.hpp"
#include "silkit/services/logging/LoggingDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace Logging {

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
    throw SilKit::TypeConversionError{};
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
    case spdlog::level::n_levels:
        throw SilKitError(
            "SpdlogTypeConversion: Don't convert the guard. Squelch warning by covering this in the switch");
    }
    throw SilKit::TypeConversionError{};
}

inline auto from_spdlog(const spdlog::source_loc& spdLoc) -> SourceLoc
{
    SourceLoc loc;
    if (spdLoc.filename != nullptr)
    {
        loc.filename = spdLoc.filename;
    }
    loc.line = spdLoc.line;
    if (spdLoc.funcname != nullptr)
    {
        loc.funcname = spdLoc.funcname;
    }

    return loc;
}

inline auto from_spdlog(const spdlog::details::log_msg& spdMsg) -> LogMsg
{
    LogMsg msg;
    if (spdMsg.logger_name.size() > 0)
        msg.loggerName = std::string{spdMsg.logger_name.data(), spdMsg.logger_name.size()};
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
    return spdlog::details::log_msg{to_spdlog(msg.source), msg.loggerName, to_spdlog(msg.level), msg.payload};
}

inline auto to_spdlog(const LogMsg& msg, const std::string& payload) -> spdlog::details::log_msg
{
    return spdlog::details::log_msg{msg.time, to_spdlog(msg.source), msg.loggerName, to_spdlog(msg.level), payload};
}


} // namespace Logging
} // namespace Services
} // namespace SilKit
