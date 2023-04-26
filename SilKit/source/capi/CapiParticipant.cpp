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

#include "ParticipantConfiguration.hpp"
#include "ParticipantConfigurationFromXImpl.hpp"
#include "CreateParticipantImpl.hpp"

#include "silkit/capi/SilKit.h"
#include "silkit/SilKit.hpp"
#include "silkit/services/logging/ILogger.hpp"
#include "silkit/services/orchestration/all.hpp"

#include "CapiImpl.hpp"
#include "TypeConversion.hpp"

#include <memory>
#include <map>
#include <mutex>


SilKit_ReturnCode SilKitCALL SilKit_Participant_Create(SilKit_Participant** outParticipant,
                                            SilKit_ParticipantConfiguration *participantConfiguration,
                                            const char* participantName, const char* registryUri)
try
{
    ASSERT_VALID_OUT_PARAMETER(outParticipant);
    ASSERT_VALID_POINTER_PARAMETER(participantConfiguration);
    ASSERT_VALID_POINTER_PARAMETER(participantName);
    ASSERT_VALID_POINTER_PARAMETER(registryUri);

    auto* cppParticipantConfiguration =
        reinterpret_cast<SilKit::Config::ParticipantConfiguration*>(participantConfiguration);

    // since we do _not_ want to consume the pointer itself, we must make a copy and store it in a shared_ptr
    auto cppSharedParticipantConfiguration =
        std::make_shared<SilKit::Config::ParticipantConfiguration>(*cppParticipantConfiguration);

    auto participant =
        SilKit::CreateParticipantImpl(std::move(cppSharedParticipantConfiguration), participantName, registryUri)
            .release();

    if (participant == nullptr)
    {
        SilKit_error_string =
            "Creating simulation participant failed due to an unknown error and returned null pointer.";
        return SilKit_ReturnCode_UNSPECIFIEDERROR;
    }

    *outParticipant = reinterpret_cast<SilKit_Participant*>(participant);
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_Participant_Destroy(SilKit_Participant* participant)
try
{
    ASSERT_VALID_POINTER_PARAMETER(participant);

    if (participant == nullptr)
    {
        SilKit_error_string = "A null pointer argument was passed to the function.";
        return SilKit_ReturnCode_BADPARAMETER;
    }

    auto cppParticipant = reinterpret_cast<SilKit::IParticipant*>(participant);
    delete cppParticipant;
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_Participant_GetLogger(SilKit_Logger** outLogger, SilKit_Participant* participant)
try
{
    ASSERT_VALID_OUT_PARAMETER(outLogger);
    ASSERT_VALID_POINTER_PARAMETER(participant);

    auto cppParticipant = reinterpret_cast<SilKit::IParticipant*>(participant);
    auto logger = cppParticipant->GetLogger();
    *outLogger = reinterpret_cast<SilKit_Logger*>(logger);
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_ParticipantConfiguration_FromString(
    SilKit_ParticipantConfiguration** outParticipantConfiguration,
    const char* participantConfigurationString)
try
{
    ASSERT_VALID_OUT_PARAMETER(outParticipantConfiguration);
    ASSERT_VALID_POINTER_PARAMETER(participantConfigurationString);

    // create the configuration using the C++ API function
    auto cppSharedParticipantConfiguration = std::dynamic_pointer_cast<SilKit::Config::ParticipantConfiguration>(
        SilKit::Config::ParticipantConfigurationFromStringImpl(participantConfigurationString));
    if (cppSharedParticipantConfiguration == nullptr)
    {
        SilKit_error_string = "Participant configuration could not be created.";
        return SilKit_ReturnCode_UNSPECIFIEDERROR;
    }

    // since it is not possible to release the "raw" pointer from a shared_ptr, we copy it into our own raw pointer
    auto* cppParticipantConfiguration =
        new SilKit::Config::ParticipantConfiguration(*cppSharedParticipantConfiguration);

    *outParticipantConfiguration = reinterpret_cast<SilKit_ParticipantConfiguration*>(cppParticipantConfiguration);
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_ParticipantConfiguration_Destroy(
    SilKit_ParticipantConfiguration* participantConfiguration)
try
{
    ASSERT_VALID_POINTER_PARAMETER(participantConfiguration);

    if (participantConfiguration == nullptr)
    {
        SilKit_error_string = "A null pointer argument was passed to the function.";
        return SilKit_ReturnCode_BADPARAMETER;
    }

    auto* cppParticipantConfiguration =
        reinterpret_cast<SilKit::Config::ParticipantConfiguration*>(participantConfiguration);
    delete cppParticipantConfiguration;

    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS
