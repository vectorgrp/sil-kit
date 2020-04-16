// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <memory>

#include "ib/mw/logging/ILogger.hpp"
#include "ib/mw/logging/LoggingDatatypes.hpp"
#include "ib/cfg/Config.hpp"

namespace spdlog {
class logger;
namespace sinks {
class sink;
} // sinks
} // spdlog

namespace ib {
namespace mw {
namespace logging {

class Logger : public ILogger
{
public:
    // ----------------------------------------
    // Constructors and Destructor
    Logger(const std::string& participantName, cfg::Logger config);

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

    void RegisterRemoteLogging(const LogMsgHandlerT& handler);
    void DisableRemoteLogging();
    void LogReceivedMsg(const LogMsg& msg);

protected:
    bool ShouldLog(Level level) const override;
private:
    // ----------------------------------------
    // private members
    cfg::Logger _config;

    std::shared_ptr<spdlog::logger> _logger;
    std::shared_ptr<spdlog::sinks::sink> _ibRemoteSink;
};

} // namespace logging
} // namespace mw
} // namespace ib
