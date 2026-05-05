// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include <functional>

#include "silkit/services/logging/LoggingDatatypes.hpp"

#include "ILoggerInternal.hpp"
#include "LoggerMessage.hpp"
#include "Configuration.hpp"

namespace spdlog {
class logger;
namespace sinks {
class sink;
} // namespace sinks
} // namespace spdlog

namespace SilKit {
namespace Services {
namespace Logging {

struct LogMsg;

class RemoteLogger
{
public:
    using LogMsgHandler = std::function<void(LogMsg)>;

public:
    RemoteLogger(Level level, std::string participantName)
        : _level(level)
        , _participantName(participantName)
    {
    }

    void Log(log_clock::time_point logTime, Level msgLevel, std::string msg)
    {
        if (nullptr != _remoteSink)
        {
            if (_level <= msgLevel)
            {
                LogMsg logmsg{};
                logmsg.loggerName = _participantName;
                logmsg.level = msgLevel;
                logmsg.time = logTime;
                logmsg.payload = msg;

                // dispatch msg
                _remoteSink(logmsg);
            }
        }
    }

    void Log(log_clock::time_point logTime, Level msgLevel, Topic topic, std::string msg)
    {
        if (nullptr != _remoteSink)
        {
            if (_level <= msgLevel)
            {
                LogMsg logmsg{};
                logmsg.loggerName = _participantName;
                logmsg.level = msgLevel;
                logmsg.time = logTime;
                logmsg.payload = msg;
                logmsg.topic = topic;

                // dispatch msg
                _remoteSink(logmsg);
            }
        }
    }

    void Log(log_clock::time_point logTime, const LoggerMessage& msg)
    {
        if (nullptr != _remoteSink)
        {
            if (_level <= msg.GetLevel())
            {
                LogMsg logmsg{_participantName, msg.GetLevel(), msg.GetTopic(), logTime, {}, msg.GetMsgString(), msg.GetKeyValues()};
                // dispatch msg
                _remoteSink(logmsg);
            }
        }
    }

    void Log(const LogMsg& msg)
    {
        if (nullptr != _remoteSink)
        {
            if (_level <= msg.level)
            {
                _remoteSink(msg);
            }
        }
    }

    void RegisterRemoteLogging(const LogMsgHandler& handler)
    {
        _remoteSink = handler;
    }


    void DisableRemoteLogging()
    {
        if (nullptr != _remoteSink)
        {
            _remoteSink = nullptr;
        }
    }
    Level level()
    {
        return _level;
    }

private:
    Level _level;
    std::function<void(const LogMsg&)> _remoteSink;
    std::string _participantName;
};


class Logger
    : public ILoggerInternal
    , ILogger
{
public:
    using LogMsgHandler = std::function<void(LogMsg)>;

public:
    // ----------------------------------------
    // Constructors and Destructor
    Logger(const std::string& participantName, Config::Logging config);


    ILogger* AsILogger() override 
    {
        return static_cast<ILogger*>(this);
    }
    // ----------------------------------------
    // Public interface methods
    //
    // ILogger

    void Trace(const std::string& msg) override 
    {
        Log(Level::Trace, msg);
    }
    void Debug(const std::string& msg) override
    {
        Log(Level::Debug, msg);
    }
    void Info(const std::string& msg) override
    {
        Log(Level::Info, msg);
    }
    void Warn(const std::string& msg) override
    {
        Log(Level::Warn, msg);
    }
    void Error(const std::string& msg) override
    {
        Log(Level::Error, msg);
    }
    void Critical(const std::string& msg) override
    {
        Log(Level::Critical, msg);
    }

    void Log(Level lvl, const std::string& msg) override 
    {
        Log(lvl, Topic::User, msg);
    }

    void Log(Level level, Topic topic, const std::string& msg) override;



    void RegisterRemoteLogging(const LogMsgHandler& handler);
    void DisableRemoteLogging();
    //void LogReceivedMsg(const LogMsg& msg);


    auto GetLogLevel() const -> Level override;

    LoggerMessage MakeMessage(Level level, Topic topic) override
    {
        return LoggerMessage(this, level, topic);
    }


    // ILoggerInternal
    void ProcessLoggerMessage(const LoggerMessage& msg) override;

    void LogReceivedMsg(const LogMsg& msg) override;

private:
    // Private members
    Config::Logging _config;
    std::map<std::shared_ptr<spdlog::logger>, Config::Sink> _spdlogLogger;
    std::map<std::shared_ptr<RemoteLogger>, Config::Sink> _remoteLogger;

    // Private methods
    void DispatchToSinks(log_clock::time_point now, Level level, Topic topic,
                         const std::function<std::string(Config::Sink::Format)>& formatter,
                         const std::function<void(const std::shared_ptr<RemoteLogger>&, log_clock::time_point)>& remoteDispatcher = nullptr);

    bool IsTopicEnabled(const std::vector<Services::Logging::Topic>& enabledTopics,
                        const std::vector<Services::Logging::Topic>& disabledTopics,
                        const Topic msgTopic) const
    {
        // Explicitly disabled topics are always filtered out
        if (std::find(disabledTopics.begin(), disabledTopics.end(), msgTopic) != disabledTopics.end())
        {
            return false;
        }

        // If an allow-list is specified, only those topics pass
        if (!enabledTopics.empty())
        {
            return std::find(enabledTopics.begin(), enabledTopics.end(), msgTopic) != enabledTopics.end();
        }

        // No filter configured - allow everything
        return true;
    }

    bool IsTopicDisabled(const std::vector<Services::Logging::Topic>& enabledTopics,
                        const std::vector<Services::Logging::Topic>& disabledTopics, const Topic msgTopic) const
    {
        // Explicitly disabled topics are always filtered out
        if (std::find(disabledTopics.begin(), disabledTopics.end(), msgTopic) != disabledTopics.end())
        {
            return true;
        }

        // If an allow-list is specified, only those topics pass
        if (!enabledTopics.empty())
        {
            return !(std::find(enabledTopics.begin(), enabledTopics.end(), msgTopic) != enabledTopics.end());
        }

        // No filter configured - allow everything
        return false;
    }

    bool IsLevelMatching( const Level& sinkLevel, const Level& msgLevel) const
    {
        return sinkLevel <= msgLevel;
    }
};

} // namespace Logging
} // namespace Services
} // namespace SilKit
