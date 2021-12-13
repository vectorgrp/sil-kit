// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once
#include <stdint.h>
#include <limits.h>
#include "ib/capi/IbMacros.h"
#include "ib/capi/Types.h"
#include "ib/capi/InterfaceIdentifiers.h"


#pragma pack(push)
#pragma pack(8)

IB_BEGIN_DECLS

//!< The kind of participant command that is sent.
typedef int8_t ib_ParticipantCommand_Kind;
#define ib_ParticipantCommand_Kind_Invalid      ((ib_ParticipantCommand_Kind) 0) //!< An invalid command
#define ib_ParticipantCommand_Kind_Initialize   ((ib_ParticipantCommand_Kind) 1) //!< The initialize command
#define ib_ParticipantCommand_Kind_ReInitialize ((ib_ParticipantCommand_Kind) 2) //!< The re-inizialize command

//!< The numeric participant id.
typedef int32_t ib_ParticipantId;

//!< A command sent to a participant by a system controller.
struct ib_ParticipantCommand
{
    ib_ParticipantCommand_Kind kind; 
};

typedef struct ib_ParticipantCommand ib_ParticipantCommand;

//!< The state of a participant.
typedef int8_t ib_ParticipantState;
#define ib_ParticipantState_Invalid               ((ib_ParticipantState) 0)   //!< An invalid participant state
#define ib_ParticipantState_Idle                  ((ib_ParticipantState) 1)   //!< The idle state
#define ib_ParticipantState_Initializing          ((ib_ParticipantState) 2)   //!< The initializing state
#define ib_ParticipantState_Initialized           ((ib_ParticipantState) 3)   //!< The initialized state
#define ib_ParticipantState_Running               ((ib_ParticipantState) 4)   //!< The running state
#define ib_ParticipantState_Paused                ((ib_ParticipantState) 5)   //!< The paused state
#define ib_ParticipantState_Stopping              ((ib_ParticipantState) 6)   //!< The stopping state
#define ib_ParticipantState_Stopped               ((ib_ParticipantState) 7)   //!< The stopped state
#define ib_ParticipantState_ColdswapPrepare       ((ib_ParticipantState) 8)   //!< The ColdswapPrepare state
#define ib_ParticipantState_ColdswapReady         ((ib_ParticipantState) 9)   //!< The ColdswapReady state
#define ib_ParticipantState_ColdswapShutdown      ((ib_ParticipantState) 10)  //!< The ColdswapShutdown state
#define ib_ParticipantState_ColdswapIgnored       ((ib_ParticipantState) 11)  //!< The ColdswapIgnored state
#define ib_ParticipantState_Error                 ((ib_ParticipantState) 12)  //!< The error state
#define ib_ParticipantState_ShuttingDown          ((ib_ParticipantState) 13)  //!< The shutting down state
#define ib_ParticipantState_Shutdown              ((ib_ParticipantState) 14)  //!< The shutdown state

typedef uint64_t ib_NanosecondsTime; //!< Simulation time

/*! \brief Join the IB simulation with the domainId as a participant.
*
* Join the IB simulation and become a participant
* based on the given configuration options.
*
* \param outParticipant The pointer through which the simulation participant will be returned (out parameter).
* \param config Configuration of the participant passed as JSON string
* \param participantName Name of the participant
* \param cDomainId ID of the domain/simulation to join
*
*/
IntegrationBusAPI ib_ReturnCode ib_SimulationParticipant_Create(ib_SimulationParticipant** outParticipant, 
    const char* cJsonConfig, const char* cParticipantName, const char* cDomainId);

typedef ib_ReturnCode (*ib_SimulationParticipant_Create_t)(ib_SimulationParticipant** outParticipant, 
    const char* cJsonConfig, const char* cParticipantName, const char* cDomainId);
    
