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

#include "ILoggerInternal.hpp"
#include "LogFunctions.hpp"

#include <string>


namespace SilKit {
namespace Services {
namespace Logging {


// forwards
template <typename... Args>
void Log(ILogger* logger, Level level, const char* fmt, const Args&... args);
inline auto FormatLabelsForLogging(const std::vector<MatchingLabel>& labels) -> std::string;

class LoggerMessage
{
public:
    LoggerMessage(ILoggerInternal* logger, Level level)
        : _logger(logger)
        , _level(level)
    {
    }

    LoggerMessage(ILoggerInternal* logger)
        : _logger(logger)
        , _level(Level::Trace)
    {
    }

    LoggerMessage(ILoggerInternal* logger, const LogMsg& msg)
        : _logger(logger)
        , _level(msg.level)
        , _msg(msg.payload)
        , _keyValues(msg.keyValues)
    {
    }


    template <typename... Args>
    void FormatMessage(fmt::format_string<Args...> fmt, Args&&... args)
    {
        if (_logger->GetLogLevel() <= _level)
        {
            _msg = fmt::format(fmt, std::forward<Args>(args)...);
        }
    }

    void SetMessage(std::string newMsg)
    {
        if (_logger->GetLogLevel() <= _level)
        {
            _msg = std::move(newMsg);
        }
    }


    template <typename Key, typename... Args>
    void FormatKeyValue(Key&& key, fmt::format_string<Args...> fmt, Args&&... args)
    {
        if (_logger->GetLogLevel() <= _level)
        {
            auto&& formattedValue = fmt::format(fmt, std::forward<Args>(args)...);
            _keyValues.emplace_back(std::forward<Key>(key), formattedValue);
        }
    }


    void SetKeyValue(const std::vector<Services::MatchingLabel>& labels)
    {
        if (_logger->GetLogLevel() <= _level)
        {
            SetKeyValue(Keys::label, FormatLabelsForLogging(labels));
        }
    }

    void SetKeyValue(const Core::ServiceDescriptor& descriptor)
    {
        if (_logger->GetLogLevel() <= _level)
        {
            for (const auto& pair :  descriptor.to_keyValues())
            {
                SetKeyValue(pair.first, pair.second);
            }
        }
    }
    template <typename Key, typename Value>
    void SetKeyValue(Key&& key, Value&& value)
    {
        if (_logger->GetLogLevel() <= _level)
        {
            _keyValues.emplace_back(std::forward<Key>(key), std::forward<Value>(value));
        }
    }

    auto GetLevel() const -> Level
    {
        return _level;
    }

    auto GetKeyValues() const -> const std::vector<std::pair<std::string, std::string>>&
    {
        return _keyValues;
    }

    auto GetMsgString() const -> const std::string&
    {
        return _msg;
    }

    void Dispatch()
    {
        if (_logger->GetLogLevel() <= _level)
        {
            _logger->ProcessLoggerMessage(*this);
        }
    }

private:
    ILoggerInternal* _logger;
    Level _level;
    std::string _msg;
    std::vector<std::pair<std::string, std::string>> _keyValues;
};


} // namespace Logging
} // namespace Services
} // namespace SilKit
