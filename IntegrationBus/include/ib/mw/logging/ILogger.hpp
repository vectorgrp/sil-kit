// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <memory>
#include <string>
#include <functional>

#include "LoggingDatatypes.hpp"

#ifdef HAVE_FMTLIB
#    ifndef FMT_USE_WINDOWS_H
#        define FMT_USE_WINDOWS_H 0
#    endif
#    include "fmt/format.h"
#    include "fmt/ostream.h"
#endif //HAVE_FMTLIB

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
    virtual void RegisterRemoteLogging(const LogMsgHandlerT& handler) = 0;

    /*! \brief Log a received remote message.
    */
    virtual void LogReceivedMsg(const LogMsg& msg) = 0;

#ifdef HAVE_FMTLIB
    template<typename... Args>
    void Log(Level level, const char* fmt, const Args&... args)
    {
        std::string msg = fmt::format(fmt, args...);
        Log(level, msg);

    }
    template<typename... Args>
    void Debug(const char* fmt, const Args&... args)
    {
        Log(Level::debug, fmt, args...);
    }
    template<typename... Args>
    void Info(const char* fmt, const Args&... args)
    {
        Log(Level::info, fmt, args...);
    }
    template<typename... Args>
    void Warn(const char* fmt, const Args&... args)
    {
        Log(Level::warn, fmt, args...);
    }
    template<typename... Args>
    void Error(const char* fmt, const Args&... args)
    {
        Log(Level::error, fmt, args...);
    }
    template<typename... Args>
    void Critical(const char* fmt, const Args&... args)
    {
        Log(Level::critical, fmt, args...);
    }
#endif //HAVE_FMTLIB

};

} // logging
} // mw
} // namespace ib
