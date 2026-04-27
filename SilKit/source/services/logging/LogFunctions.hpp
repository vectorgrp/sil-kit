// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <atomic>


#include "StructuredLoggingKeys.hpp"
#include "SilKitFmtFormatters.hpp"

#include <string>


namespace SilKit {
namespace Services {
namespace Logging {

class LogOnceFlag
{
    std::atomic_bool _once{false};

public:
    operator bool() const
    {
        return _once.load();
    }
    bool WasCalled()
    {
        bool expected{false};
        return !_once.compare_exchange_strong(expected, true);
    }
};

template <typename... Args>
void Log(ILoggerInternal* logger, Level level, const char* fmt, const Args&... args)
{
    if (logger && (logger->GetLogLevel() <= level))
    {
        const std::string msg = fmt::format(fmt, args...);
        logger->Log(level, Topic::None, msg);
    }
}

template <typename... Args>
void Trace(ILoggerInternal* logger, const char* fmt, const Args&... args)
{
    Log(logger, Level::Trace, fmt, args...);
}
template <typename... Args>
void Debug(ILoggerInternal* logger, const char* fmt, const Args&... args)
{
    Log(logger, Level::Debug, fmt, args...);
}

template <typename... Args>
void Debug(ILoggerInternal* logger, LogOnceFlag& onceflag, const char* fmt, const Args&... args)
{
    if (onceflag.WasCalled())
    {
        return;
    }

    Log(logger, Level::Debug, fmt, args...);
}

template <typename... Args>
void Info(ILoggerInternal* logger, const char* fmt, const Args&... args)
{
    Log(logger, Level::Info, fmt, args...);
}
template <typename... Args>
void Warn(ILoggerInternal* logger, const char* fmt, const Args&... args)
{
    Log(logger, Level::Warn, fmt, args...);
}

template <typename... Args>
void Warn(ILoggerInternal* logger, LogOnceFlag& onceFlag, const char* fmt, const Args&... args)
{
    if (onceFlag.WasCalled())
    {
        return;
    }

    Log(logger, Level::Warn, fmt, args...);
}

template <typename... Args>
void Error(ILoggerInternal* logger, const char* fmt, const Args&... args)
{
    Log(logger, Level::Error, fmt, args...);
}
template <typename... Args>
void Critical(ILoggerInternal* logger, const char* fmt, const Args&... args)
{
    Log(logger, Level::Critical, fmt, args...);
}


inline auto FormatLabelsForLogging(const std::vector<MatchingLabel>& labels) -> std::string
{
    std::ostringstream os;

    if (labels.empty())
    {
        os << "(no labels)";
    }

    bool first = true;

    for (const auto& label : labels)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            os << ", ";
        }

        switch (label.kind)
        {
        case MatchingLabel::Kind::Optional:
            os << "Optional";
            break;
        case MatchingLabel::Kind::Mandatory:
            os << "Mandatory";
            break;
        default:
            os << "MatchingLabel::Kind(" << static_cast<std::underlying_type_t<MatchingLabel::Kind>>(label.kind) << ")";
            break;
        }

        os << " '" << label.key << "': '" << label.value << "'";
    }

    return os.str();
}

} // namespace Logging
} // namespace Services
} // namespace SilKit
