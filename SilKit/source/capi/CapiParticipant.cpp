// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "ParticipantConfiguration.hpp"

#include "silkit/capi/SilKit.h"
#include "silkit/SilKit.hpp"
#include "silkit/services/logging/ILogger.hpp"
#include "silkit/services/orchestration/all.hpp"
#include "silkit/services/orchestration/string_utils.hpp"
#include "IParticipantInternal.hpp"

#include "CapiImpl.hpp"
#include "TypeConversion.hpp"

#include <memory>
#include <string>
#include <iostream>
#include <algorithm>
#include <map>
#include <mutex>
#include <cstring>

extern "C"
{
SilKit_ReturnCode SilKit_Participant_Create(SilKit_Participant** outParticipant, const char* cParticipantConfigurationString,
                                    const char* cParticipantName, const char* cRegistryUri,
                                    SilKit_Bool /*unused isSynchronized*/)
{
    ASSERT_VALID_OUT_PARAMETER(outParticipant);
    ASSERT_VALID_POINTER_PARAMETER(cParticipantConfigurationString);
    ASSERT_VALID_POINTER_PARAMETER(cParticipantName);
    ASSERT_VALID_POINTER_PARAMETER(cRegistryUri);
    CAPI_ENTER
    {
        auto config = SilKit::Config::ParticipantConfigurationFromString(cParticipantConfigurationString);

        auto participant = SilKit::CreateParticipant(config, cParticipantName, cRegistryUri).release();

        if (participant == nullptr)
        {
            SilKit_error_string =
                "Creating Simulation Participant failed due to unknown error and returned null pointer.";
            return SilKit_ReturnCode_UNSPECIFIEDERROR;
        }

        auto* logger = participant->GetLogger();
        if (logger)
        {
            logger->Info("Creating participant '{}' in domain {}", cParticipantName, cRegistryUri);
        }

        *outParticipant = reinterpret_cast<SilKit_Participant*>(participant);
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_Participant_Destroy(SilKit_Participant* participant)
{
    ASSERT_VALID_POINTER_PARAMETER(participant);
    CAPI_ENTER
    {
        if (participant == nullptr)
        {
            SilKit_error_string = "A null pointer argument was passed to the function.";
            return SilKit_ReturnCode_BADPARAMETER;
        }

        auto cppParticipant = reinterpret_cast<SilKit::IParticipant*>(participant);
        delete cppParticipant;
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_Participant_GetLogger(SilKit_Logger** outLogger, SilKit_Participant* participant)
{
    ASSERT_VALID_OUT_PARAMETER(outLogger);
    ASSERT_VALID_POINTER_PARAMETER(participant);
    CAPI_ENTER
    {
        auto cppParticipant = reinterpret_cast<SilKit::IParticipant*>(participant);
        auto logger = cppParticipant->GetLogger();
        *outLogger = reinterpret_cast<SilKit_Logger*>(logger);
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

} //extern "C"
