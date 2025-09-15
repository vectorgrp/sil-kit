// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "ParticipantConfiguration.hpp"
#include "ParticipantConfigurationFromXImpl.hpp"
#include "CreateParticipantImpl.hpp"
#include "YamlParser.hpp"

#include "silkit/capi/SilKit.h"
#include "silkit/SilKit.hpp"
#include "silkit/services/logging/ILogger.hpp"
#include "silkit/services/orchestration/all.hpp"
#include "silkit/participant/parameters.hpp"

#include "CapiImpl.hpp"
#include "TypeConversion.hpp"

#include <memory>
#include <map>
#include <mutex>
#include <fstream>

SilKit_ReturnCode SilKitCALL SilKit_Participant_Create(SilKit_Participant** outParticipant,
                                                       SilKit_ParticipantConfiguration* participantConfiguration,
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

SilKit_ReturnCode SilKitCALL SilKit_Participant_GetParameter(void* outParameterValue, size_t* inOutParameterValueSize,
                                                             SilKit_Parameter parameter,
                                                             SilKit_Participant* participant)
try
{
    ASSERT_VALID_OUT_PARAMETER(inOutParameterValueSize);
    ASSERT_VALID_POINTER_PARAMETER(participant);

    auto cppParticipant = reinterpret_cast<SilKit::IParticipant*>(participant);
    auto cppParameter = static_cast<SilKit::Parameter>(parameter);
    auto parameterValue = cppParticipant->GetParameter(cppParameter);

    // outParameterValue == nullptr indicates a size-check only, otherwise copy
    if (outParameterValue != nullptr)
    {
        size_t sizeToCopy;
        if (*inOutParameterValueSize > parameterValue.size())
        {
            // Don't copy more than we actually have
            sizeToCopy = parameterValue.size();
        }
        else
        {
            // Don't copy more than the given size
            sizeToCopy = *inOutParameterValueSize;
        }
        parameterValue.copy(static_cast<char*>(outParameterValue), sizeToCopy);
    }
    *inOutParameterValueSize = parameterValue.size();
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_ParticipantConfiguration_FromString(
    SilKit_ParticipantConfiguration** outParticipantConfiguration, const char* participantConfigurationString)
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


SilKit_ReturnCode SilKitCALL SilKit_ParticipantConfiguration_FromFile(
    SilKit_ParticipantConfiguration** outParticipantConfiguration, const char* participantConfigurationPath)
try
{
    ASSERT_VALID_OUT_PARAMETER(outParticipantConfiguration);
    ASSERT_VALID_POINTER_PARAMETER(participantConfigurationPath);

    // create the configuration using the C++ API function
    auto cppSharedParticipantConfiguration = std::dynamic_pointer_cast<SilKit::Config::ParticipantConfiguration>(
        SilKit::Config::ParticipantConfigurationFromFileImpl(participantConfigurationPath));
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


SilKit_ReturnCode SilKitCALL SilKit_ParticipantConfiguration_ToJson(
    const SilKit_ParticipantConfiguration* participantConfiguration, char** outputString, size_t* requiredSize)
try
{
    ASSERT_VALID_OUT_PARAMETER(participantConfiguration);
    ASSERT_VALID_POINTER_PARAMETER(requiredSize);
    //outputString may be NULL

    auto origOutputSize = *requiredSize;
    auto* cppParticipantConfiguration =
        reinterpret_cast<const SilKit::Config::ParticipantConfiguration*>(participantConfiguration);

    auto&& jsonString = SilKit::Config::SerializeAsJson(*cppParticipantConfiguration);
    *requiredSize = jsonString.size();

    if(outputString != nullptr && origOutputSize > 0)
    {
        const auto sizeToWrite = std::min(origOutputSize, jsonString.size());
        std::copy(std::cbegin(jsonString), std::cbegin(jsonString) + sizeToWrite, *outputString);
    }

    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS

SilKit_ReturnCode SilKitCALL
SilKit_ParticipantConfiguration_Destroy(SilKit_ParticipantConfiguration* participantConfiguration)
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
