// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "ib/capi/IntegrationBus.h"
#include "ib/IntegrationBus.hpp"
#include "ib/mw/logging/ILogger.hpp"

#include "CapiImpl.hpp"

#include <string>

extern "C" {

ib_ReturnCode ib_Logger_Log(ib_Logger* self, ib_LoggingLevel level, const char* message)
{
    ASSERT_VALID_POINTER_PARAMETER(self);
    ASSERT_VALID_POINTER_PARAMETER(message);
    CAPI_ENTER
    {
        auto logger = reinterpret_cast<ib::mw::logging::ILogger*>(self);
        auto enumLevel = static_cast<ib::mw::logging::Level>(level);
        std::string useString{message}; //ensure we do not trigger the FMT template overload for const char*
        logger->Log(enumLevel, useString);
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

}
