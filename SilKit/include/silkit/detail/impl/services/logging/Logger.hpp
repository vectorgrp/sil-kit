// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/capi/Logger.h"

#include "silkit/services/logging/ILogger.hpp"


namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Impl {
namespace Services {
namespace Logging {

class Logger : public SilKit::Services::Logging::ILogger
{
public:
    inline Logger(SilKit_Participant* participant);
    inline Logger(SilKit_Vendor_Vector_SilKitRegistry* silKitRegistry);

    inline ~Logger() override = default;

    inline void Log(SilKit::Services::Logging::Level level, const std::string& msg) override;

    inline void Trace(const std::string& msg) override;

    inline void Debug(const std::string& msg) override;

    inline void Info(const std::string& msg) override;

    inline void Warn(const std::string& msg) override;

    inline void Error(const std::string& msg) override;

    inline void Critical(const std::string& msg) override;

    inline auto GetLogLevel() const -> SilKit::Services::Logging::Level override;

private:
    SilKit_Logger* _logger{nullptr};
};

} // namespace Logging
} // namespace Services
} // namespace Impl
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit


// ================================================================================
//  Inline Implementations
// ================================================================================

#include "silkit/detail/impl/ThrowOnError.hpp"

namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Impl {
namespace Services {
namespace Logging {

Logger::Logger(SilKit_Participant* participant)
{
    const auto returnCode = SilKit_Participant_GetLogger(&_logger, participant);
    ThrowOnError(returnCode);
}

Logger::Logger(SilKit_Vendor_Vector_SilKitRegistry* silKitRegistry)
{
    const auto returnCode = SilKit_Vendor_Vector_SilKitRegistry_GetLogger(&_logger, silKitRegistry);
    ThrowOnError(returnCode);
}

void Logger::Log(SilKit::Services::Logging::Level level, const std::string& msg)
{
    const auto loggingLevel = static_cast<SilKit_LoggingLevel>(level);

    const auto returnCode = SilKit_Logger_Log(_logger, loggingLevel, msg.c_str());
    ThrowOnError(returnCode);
}

void Logger::Trace(const std::string& msg)
{
    const auto returnCode = SilKit_Logger_Log(_logger, SilKit_LoggingLevel_Trace, msg.c_str());
    ThrowOnError(returnCode);
}

void Logger::Debug(const std::string& msg)
{
    const auto returnCode = SilKit_Logger_Log(_logger, SilKit_LoggingLevel_Debug, msg.c_str());
    ThrowOnError(returnCode);
}

void Logger::Info(const std::string& msg)
{
    const auto returnCode = SilKit_Logger_Log(_logger, SilKit_LoggingLevel_Info, msg.c_str());
    ThrowOnError(returnCode);
}

void Logger::Warn(const std::string& msg)
{
    const auto returnCode = SilKit_Logger_Log(_logger, SilKit_LoggingLevel_Warn, msg.c_str());
    ThrowOnError(returnCode);
}

void Logger::Error(const std::string& msg)
{
    const auto returnCode = SilKit_Logger_Log(_logger, SilKit_LoggingLevel_Error, msg.c_str());
    ThrowOnError(returnCode);
}

void Logger::Critical(const std::string& msg)
{
    const auto returnCode = SilKit_Logger_Log(_logger, SilKit_LoggingLevel_Critical, msg.c_str());
    ThrowOnError(returnCode);
}

auto Logger::GetLogLevel() const -> SilKit::Services::Logging::Level
{
    SilKit_LoggingLevel loggingLevel{SilKit_LoggingLevel_Info};

    const auto returnCode = SilKit_Logger_GetLogLevel(_logger, &loggingLevel);
    ThrowOnError(returnCode);

    return static_cast<SilKit::Services::Logging::Level>(loggingLevel);
}

} // namespace Logging
} // namespace Services
} // namespace Impl
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit
