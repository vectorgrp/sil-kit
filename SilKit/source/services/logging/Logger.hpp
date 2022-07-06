// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <memory>

#include "silkit/services/logging/ILogger.hpp"
#include "silkit/services/logging/LoggingDatatypes.hpp"

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

class Logger : public ILogger
{
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

    void RegisterRemoteLogging(const LogMsgHandlerT& handler);
    void DisableRemoteLogging();
    void LogReceivedMsg(const LogMsg& msg);

protected:
    bool ShouldLog(Level level) const override;
private:
    // ----------------------------------------
    // Private members
    Config::Logging _config;

    std::shared_ptr<spdlog::logger> _logger;
    std::shared_ptr<spdlog::sinks::sink> _remoteSink;
};

} // namespace Logging
} // namespace Services
} // namespace SilKit
