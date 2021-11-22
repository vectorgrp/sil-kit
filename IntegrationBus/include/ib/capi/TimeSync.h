/* Copyright (c) Vector Informatik GmbH. All rights reserved. */

#pragma once
#include <stdint.h>
#include "ib/capi/Utils.h"
#include "ib/capi/InterfaceIdentifiers.h"

#pragma pack(push)
#pragma pack(8)

__IB_BEGIN_DECLS

typedef uint64_t ib_NanosecondsTime; //!< Simulation time


typedef ib_ReturnCode (*ib_SimulationParticipant_SetPeriod_t)(ib_SimulationParticipant* participant,
                                                              ib_NanosecondsTime period);
/*! \brief Set the simulation duration to be requested
 *
 * Can only be used with time quantum synchronization.
 * 
 * \param participant The simulation participant
 * \param period The cycle time of the simulation task
 */
CIntegrationBusAPI ib_ReturnCode ib_SimulationParticipant_SetPeriod(ib_SimulationParticipant* participant,
                                                                    ib_NanosecondsTime        period);

/*! \brief The handler to be called if the simulation task is due
 *
 * \param context The user provided context passed in \ref ib_SimulationParticipant_RunAsync
 * \param participant The simulation participant
 * \param now The current simulation time
 */
typedef void (*ib_ParticipantSimulationTaskHandler_t)(void* context, ib_SimulationParticipant* participant,
                                                   ib_NanosecondsTime now);
typedef ib_ReturnCode(*ib_SimulationParticipant_SetSimulationTask_t)(ib_SimulationParticipant* participant,
    void* context, ib_ParticipantSimulationTaskHandler_t handler);

/*! \brief Set the task to be executed with each grant / tick
 *
 * Can be changed at runtime. Execution context depends on the run type.
 *
 * \param participant The simulation participant to start running
 * \param context A user provided context accessible in the handler
 * \param handler The handler to be called if the simulation task is due
 */
CIntegrationBusAPI ib_ReturnCode ib_SimulationParticipant_SetSimulationTask(ib_SimulationParticipant* participant,
    void* context, ib_ParticipantSimulationTaskHandler_t handler);

__IB_END_DECLS

#pragma pack(pop)
