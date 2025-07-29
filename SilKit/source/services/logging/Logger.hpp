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

    void Log(log_clock::time_point logTime, const LoggerMessage& msg)
    {
        if (nullptr != _remoteSink)
        {
            if (_level <= msg.GetLevel())
            {
                LogMsg logmsg{_participantName, msg.GetLevel(), logTime, {}, msg.GetMsgString(), msg.GetKeyValues()};
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


class Logger : public ILoggerInternal
{
public:
    using LogMsgHandler = std::function<void(LogMsg)>;

public:
    // ----------------------------------------
    // Constructors and Destructor
    Logger(const std::string& participantName, Config::Logging config);

    // ----------------------------------------
    // Public interface methods
    //
    // ILogger
    void Log(Level level, const std::string& msg) override;

    void Trace(const std::string& msg) override;

    void Debug(const std::string& msg) override;

    void Info(const std::string& msg) override;

    void Warn(const std::string& msg) override;

    void Error(const std::string& msg) override;

    void Critical(const std::string& msg) override;

    void RegisterRemoteLogging(const LogMsgHandler& handler);
    void DisableRemoteLogging();
    //void LogReceivedMsg(const LogMsg& msg);

    auto GetLogLevel() const -> Level override;

    // ILoggerInternal
    void ProcessLoggerMessage(const LoggerMessage& msg) override;

    void LogReceivedMsg(const LogMsg& msg) override;

private:
    // ----------------------------------------
    // Private members
    Config::Logging _config;

    std::shared_ptr<spdlog::logger> _loggerJson;
    std::shared_ptr<spdlog::logger> _loggerSimple;
    std::shared_ptr<RemoteLogger> _loggerRemote;
};

} // namespace Logging
} // namespace Services
} // namespace SilKit
