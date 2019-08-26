// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <memory>

#include "ib/mw/logging/ILogger.hpp"
#include "ib/mw/logging/LoggingDatatypes.hpp"

#include "SpdlogTypeConversion.hpp"

namespace ib {
namespace mw {
namespace logging {

class Logger : public ILogger
{
public:
    // ----------------------------------------
    // Constructors and Destructor
    Logger(const std::string& participantName);

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

    void RegisterLogMsgHandler(LogMsgHandlerT handler) override;

    void LogReceivedMsg(const LogMsg& msg) override;

public:
    // ----------------------------------------
    // Public methods
    void Distribute(const LogMsg& msg);
    void Distribute(LogMsg&& msg);

private:
    // ----------------------------------------
    // private members
    std::shared_ptr<spdlog::logger> _logger;
    std::shared_ptr<spdlog::sinks::sink> _ibRemoteSink;

    LogMsgHandlerT _logMsgHandler;
};

inline bool operator==(const SourceLoc& lhs, const SourceLoc& rhs)
{
    return lhs.filename == rhs.filename
        && lhs.line == rhs.line
        && lhs.funcname == rhs.funcname;
}

inline bool operator==(const LogMsg& lhs, const LogMsg& rhs)
{
    return lhs.source == rhs.source
        && lhs.time == rhs.time
        && lhs.logger_name == rhs.logger_name

        && lhs.level == rhs.level
        && lhs.payload == rhs.payload;
}

} // namespace logging
} // namespace mw
} // namespace ib
