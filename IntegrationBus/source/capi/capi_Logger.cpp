/* Copyright (c) Vector Informatik GmbH. All rights reserved. */

//#define CIntegrationBusAPI_EXPORT
#include "ib/capi/IntegrationBus.h"
#include "ib/IntegrationBus.hpp"
#include "ib/mw/logging/ILogger.hpp"

#include "capiImpl.h"

#include <string>

extern "C" {

#pragma region Logger

CIntegrationBusAPI ib_ReturnCode ib_Logger_Log(ib_Logger* self, ib_LoggingLevel level, const char* message)
{
    ASSERT_VALID_POINTER_PARAMETER(self);
    ASSERT_VALID_POINTER_PARAMETER(message);
    CAPI_ENTER
    {
        auto logger = reinterpret_cast<ib::mw::logging::ILogger*>(self);
        std::string strMessage(message);
        auto enumLevel = static_cast<ib::mw::logging::Level>(level);
        logger->Log(enumLevel, strMessage);
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

#pragma endregion Logger

}
