// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <atomic>

#include "silkit/services/logging/ILogger.hpp"

#include "StructuredLoggingKeys.hpp"
#include "SilKitFmtFormatters.hpp"
#include "fmt/format.h"

namespace SilKit {
namespace Services {
namespace Logging {


class LoggerMessage;

struct ILoggerInternal : ILogger
{
    virtual void ProcessLoggerMessage(const LoggerMessage& msg) = 0;
    virtual void LogReceivedMsg(const LogMsg& msg) = 0;
};


} // namespace Logging
} // namespace Services
} // namespace SilKit