/*! \brief Destroy a simulation participant and its associated simulation elements.
*
* Destroys the simulation participant and its created simulation elements such as e.g. Can controllers.
*
* \param participant The simulation participant to be destroyed.
*
*/
IntegrationBusAPI ib_ReturnCode ib_SimulationParticipant_Destroy(ib_SimulationParticipant* participant);

typedef ib_ReturnCode (*ib_SimulationParticipant_Destroy_t)(ib_SimulationParticipant* participant);

/*! \brief  The handler to be called on initialization
 *
 * \param context The user provided context passed in \ref ib_SimulationParticipant_SetInitHandler
 * \param participant The simulation participant receiving the (re-)initialization command
 * \param command The \ref ib_ParticipantCommand that triggered the initialization
 */
typedef void (*ib_ParticipantInitHandler_t)(void* context, ib_SimulationParticipant* participant,
    ib_ParticipantCommand* command);
/*! \brief Register a callback to perform initialization
 *
 * The handler is called when an \ref ib_ParticipantCommand_Kind_Initialize
 * or \ref ib_ParticipantCommand_Kind_ReInitialize has been received.
 * The callback is executed in the context of the middleware
 * thread that received the command.
 * After the handler has been processed, the participant
 * switches to the \ref ib_ParticipantState_Initialized state.
 *
 * \param participant The simulation participant receiving the (re-)initialization command
 * \param context A user provided context accessible in the handler
 * \param handler The handler to be called on initialization
 */
IntegrationBusAPI ib_ReturnCode ib_SimulationParticipant_SetInitHandler(ib_SimulationParticipant* participant,
    void* context, ib_ParticipantInitHandler_t handler);

typedef ib_ReturnCode(*ib_SimulationParticipant_SetInitHandler_t)(ib_SimulationParticipant* participant,
    void* context, ib_ParticipantInitHandler_t handler);

/*! \brief The handler to be called on a simulation stop
 *
 * \param context The user provided context passed in \ref ib_SimulationParticipant_SetStopHandler
 * \param participant The simulation participant receiving the stop command
 */
typedef void (*ib_ParticipantStopHandler_t)(void* context, ib_SimulationParticipant* participant);
/*! \brief Register a callback that is executed on simulation stop
 *
 * The handler is called when a \ref SystemCommand::Kind::Stop has been
 * received. It is executed in the context of the middleware
 * thread that received the command. After the handler has been
 * processed, the participant switches to the
 * \ref ib_ParticipantState_Stopped state.
 *
 * \param participant The simulation participant receiving the stop command
 * \param context A user provided context accessible in the handler
 * \param handler The handler to be called
 */
IntegrationBusAPI ib_ReturnCode ib_SimulationParticipant_SetStopHandler(ib_SimulationParticipant* participant,
    void* context, ib_ParticipantStopHandler_t handler);

typedef ib_ReturnCode(*ib_SimulationParticipant_SetStopHandler_t)(ib_SimulationParticipant* participant,
    void* context, ib_ParticipantStopHandler_t handler);

/*! \brief The handler to be called on a simulation shutdown
 *
 * \param context The user provided context passed in \ref ib_SimulationParticipant_SetShutdownHandler
 * \param participant The simulation participant receiving the shutdown command
 */
typedef void (*ib_ParticipantShutdownHandler_t)(void* context, ib_SimulationParticipant* participant);
/*! \brief Register a callback that is executed on simulation shutdown.
 *
 * The handler is called when the \ref SystemCommand::Kind::Shutdown
 * has been received. It is executed in the context of the middleware
 * thread that received the command. After the handler has been
 * processed, the participant switches to the
 * \ref ib_ParticipantState_Shutdown state and is allowed to terminate.
 *
 * \param participant The simulation participant receiving the shutdown command
 * \param context A user provided context accessible in the handler
 * \param handler The handler to be called
 */
IntegrationBusAPI ib_ReturnCode ib_SimulationParticipant_SetShutdownHandler(ib_SimulationParticipant* participant,
    void* context, ib_ParticipantShutdownHandler_t handler);

