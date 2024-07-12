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

#include "silkit/services/logging/ILogger.hpp"

#include "SilKitFmtFormatters.hpp"
#include "fmt/format.h"
#include <unordered_map>
#include <string>




namespace SilKit {
namespace Services {
namespace Logging {

class LoggerMessage;


struct ILoggerInternal : ILogger
{
    virtual void Log(const LoggerMessage& msg) = 0;
    virtual void Log(const LogMsg& msg) = 0;
};



template <typename... Args>
void Log(ILogger* logger, Level level, const char* fmt, const Args&... args);


class LoggerMessage
{
public:
    LoggerMessage(ILoggerInternal* logger, Level level)
        : _logger(logger)
        , _level(level)
        , _minlevel(logger->GetLogLevel())
    {}

    LoggerMessage(ILoggerInternal* logger)
        : _logger(logger)
        , _level(Level::Trace)
        , _minlevel(logger->GetLogLevel())
    {}

    LoggerMessage(ILoggerInternal* logger, const LogMsg& msg)
        : _logger(logger)
        , _level(msg.level)
        , _msg(msg.payload)
        , _keyValues(msg.keyValues)
    {}

    template <typename... Args>
    void SetMessage(const char* fmt, const Args&... args)
    {
        _msg = fmt::format(fmt, args...);
    }

    void SetMessage(std::string newMsg)
    {
        _msg = newMsg;
    }

    void AddKeyValue(std::string key, std::string value)
    {       
        _keyValues[key] = value;
    }

    Level GetLevel() const
    {
        return _level;
    }

    std::unordered_map<std::string, std::string> GetKeyValues() const
    {
        return _keyValues;
    }

    bool HasKeyValues() const
    {
        return _keyValues.size() > 0 ? true : false;
    }

    std::string GetMsgString() const
    {
        return _msg;
    }

    ILoggerInternal* GetLogger() const
    {
        return _logger;
    }

    void Dispatch()
    {
        if ((_minlevel <= _level))
        {
            _logger->Log(*this);
        }
    }


private:
    ILoggerInternal* _logger;
    std::unordered_map<std::string, std::string> _keyValues;
    Level _level;
    Level _minlevel;
    std::string _msg;
};


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
} // namespace Logging
} // namespace Services
} // namespace SilKit
