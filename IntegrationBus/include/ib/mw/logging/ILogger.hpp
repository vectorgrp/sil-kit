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
#    include "VibFmtFormatters.hpp"
#endif //HAVE_FMTLIB

namespace ib {
namespace mw {
namespace logging {

class ILogger
{
public:
    using LogMsgHandlerT = std::function<void(LogMsg)>;

public:
    virtual ~ILogger() = default;
    /*! \brief Log a message with a specified level
    *
    * \param level The log level for the message
    * \param msg The message which shall be logged.
    */
    virtual void Log(Level level, const std::string& msg) = 0;

    //! \brief Log a message with log level trace.
    virtual void Trace(const std::string& msg) = 0;

    //! \brief Log a message with log level debug.
    virtual void Debug(const std::string& msg) = 0;

    //! \brief Log a message with log level info.
    virtual void Info(const std::string& msg) = 0;

    //! \brief Log a message with log level warn.
    virtual void Warn(const std::string& msg) = 0;

    //! \brief Log a message with log level error.
    virtual void Error(const std::string& msg) = 0;

    //! \brief Log a message with log level critical.
    virtual void Critical(const std::string& msg) = 0;

#ifdef HAVE_FMTLIB
    template<typename... Args>
    void Log(Level level, const char* fmt, const Args&... args)
    {
        if(ShouldLog(level))
        {
            std::string msg = fmt::format(fmt, args...);
            Log(level, msg);
        }

    }
    template<typename... Args>
    void Trace(const char* fmt, const Args&... args)
    {
        Log(Level::Trace, fmt, args...);
    }
    template<typename... Args>
    void Debug(const char* fmt, const Args&... args)
    {
        Log(Level::Debug, fmt, args...);
    }
    template<typename... Args>
    void Info(const char* fmt, const Args&... args)
    {
        Log(Level::Info, fmt, args...);
    }
    template<typename... Args>
    void Warn(const char* fmt, const Args&... args)
    {
        Log(Level::Warn, fmt, args...);
    }
    template<typename... Args>
    void Error(const char* fmt, const Args&... args)
    {
        Log(Level::Error, fmt, args...);
    }
    template<typename... Args>
    void Critical(const char* fmt, const Args&... args)
    {
        Log(Level::Critical, fmt, args...);
    }
#endif //HAVE_FMTLIB

protected:
    virtual bool ShouldLog(Level) const = 0;
};

} // logging
} // mw
} // namespace ib
