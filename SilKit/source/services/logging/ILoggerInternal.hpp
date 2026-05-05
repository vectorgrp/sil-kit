// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <atomic>

#include "silkit/services/logging/ILogger.hpp"

#include "StructuredLoggingKeys.hpp"
#include "SilKitFmtFormatters.hpp"
#include "fmt/format.h"
#include "traits/SilKitLoggingTraits.hpp"

namespace SilKit {
namespace Services {
namespace Logging {


class LoggerMessage;

struct ILoggerInternal
{
    virtual void ProcessLoggerMessage(const LoggerMessage& msg) = 0;
    virtual void LogReceivedMsg(const LogMsg& msg) = 0;

    virtual ~ILoggerInternal() = default;

    virtual ILogger* AsILogger() = 0;

    virtual void Log(Level level, Topic topic, const std::string& msg) = 0;

    //! \brief Get the lowest configured log level of all sinks
    virtual Level GetLogLevel() const = 0;

    virtual LoggerMessage MakeMessage(Level level, Topic topic) = 0;
};


} // namespace Logging
} // namespace Services
} // namespace SilKit
