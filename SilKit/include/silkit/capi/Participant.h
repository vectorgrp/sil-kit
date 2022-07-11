// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once
#include <stdint.h>
#include <limits.h>
#include "silkit/capi/SilKitMacros.h"
#include "silkit/capi/Types.h"


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
* \param config Configuration of the participant passed as YAML/JSON string
* \param participantName Name of the participant
* \param cRegistryUri The `silkit://` URI of the registry
*
*/
SilKitAPI SilKit_ReturnCode SilKit_Participant_Create(SilKit_Participant** outParticipant, 
    const char* cJsonConfig, const char* cParticipantName, const char* cRegistryUri, SilKit_Bool isSynchronized);

typedef SilKit_ReturnCode (*SilKit_Participant_Create_t)(SilKit_Participant** outParticipant, 
    const char* cJsonConfig, const char* cParticipantName, const char* cRegistryUri, SilKit_Bool isSynchronized);
    
/*! \brief Destroy a simulation participant and its associated simulation elements.
*
* Destroys the simulation participant and its created simulation elements such as e.g. Can controllers.
*
* \param participant The simulation participant to be destroyed.
*
*/
SilKitAPI SilKit_ReturnCode SilKit_Participant_Destroy(SilKit_Participant* participant);

typedef SilKit_ReturnCode (*SilKit_Participant_Destroy_t)(SilKit_Participant* participant);

SILKIT_END_DECLS

#pragma pack(pop)
