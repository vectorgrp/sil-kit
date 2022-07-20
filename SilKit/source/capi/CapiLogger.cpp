// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "silkit/capi/SilKit.h"
#include "silkit/SilKit.hpp"
#include "silkit/services/logging/ILogger.hpp"

#include "CapiImpl.hpp"

#include <string>

extern "C" {

SilKit_ReturnCode SilKit_Logger_Log(SilKit_Logger* self, SilKit_LoggingLevel level, const char* message)
{
    ASSERT_VALID_POINTER_PARAMETER(self);
    ASSERT_VALID_POINTER_PARAMETER(message);
    CAPI_ENTER
    {
        auto logger = reinterpret_cast<SilKit::Services::Logging::ILogger*>(self);
        auto enumLevel = static_cast<SilKit::Services::Logging::Level>(level);
        std::string useString{message}; //ensure we do not trigger the FMT template overload for const char*
        logger->Log(enumLevel, useString);
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}


SilKit_ReturnCode SilKit_Logger_GetLogLevel(SilKit_Logger* self, SilKit_LoggingLevel* outLevel) {
    ASSERT_VALID_POINTER_PARAMETER(self);
    ASSERT_VALID_POINTER_PARAMETER(outLevel);
    CAPI_ENTER
    {
        auto logger = reinterpret_cast<SilKit::Services::Logging::ILogger*>(self);
        auto enumLevel = static_cast<SilKit_LoggingLevel>(logger->GetLogLevel());
        *outLevel = enumLevel;
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

}
