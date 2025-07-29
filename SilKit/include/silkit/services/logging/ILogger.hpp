// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

#include "LoggingDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace Logging {

class ILogger
{
public:
    virtual ~ILogger() = default;

    /*! \brief Log a message with a specified level
    *
    * \param level The log level for the message
    * \param msg The message which shall be logged (UTF-8).
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

    //! \brief Get the lowest configured log level of all sinks
    virtual Level GetLogLevel() const = 0;
};

} // namespace Logging
} // namespace Services
} // namespace SilKit