typedef ib_ReturnCode(*ib_SimulationParticipant_SetShutdownHandler_t)(ib_SimulationParticipant* participant,
    void* context, ib_ParticipantShutdownHandler_t handler);


/*! \brief Start the blocking run operation
 *
 * Executes simulation until shutdown is received. The simulation
 * task is executed in the context of the calling thread.
 *
 * \param participant The simulation participant to start running
 * \param outParticipantState Pointer for storing the final participant state (out parameter)
 * 
 */
IntegrationBusAPI ib_ReturnCode ib_SimulationParticipant_Run(
    ib_SimulationParticipant* participant, ib_ParticipantState* outParticipantState);

typedef ib_ReturnCode (*ib_SimulationParticipant_Run_t)(
    ib_SimulationParticipant* participant, ib_ParticipantState* outParticipantState);

/*! \brief Start the non blocking run operation, returns immediately
 *
 * Executes simulation until shutdown is received. The simulation
 * task is executed in the context of the middleware thread that
 * receives the grant or tick.
 *
 * NB: RunAsync() cannot be used with
 * \ref ib::cfg::TimeSync::SyncPolicy::Strict,
 * which will inherently lead to a deadlock!
 *
 * \param participant The simulation participant to start running
 */
IntegrationBusAPI ib_ReturnCode ib_SimulationParticipant_RunAsync(ib_SimulationParticipant* participant);

typedef ib_ReturnCode (*ib_SimulationParticipant_RunAsync_t)(ib_SimulationParticipant* participant);

/*! \brief Wait for to asynchronous run operation to complete and return the final participant state
 *
 * Blocks until the simulation is shutdown. Prior to this method,
 * \ref ib_SimulationParticipant_RunAsync has to be called.
 *
 * \param participant The simulation participant to wait for completing the asynchronous run operation
 * \param outParticipantState Pointer for storing the final participant state (out parameter)
 */
IntegrationBusAPI ib_ReturnCode ib_SimulationParticipant_WaitForRunAsyncToComplete(
    ib_SimulationParticipant* participant, ib_ParticipantState* outParticipantState);

typedef ib_ReturnCode (*ib_SimulationParticipant_WaitForRunAsyncToComplete_t)(
    ib_SimulationParticipant* participant, ib_ParticipantState* outParticipantState);

typedef ib_ReturnCode (*ib_SimulationParticipant_SetPeriod_t)(ib_SimulationParticipant* participant,
                                                              ib_NanosecondsTime period);
/*! \brief Set the simulation duration to be requested
 *
 * Can only be used with time quantum synchronization.
 * 
 * \param participant The simulation participant
 * \param period The cycle time of the simulation task
 */
IntegrationBusAPI ib_ReturnCode ib_SimulationParticipant_SetPeriod(ib_SimulationParticipant* participant,
                                                                    ib_NanosecondsTime        period);

/*! \brief The handler to be called if the simulation task is due
 *
 * \param context The user provided context passed in \ref ib_SimulationParticipant_RunAsync
 * \param participant The simulation participant
 * \param now The current simulation time
 */
typedef void (*ib_ParticipantSimulationTaskHandler_t)(void* context, ib_SimulationParticipant* participant,
                                                   ib_NanosecondsTime now);
/*! \brief Set the task to be executed with each grant / tick
 *
 * Can be changed at runtime. Execution context depends on the run type.
 *
 * \param participant The simulation participant to start running
 * \param context A user provided context accessible in the handler
 * \param handler The handler to be called if the simulation task is due
 */
IntegrationBusAPI ib_ReturnCode ib_SimulationParticipant_SetSimulationTask(ib_SimulationParticipant* participant,
    void* context, ib_ParticipantSimulationTaskHandler_t handler);

typedef ib_ReturnCode(*ib_SimulationParticipant_SetSimulationTask_t)(ib_SimulationParticipant* participant,
    void* context, ib_ParticipantSimulationTaskHandler_t handler);

IB_END_DECLS

#pragma pack(pop)
