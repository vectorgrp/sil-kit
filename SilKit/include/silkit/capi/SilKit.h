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

SILKIT_BEGIN_DECLS

/*! \brief Get the corresponding static error string for a given return code.
 *
 * \param outString The pointer through which the resulting human readable error string will be returned.
 * \param returnCode The return code for which the string should be obtained.
 *
 * \ref SilKit_GetLastErrorString()
 *
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_ReturnCodeToString(const char** outString, SilKit_ReturnCode returnCode);

typedef  SilKit_ReturnCode (SilKitFPTR *SilKit_ReturnCodeToString_t)(const char** outString, SilKit_ReturnCode returnCode);

/*! \brief Get a human readable error description of the last error on the current thread.
 *
 * This method is intended to get specific error messages in case of a non success return code.
 * In comparison to SilKit_ReturnCodeToString this function returns dynamic and more specific error messages.
 *
 * \return A specific string containing the last error message of the current thread.
 *
 */
SilKitAPI const char* SilKitCALL SilKit_GetLastErrorString();

typedef const char*(SilKitFPTR *SilKit_GetLastErrorString_t)();

/*! \brief Create an opaque participant configuration from the configuration text.
 *
 * To destroy the participant configuration, call \ref SilKit_ParticipantConfiguration_Destroy.
 *
 * \param outParticipantConfiguration The pointer through which the participant configuration will be returned (out parameter).
 * \param participantConfigurationString The configuration as a string (e.g., read from a configuration file or a string constant).
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_ParticipantConfiguration_FromString(
    SilKit_ParticipantConfiguration** outParticipantConfiguration,
    const char* participantConfigurationString);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_ParticipantConfiguration_FromString_t)(
    SilKit_ParticipantConfiguration** outParticipantConfiguration,
    const char* participantConfigurationString);


/*! \brief Destroy a participant configuration.
 *
 * \param participantConfiguration The participant configuration to be destroyed.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_ParticipantConfiguration_Destroy(
    SilKit_ParticipantConfiguration* participantConfiguration);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_ParticipantConfiguration_Destroy_t)(
    SilKit_ParticipantConfiguration* participantConfiguration);


SILKIT_END_DECLS
