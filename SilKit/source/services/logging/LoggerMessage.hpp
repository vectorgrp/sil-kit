// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

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

    LoggerMessage(ILoggerInternal* logger, Level level, Topic topic)
        : _logger(logger)
        , _level(level)
        , _topic(topic)
    {
    }

    LoggerMessage(ILoggerInternal* logger, const LogMsg& msg)
        : _logger(logger)
        , _level(msg.level)
        , _topic(msg.topic)
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

    template <typename... Args>
    LoggerMessage& SetMessage(fmt::format_string<Args...> fmt, Args&&... args)
    {
        if (_logger->GetLogLevel() <= _level)
        {
            _msg = fmt::format(fmt, std::forward<Args>(args)...);
        }
        return *this;
    }

    auto SetTopic(Topic topic) -> LoggerMessage&
    {
        _topic = topic;
        return *this;
    }

    template <typename Key, typename... Args>
    LoggerMessage& AddKeyValue(Key&& key, Args&&... args)
    {
        FormatKeyValue(key, "{}", std::forward<Args>(args)...);
        return *this;
    }

    LoggerMessage& AddKeyValue(const Core::ServiceDescriptor& descriptor)
    {
        if (_logger->GetLogLevel() <= _level)
        {
            for (const auto& pair : descriptor.to_keyValues())
            {
                SetKeyValue(pair.first, pair.second);
            }
        }
        return *this;
    }

    LoggerMessage& AddKeyValue(const std::vector<Services::MatchingLabel>& labels)
    {
        if (_logger->GetLogLevel() <= _level)
        {
            SetKeyValue(Keys::label, FormatLabelsForLogging(labels));
        }
        return *this;
    }

    template <typename Key, typename... Args>
    LoggerMessage& SetKeyValue(Key&& key, Args&&... args)
    {
        if (_logger->GetLogLevel() <= _level)
        {
            for (auto& kv : _keyValues)
            {
                if (kv.first == key)
                {
                    auto&& formattedValue = fmt::format("{}", std::forward<Args>(args)...);
                    kv.second = formattedValue;
                    return *this;
                }
            }
            return AddKeyValue(key, std::forward<Args>(args)...);
        }
        return *this;
    }

    auto SetLevel(const Level level) -> LoggerMessage&
    {
        _level = level;
        return *this;
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
            for (const auto& pair : descriptor.to_keyValues())
            {
                SetKeyValue(pair.first, pair.second);
            }
        }
    }

    auto GetLevel() const -> Level
    {
        return _level;
    }

    auto GetTopic() const -> Topic
    {
        return _topic;
    }

    auto GetKeyValues() const -> const std::vector<std::pair<std::string, std::string>>&
    {
        return _keyValues;
    }

    auto GetMsgString() const -> const std::string&
    {
        return _msg;
    }

    LoggerMessage Dispatch()
    {
        if (_logger->GetLogLevel() <= _level)
        {
            _logger->ProcessLoggerMessage(*this);
        }
        return std::move(*this);
    }

private:
    ILoggerInternal* _logger;
    Level _level;
    Topic _topic{Topic::None};
    std::string _msg;
    std::vector<std::pair<std::string, std::string>> _keyValues;
};


} // namespace Logging
} // namespace Services
} // namespace SilKit
