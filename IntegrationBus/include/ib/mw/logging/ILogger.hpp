// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <memory>
#include <string>
#include <functional>

#include "LoggingDatatypes.hpp"

namespace ib {
namespace mw {
namespace logging {

class ILogger
{
public:
    using LogMsgHandlerT = std::function<void(LogMsg)>;

public:
    /*! \brief Log a message with a specified level
    *
    * \param level The log level for the message
    * \param msg The message which shall be logged.
    */
    virtual void Log(Level level, const std::string& msg) = 0;

    //! \brief Log a message with the trace level.
    virtual void Trace(const std::string& msg) = 0;

    //! \brief Log a message with the debug level.
    virtual void Debug(const std::string& msg) = 0;

    //! \brief Log a message with the info level.
    virtual void Info(const std::string& msg) = 0;

    //! \brief Log a message with the warn level.
    virtual void Warn(const std::string& msg) = 0;

    //! \brief Log a message with the error level.
    virtual void Error(const std::string& msg) = 0;

    //! \brief Log a message with the critical level.
    virtual void Critical(const std::string& msg) = 0;

    /*! \brief Register a callback for remote logging.
    */
    virtual void RegisterLogMsgHandler(LogMsgHandlerT handler) = 0;

    /*! \brief Log a received remote message.
    */
    virtual void LogReceivedMsg(const LogMsg& msg) = 0;
};

} // logging
} // mw
} // namespace ib
