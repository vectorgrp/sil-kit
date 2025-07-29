// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "silkit/capi/SilKit.h"
#include "silkit/SilKit.hpp"
#include "silkit/services/logging/ILogger.hpp"

#include "CapiImpl.hpp"

#include <string>


SilKit_ReturnCode SilKitCALL SilKit_Logger_Log(SilKit_Logger* self, SilKit_LoggingLevel level, const char* message)
try
{
    ASSERT_VALID_POINTER_PARAMETER(self);
    ASSERT_VALID_POINTER_PARAMETER(message);

    auto logger = reinterpret_cast<SilKit::Services::Logging::ILogger*>(self);
    auto enumLevel = static_cast<SilKit::Services::Logging::Level>(level);
    std::string useString{message}; //ensure we do not trigger the FMT template overload for const char*
    logger->Log(enumLevel, useString);
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_Logger_GetLogLevel(SilKit_Logger* self, SilKit_LoggingLevel* outLevel)
try
{
    ASSERT_VALID_POINTER_PARAMETER(self);
    ASSERT_VALID_POINTER_PARAMETER(outLevel);

    auto logger = reinterpret_cast<SilKit::Services::Logging::ILogger*>(self);
    auto enumLevel = static_cast<SilKit_LoggingLevel>(logger->GetLogLevel());
    *outLevel = enumLevel;
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS
