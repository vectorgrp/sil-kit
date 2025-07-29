// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once
#include <stdint.h>
#include <limits.h>
#include "silkit/capi/SilKitMacros.h"
#include "silkit/capi/Types.h"
#include "silkit/capi/Logger.h"

#pragma pack(push)
#pragma pack(8)

SILKIT_BEGIN_DECLS

typedef uint64_t SilKit_NanosecondsTime; //!< Simulation time

typedef uint64_t SilKit_NanosecondsWallclockTime; //!< Wall clock time since epoch

/*! \brief Join the SIL Kit simulation hosted by the registry listening at URI as a participant.
*
* Join the SIL Kit simulation and become a participant
* based on the given configuration options.
*
* \param outParticipant The pointer through which the simulation participant will be returned (out parameter).
* \param participantConfiguration Configuration of the participant (see \ref SilKit_ParticipantConfiguration_FromString)
* \param participantName Name of the participant
* \param registryUri The `silkit://` URI of the registry
*
*/
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Participant_Create(
    SilKit_Participant** outParticipant, SilKit_ParticipantConfiguration* participantConfiguration,
    const char* participantName, const char* registryUri);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_Participant_Create_t)(
    SilKit_Participant** outParticipant, SilKit_ParticipantConfiguration* participantConfiguration,
    const char* participantName, const char* registryUri);

/*! \brief Destroy a simulation participant and its associated simulation elements.
*
* Destroys the simulation participant and its created simulation elements such as e.g. Can controllers.
*
* \param participant The simulation participant to be destroyed.
*
*/
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Participant_Destroy(SilKit_Participant* participant);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_Participant_Destroy_t)(SilKit_Participant* participant);

/*! \brief Obtain the logger of a particular simulation participant.
 *
 * \param outLogger Pointer to the resulting logger instance (out parameter).
 * \param participant The simulation participant whose logger should be returned.
 *
 * The lifetime of the returned logger is directly bound to the lifetime of the simulation participant.
 * There is no futher cleanup necessary, except for destroying the simulation participant at the end of the
 * simulation.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Participant_GetLogger(SilKit_Logger** outLogger,
                                                                    SilKit_Participant* participant);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_Participant_GetLogger_t)(SilKit_Logger** outLogger,
                                                                      SilKit_Participant* participant);

SILKIT_END_DECLS

#pragma pack(pop)
