// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/capi/SilKitMacros.h"
#include "silkit/capi/Types.h"
#include "silkit/capi/Can.h"
#include "silkit/capi/Lin.h"
#include "silkit/capi/DataPubSub.h"
#include "silkit/capi/Rpc.h"
#include "silkit/capi/Ethernet.h"
#include "silkit/capi/Flexray.h"
#include "silkit/capi/Logger.h"
#include "silkit/capi/InterfaceIdentifiers.h"
#include "silkit/capi/Participant.h"
#include "silkit/capi/Orchestration.h"
#include "silkit/capi/Vendor.h"
#include "silkit/capi/Version.h"
#include "silkit/capi/NetworkSimulator.h"
#include "silkit/capi/EventProducer.h"

SILKIT_BEGIN_DECLS

/*! \brief Get the corresponding static error string for a given return code.
 *
 * \param outString The pointer through which the resulting human readable error string, encoded in UTF-8, will be returned.
 * \param returnCode The return code for which the string should be obtained.
 *
 * \ref SilKit_GetLastErrorString()
 *
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_ReturnCodeToString(const char** outString, SilKit_ReturnCode returnCode);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_ReturnCodeToString_t)(const char** outString,
                                                                   SilKit_ReturnCode returnCode);

/*! \brief Get a human readable error description of the last error on the current thread.
 *
 * This method is intended to get specific error messages in case of a non success return code.
 * In comparison to SilKit_ReturnCodeToString this function returns dynamic and more specific error messages.
 *
 * \return A specific string containing the last error message of the current thread.
 *
 */
const SilKitAPI char* SilKitCALL SilKit_GetLastErrorString();

typedef const char*(SilKitFPTR* SilKit_GetLastErrorString_t)();

/*! \brief Create an opaque participant configuration from the configuration text.
 *
 * To destroy the participant configuration, call \ref SilKit_ParticipantConfiguration_Destroy.
 *
 * \param outParticipantConfiguration The pointer through which the participant configuration will be returned (out parameter).
 * \param participantConfigurationString The configuration as a UTF-8 encoded string (e.g., read from a configuration file or a string constant).
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_ParticipantConfiguration_FromString(
    SilKit_ParticipantConfiguration** outParticipantConfiguration, const char* participantConfigurationString);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_ParticipantConfiguration_FromString_t)(
    SilKit_ParticipantConfiguration** outParticipantConfiguration, const char* participantConfigurationString);


/*! \brief Create an opaque participant configuration from the contents of the UTF-8 encoded text file.
 *
 * To destroy the participant configuration, call \ref SilKit_ParticipantConfiguration_Destroy.
 *
 * \param outParticipantConfiguration The pointer through which the participant configuration will be returned (out parameter).
 * \param participantConfigurationPath The path to the configuration file as a UTF-8 encoded string.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_ParticipantConfiguration_FromFile(
    SilKit_ParticipantConfiguration** outParticipantConfiguration, const char* participantConfigurationPath);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_ParticipantConfiguration_FromFile_t)(
    SilKit_ParticipantConfiguration** outParticipantConfiguration, const char* participantConfigurationPath);


/*! \brief Destroy a participant configuration.
 *
 * \param participantConfiguration The participant configuration to be destroyed.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL
SilKit_ParticipantConfiguration_Destroy(SilKit_ParticipantConfiguration* participantConfiguration);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_ParticipantConfiguration_Destroy_t)(
    SilKit_ParticipantConfiguration* participantConfiguration);


/*! \brief Return a JSON-string of the complete parsed participant configuration.
 *  \param inParticipantConfiguration the participant configuration to process
 *  \param outputJsonString the JSON string of the configuration. When this is NULL outputSize will contain the required size for the JSON string.
 *  \param outputSize the size of the outputJsonString is stored here. Can be called with outputJsonString set to NULL to get the required size.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_ParticipantConfiguration_ToJson(
    const SilKit_ParticipantConfiguration* inParticipantConfiguration, char** outputJsonString, size_t* outputSize);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_ParticipantConfiguration_ToJson_t)(
    const SilKit_ParticipantConfiguration* intParticipantConfiguration, char** outputJsonString, size_t* outputSize);

SILKIT_END_DECLS
