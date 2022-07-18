// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once
#include <stdint.h>
#include <limits.h>
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


SILKIT_BEGIN_DECLS

/*! \brief Get the corresponding static error string for a given return code.
*
* \param outString The pointer through which the resulting human readable error string will be returned.
* \param returnCode The return code for which the string should be obtained.
*
* \ref SilKit_GetLastErrorString()
*
*/
SilKitAPI SilKit_ReturnCode SilKit_ReturnCodeToString(const char** outString, SilKit_ReturnCode returnCode);

typedef  SilKit_ReturnCode (*SilKit_ReturnCodeToString_t)(const char** outString, SilKit_ReturnCode returnCode);

/*! \brief Get a human readable error description of the last error on the current thread.
*
* This method is intended to get specific error messages in case of a non success return code.
* In comparison to SilKit_ReturnCodeToString this function returns dynamic and more specific error messages.
*
* \return A specific string containing the last error message of the current thread.
*
*/
SilKitAPI const char* SilKit_GetLastErrorString();

typedef  const char*(*SilKit_GetLastErrorString_t)();

typedef SilKit_ReturnCode(*SilKit_Participant_CreateLogger_t)(
    SilKit_Logger** outLogger,
    SilKit_Participant* participant);

/*! \brief Obtain the logger of a particular simulation participant.
 *
 * \param outLogger Pointer to the resulting logger instance (out parameter).
 * \param participant The simulation participant whose logger should be returned.
 *
 * The lifetime of the returned logger is directly bound to the lifetime of the simulation participant.
 * There is no futher cleanup necessary, except for destroying the simulation participant at the end of the
 * simulation.
 */
SilKitAPI SilKit_ReturnCode SilKit_Participant_CreateLogger(
    SilKit_Logger** outLogger,
    SilKit_Participant* participant);


SILKIT_END_DECLS
