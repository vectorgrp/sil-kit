/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

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

    //! \brief Get the lowest configured log level of all sinks
    virtual Level GetLogLevel() const = 0;
};

} // namespace Logging
} // namespace Services
} // namespace SilKit
