// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once
#include <stdint.h>
#include <limits.h>
#include "ib/capi/IbMacros.h"
#include "ib/capi/Types.h"
#include "ib/capi/Can.h"
#include "ib/capi/Lin.h"
#include "ib/capi/DataPubSub.h"
#include "ib/capi/Rpc.h"
#include "ib/capi/Ethernet.h"
#include "ib/capi/FlexRay.h"
#include "ib/capi/Logger.h"
#include "ib/capi/InterfaceIdentifiers.h"
#include "ib/capi/Participant.h"


IB_BEGIN_DECLS

/*! \brief Get the corresponding static error string for a given return code.
*
* \param outString The pointer through which the resulting human readable error string will be returned.
* \param returnCode The return code for which the string should be obtained.
*
* \ref ib_GetLastErrorString()
*
*/
IntegrationBusAPI ib_ReturnCode ib_ReturnCodeToString(const char** outString, ib_ReturnCode returnCode);

typedef  ib_ReturnCode (*ib_ReturnCodeToString_t)(const char** outString, ib_ReturnCode returnCode);

/*! \brief Get a human readable error description of the last error on the current thread.
*
* This method is intended to get specific error messages in case of a non success return code.
* In comparison to ib_ReturnCodeToString this function returns dynamic and more specific error messages.
*
* \return A specific string containing the last error message of the current thread.
*
*/
IntegrationBusAPI const char* ib_GetLastErrorString();

typedef  const char*(*ib_GetLastErrorString_t)();

typedef ib_ReturnCode(*ib_Participant_GetLogger_t)(
    ib_Logger** outLogger,
    ib_Participant* participant);

/*! \brief Obtain the logger of a particular simulation participant.
 *
 * \param outLogger Pointer to the resulting logger instance (out parameter).
 * \param participant The simulation participant whose logger should be returned.
 *
 * The lifetime of the returned logger is directly bound to the lifetime of the simulation participant.
 * There is no futher cleanup necessary, except for destroying the simulation participant at the end of the
 * simulation.
 */
IntegrationBusAPI ib_ReturnCode ib_Participant_GetLogger(
    ib_Logger** outLogger,
    ib_Participant* participant);


IB_END_DECLS
