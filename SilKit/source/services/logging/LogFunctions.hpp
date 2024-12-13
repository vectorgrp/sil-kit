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
void Log(ILogger* logger, Level level, const char* fmt, const Args&... args)
{
    if (logger && (logger->GetLogLevel() <= level))
    {
        const std::string msg = fmt::format(fmt, args...);
        logger->Log(level, msg);
    }
}

template <typename... Args>
void Trace(ILogger* logger, const char* fmt, const Args&... args)
{
    Log(logger, Level::Trace, fmt, args...);
}
template <typename... Args>
void Debug(ILogger* logger, const char* fmt, const Args&... args)
{
    Log(logger, Level::Debug, fmt, args...);
}

template <typename... Args>
void Debug(ILogger* logger, LogOnceFlag& onceflag, const char* fmt, const Args&... args)
{
    if (onceflag.WasCalled())
    {
        return;
    }

    Log(logger, Level::Debug, fmt, args...);
}

template <typename... Args>
void Info(ILogger* logger, const char* fmt, const Args&... args)
{
    Log(logger, Level::Info, fmt, args...);
}
template <typename... Args>
void Warn(ILogger* logger, const char* fmt, const Args&... args)
{
    Log(logger, Level::Warn, fmt, args...);
}

template <typename... Args>
void Warn(ILogger* logger, LogOnceFlag& onceFlag, const char* fmt, const Args&... args)
{
    if (onceFlag.WasCalled())
    {
        return;
    }

    Log(logger, Level::Warn, fmt, args...);
}

template <typename... Args>
void Error(ILogger* logger, const char* fmt, const Args&... args)
{
    Log(logger, Level::Error, fmt, args...);
}
template <typename... Args>
void Critical(ILogger* logger, const char* fmt, const Args&... args)
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
            os << "MatchingLabel::Kind(" << static_cast<std::underlying_type_t<MatchingLabel::Kind>>(label.kind)
               << ")";
            break;
        }

        os << " '" << label.key << "': '" << label.value << "'";
    }

    return os.str();
}

} // namespace Logging
} // namespace Services
} // namespace SilKit
