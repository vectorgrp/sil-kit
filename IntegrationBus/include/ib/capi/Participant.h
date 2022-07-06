// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once
#include <stdint.h>
#include <limits.h>
#include "silkit/capi/SilKitMacros.h"
#include "silkit/capi/Types.h"
#include "silkit/capi/InterfaceIdentifiers.h"


#pragma pack(push)
#pragma pack(8)

SILKIT_BEGIN_DECLS

//!< The kind of participant command that is sent.
typedef int8_t SilKit_ParticipantCommand_Kind;
#define SilKit_ParticipantCommand_Kind_Invalid      ((SilKit_ParticipantCommand_Kind) 0) //!< An invalid command
#define SilKit_ParticipantCommand_Kind_Restart      ((SilKit_ParticipantCommand_Kind) 1) //!< The restart command
#define SilKit_ParticipantCommand_Kind_Shutdown     ((SilKit_ParticipantCommand_Kind) 2) //!< The shutdown command

//!< The numeric participant id.
typedef int32_t SilKit_ParticipantId;

//!< A command sent to a participant by a system controller.
struct SilKit_ParticipantCommand
{
    SilKit_ParticipantCommand_Kind kind; 
};

typedef struct SilKit_ParticipantCommand SilKit_ParticipantCommand;

//!< The state of a participant.
typedef int8_t SilKit_ParticipantState;
#define SilKit_ParticipantState_Invalid                     ((SilKit_ParticipantState)   0) //!< An invalid participant state
#define SilKit_ParticipantState_ServicesCreated             ((SilKit_ParticipantState)  10) //!< The controllers created state
#define SilKit_ParticipantState_CommunicationInitializing   ((SilKit_ParticipantState)  20) //!< The communication initializing state
#define SilKit_ParticipantState_CommunicationInitialized    ((SilKit_ParticipantState)  30) //!< The communication initialized state
#define SilKit_ParticipantState_ReadyToRun                  ((SilKit_ParticipantState)  40) //!< The initialized state
#define SilKit_ParticipantState_Running                     ((SilKit_ParticipantState)  50) //!< The running state
#define SilKit_ParticipantState_Paused                      ((SilKit_ParticipantState)  60) //!< The paused state
#define SilKit_ParticipantState_Stopping                    ((SilKit_ParticipantState)  70) //!< The stopping state
#define SilKit_ParticipantState_Stopped                     ((SilKit_ParticipantState)  80) //!< The stopped state
#define SilKit_ParticipantState_Error                       ((SilKit_ParticipantState)  90) //!< The error state
#define SilKit_ParticipantState_ShuttingDown                ((SilKit_ParticipantState) 100) //!< The shutting down state
#define SilKit_ParticipantState_Shutdown                    ((SilKit_ParticipantState) 110) //!< The shutdown state
#define SilKit_ParticipantState_Reinitializing              ((SilKit_ParticipantState) 120) //!< The reinitializing state

//!< The state of a system, deduced by states of the required participants.
typedef int8_t SilKit_SystemState;
#define SilKit_SystemState_Invalid                     ((SilKit_SystemState)   0) //!< An invalid participant state
#define SilKit_SystemState_ServicesCreated             ((SilKit_SystemState)  10) //!< The controllers created state
#define SilKit_SystemState_CommunicationInitializing   ((SilKit_SystemState)  20) //!< The communication initializing state
#define SilKit_SystemState_CommunicationInitialized    ((SilKit_SystemState)  30) //!< The communication initialized state
#define SilKit_SystemState_ReadyToRun                  ((SilKit_SystemState)  40) //!< The initialized state
#define SilKit_SystemState_Running                     ((SilKit_SystemState)  50) //!< The running state
#define SilKit_SystemState_Paused                      ((SilKit_SystemState)  60) //!< The paused state
#define SilKit_SystemState_Stopping                    ((SilKit_SystemState)  70) //!< The stopping state
#define SilKit_SystemState_Stopped                     ((SilKit_SystemState)  80) //!< The stopped state
#define SilKit_SystemState_Error                       ((SilKit_SystemState)  90) //!< The error state
#define SilKit_SystemState_ShuttingDown                ((SilKit_SystemState) 100) //!< The shutting down state
#define SilKit_SystemState_Shutdown                    ((SilKit_SystemState) 110) //!< The shutdown state
#define SilKit_SystemState_Reinitializing              ((SilKit_SystemState) 120) //!< The reinitializing state

typedef uint64_t SilKit_NanosecondsTime; //!< Simulation time

typedef uint64_t SilKit_NanosecondsWallclockTime; //!< Wall clock time since epoch

//!< Details about a status change of a participant.
typedef struct 
{
    SilKit_InterfaceIdentifier interfaceId;
    const char* participantName; //!< Name of the participant.
    SilKit_ParticipantState participantState; //!< The new state of the participant.
    const char* enterReason; //!< The reason for the participant to enter the new state.
    SilKit_NanosecondsWallclockTime enterTime; //!< The enter time of the participant.
    SilKit_NanosecondsWallclockTime refreshTime; //!< The refresh time.
} SilKit_ParticipantStatus;

//!< Details about a status change of a participant.
typedef struct
{
    SilKit_InterfaceIdentifier interfaceId;
    SilKit_StringList* requiredParticipantNames; //!< Participants that are waited for when coordinating the simulation start/stop.

} SilKit_WorkflowConfiguration;

/*! \brief Join the SilKit simulation hosted by the registry listening at URI as a participant.
*
* Join the SilKit simulation and become a participant
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

/*! \brief  The handler to be called on initialization
 *
 * \param context The user provided context passed in \ref SilKit_Participant_SetCommunicationReadyHandler
 * \param participant The simulation participant entering the initialized state
 */
typedef void (*SilKit_ParticipantCommunicationReadyHandler_t)(void* context, SilKit_Participant* participant);

/*! \brief Register a callback to perform initialization
 *
 * The handler is called when an \ref SilKit_ParticipantCommand_Kind_Initialize
 * or \ref SilKit_ParticipantCommand_Kind_Restart has been received.
 * The callback is executed in the context of the middleware
 * thread that received the command.
 * After the handler has been processed, the participant
 * switches to the \ref SilKit_ParticipantState_Initialized state.
 *
 * \param participant The simulation participant receiving the (re-)initialization command
 * \param context A user provided context accessible in the handler
 * \param handler The handler to be called on initialization
 */
SilKitAPI SilKit_ReturnCode SilKit_Participant_SetCommunicationReadyHandler(SilKit_Participant* participant,
    void* context, SilKit_ParticipantCommunicationReadyHandler_t handler);

typedef SilKit_ReturnCode (*SilKit_Participant_SetCommunicationReadyHandler_t)(
    SilKit_Participant* participant,
    void* context, SilKit_ParticipantCommunicationReadyHandler_t handler);

/*! \brief The handler to be called on a simulation stop
 *
 * \param context The user provided context passed in \ref SilKit_Participant_SetStopHandler
 * \param participant The simulation participant receiving the stop command
 */
typedef void (*SilKit_ParticipantStopHandler_t)(void* context, SilKit_Participant* participant);

/*! \brief Register a callback that is executed on simulation stop
 *
 * The handler is called when a \ref SystemCommand::Kind::Stop has been
 * received. It is executed in the context of the middleware
 * thread that received the command. After the handler has been
 * processed, the participant switches to the
 * \ref SilKit_ParticipantState_Stopped state.
 *
 * \param participant The simulation participant receiving the stop command
 * \param context A user provided context accessible in the handler
 * \param handler The handler to be called
 */
SilKitAPI SilKit_ReturnCode SilKit_Participant_SetStopHandler(SilKit_Participant* participant,
    void* context, SilKit_ParticipantStopHandler_t handler);

typedef SilKit_ReturnCode(*SilKit_Participant_SetStopHandler_t)(SilKit_Participant* participant,
    void* context, SilKit_ParticipantStopHandler_t handler);

/*! \brief The handler to be called on a simulation shutdown
 *
 * \param context The user provided context passed in \ref SilKit_Participant_SetShutdownHandler
 * \param participant The simulation participant receiving the shutdown command
 */
typedef void (*SilKit_ParticipantShutdownHandler_t)(void* context, SilKit_Participant* participant);

/*! \brief Register a callback that is executed on simulation shutdown.
 *
 * The handler is called when the \ref SystemCommand::Kind::Shutdown
 * has been received. It is executed in the context of the middleware
 * thread that received the command. After the handler has been
 * processed, the participant switches to the
 * \ref SilKit_ParticipantState_Shutdown state and is allowed to terminate.
 *
 * \param participant The simulation participant receiving the shutdown command
 * \param context A user provided context accessible in the handler
 * \param handler The handler to be called
 */
SilKitAPI SilKit_ReturnCode SilKit_Participant_SetShutdownHandler(SilKit_Participant* participant,
    void* context, SilKit_ParticipantShutdownHandler_t handler);

typedef SilKit_ReturnCode(*SilKit_Participant_SetShutdownHandler_t)(SilKit_Participant* participant,
    void* context, SilKit_ParticipantShutdownHandler_t handler);

/*! \brief Set the simulation duration to be requested
 *
 * Can only be used with time quantum synchronization.
 * 
 * \param participant The simulation participant
 * \param period The cycle time of the simulation task
 */
SilKitAPI SilKit_ReturnCode SilKit_Participant_SetPeriod(SilKit_Participant* participant, SilKit_NanosecondsTime period);

typedef SilKit_ReturnCode (*SilKit_Participant_SetPeriod_t)(SilKit_Participant* participant, SilKit_NanosecondsTime period);

/*! \brief The handler to be called if the simulation task is due
 *
 * \param context The user provided context passed in \ref SilKit_Participant_SetSimulationTask
 * \param participant The simulation participant
 * \param now The current simulation time
 */
typedef void (*SilKit_ParticipantSimulationTaskHandler_t)(void* context, SilKit_Participant* participant,
                                                   SilKit_NanosecondsTime now);
/*! \brief Set the task to be executed with each grant / tick
 *
 * Can be changed at runtime. Execution context depends on the run type.
 *
 * \param participant The simulation participant to start running
 * \param context A user provided context accessible in the handler
 * \param handler The handler to be called if the simulation task is due
 */
SilKitAPI SilKit_ReturnCode SilKit_Participant_SetSimulationTask(SilKit_Participant* participant,
    void* context, SilKit_ParticipantSimulationTaskHandler_t handler);

typedef SilKit_ReturnCode(*SilKit_Participant_SetSimulationTask_t)(SilKit_Participant* participant,
    void* context, SilKit_ParticipantSimulationTaskHandler_t handler);

/*! \brief Set the task to be executed with each grant / tick
 *
 * Can be changed at runtime. Execution context depends on the run type.
 *
 * The difference to SetSimulationTask is, that after execution of the simulation task
 * the advance in simulation time will NOT be signaled to other participants.
 * Progress in simulation time (including all other participants) will cease.
 * Instead, SilKit_Participant_CompleteSimulationTask must be called
 * FROM ANY OTHER THREAD to 'unlock' the thread executing the simulation task, and let it execute again.
 * Thus, a fine grained control over the whole simulation time progress can be achieved
 * by calling CompleteSimulationTask from an application thread.
 * Participants using 'regular' simulation tasks and non-blocking simulation tasks may be freely mixed.
 *
 * \param participant The simulation participant
 * \param context A user provided context accessible in the handler
 * \param handler The handler to be called if the simulation task is due
 */
SilKitAPI SilKit_ReturnCode SilKit_Participant_SetSimulationTaskAsync(SilKit_Participant* participant,
    void* context, SilKit_ParticipantSimulationTaskHandler_t handler);

typedef SilKit_ReturnCode(*SilKit_Participant_SetSimulationTaskNonBlocking_t)(SilKit_Participant* participant,
    void* context, SilKit_ParticipantSimulationTaskHandler_t handler);

/*! \brief Complete the current step of a non-blocking simulation task.
 *
 * \param participant The simulation participant
 */
SilKitAPI SilKit_ReturnCode SilKit_Participant_CompleteSimulationTask(SilKit_Participant* participant);

typedef SilKit_ReturnCode(*SilKit_Participant_CompleteSimulationTask_t)(SilKit_Participant* participant);

/*! \brief Send \ref the Restart command to a specific participant
  *
  *  The command is only allowed if the participant is in the
  *  SilKit_ParticipantState_Stopped or SilKit_ParticipantState_Error state.
  *
  *  \param participantName identifies the participant to be initialized
  *
  *  NB:
  *   - Parametrization is yet to be determined.
  *   - Restart is still subject to changed! It might be changed to
  *     a SystemCommand to Restart all participants without sending
  *     new parameters.
  */
SilKitAPI SilKit_ReturnCode SilKit_Participant_Restart(SilKit_Participant* participant, const char* participantName);

typedef SilKit_ReturnCode (*SilKit_Participant_Restart_t)(SilKit_Participant* participant, const char* participantName);

/*! \brief Send \ref the Run command to all participants
  *
  *  The command is only allowed if system is in state SilKit_SystemState_Initialized.
  */
SilKitAPI SilKit_ReturnCode SilKit_Participant_RunSimulation(SilKit_Participant* participant);

typedef SilKit_ReturnCode (*SilKit_Participant_RunSimulation_t)(SilKit_Participant* participant);

/*! \brief Send \ref the Stop command to all participants
  *
  *  The command is only allowed if system is in SilKit_SystemState_Running.
  */
SilKitAPI SilKit_ReturnCode SilKit_Participant_StopSimulation(SilKit_Participant* participant);

typedef SilKit_ReturnCode (*SilKit_Participant_StopSimulation_t)(SilKit_Participant* participant);

/*! \brief Pause execution of the participant
  *
  * Switch to \ref SilKit_ParticipantState_Paused due to the provided \p reason.
  *
  * When a client is in state \ref SilKit_ParticipantState_Paused,
  * it must not be considered as unresponsive even if a
  * health monitoring related timeout occurs.
  *
  * Precondition: State() == \ref SilKit_ParticipantState_Running
  */
SilKitAPI SilKit_ReturnCode SilKit_Participant_Pause(SilKit_Participant* participant, const char* reason);

typedef SilKit_ReturnCode (*SilKit_Participant_Pause_t)(SilKit_Participant* participant, const char* reason);

/*! \brief Switch back to \ref SilKit_ParticipantState_Running
  * after having paused.
  *
  * Precondition: State() == \ref SilKit_ParticipantState_Paused
  */
SilKitAPI SilKit_ReturnCode SilKit_Participant_Continue(SilKit_Participant* participant);

typedef SilKit_ReturnCode (*SilKit_Participant_Continue_t)(SilKit_Participant* participant);

/*! \brief Send \ref the Shutdown command to all participants
  *
  *  The command is only allowed if system is in
  *  SilKit_SystemState_Stopped or SilKit_SystemState_Error.
  */
SilKitAPI SilKit_ReturnCode SilKit_Participant_Shutdown(SilKit_Participant* participant);

typedef SilKit_ReturnCode(*SilKit_Participant_Shutdown_t)(SilKit_Participant* participant);

/*! \brief Get the current participant state of the participant given by participantName
  */
SilKitAPI SilKit_ReturnCode SilKit_Participant_GetParticipantState(SilKit_ParticipantState* outParticipantState,
  SilKit_Participant* participant, const char* participantName);

typedef SilKit_ReturnCode (*SilKit_Participant_GetParticipantState_t)(SilKit_ParticipantState* outParticipantState,
  SilKit_Participant* participant, const char* participantName);

//! \brief Get the current ::SystemState
SilKitAPI SilKit_ReturnCode SilKit_Participant_GetSystemState(SilKit_SystemState* outSystemState, SilKit_Participant* participant);

typedef SilKit_ReturnCode (*SilKit_Participant_GetSystemState_t)(SilKit_SystemState* outSystemState, SilKit_Participant* participant);

typedef void (*SilKit_SystemStateHandler_t)(void* context, SilKit_Participant* participant,
    SilKit_SystemState state);

/*! \brief Register a callback for ::SystemState changes
 *
 * If the current SystemState is not \ref SilKit_SystemState_Invalid,
 * the handler will be called immediately.
 *
 * \param participant The simulation participant
 * \param context The user context pointer made available to the handler
 * \param handler The handler to be called to be called when the ::SystemState changes
 * \param outHandlerId The handler identifier that can be used to remove the callback.
 */
SilKitAPI SilKit_ReturnCode SilKit_Participant_AddSystemStateHandler(SilKit_Participant* participant,
                                                                     void* context,
                                                                     SilKit_SystemStateHandler_t handler,
                                                                     SilKit_HandlerId* outHandlerId);
typedef SilKit_ReturnCode (*SilKit_Participant_AddSystemStateHandler_t)(SilKit_Participant* participant,
                                                                void* context,
                                                                SilKit_SystemStateHandler_t handler,
                                                                SilKit_HandlerId* outHandlerId);

/*! \brief  Remove a \ref SilKit_SystemStateHandler_t by SilKit_HandlerId on this participant
 *
 * \param participant The simulation participant
 * \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
 */
SilKitAPI SilKit_ReturnCode SilKit_Participant_RemoveSystemStateHandler(SilKit_Participant* participant,
                                                                        SilKit_HandlerId handlerId);

typedef SilKit_ReturnCode (*SilKit_Participant_RemoveSystemStateHandler_t)(SilKit_Participant* participant,
                                                                   SilKit_HandlerId handlerId);

typedef void (*SilKit_ParticipantStatusHandler_t)(void* context, SilKit_Participant* participant,
    const char* participantName, SilKit_ParticipantStatus* status);

/*! \brief Register a callback for status changes of participants.
 *
 * The handler will be called immediately for any participant that is
 * not in \ref SilKit_ParticipantState_Invalid.
 *
 * \param participant The simulation participant
 * \param context The user context pointer made available to the handler
 * \param handler The handler to be called to be called when the ::SystemState changes
 * \param outHandlerId The handler identifier that can be used to remove the callback.
 */
SilKitAPI SilKit_ReturnCode SilKit_Participant_AddParticipantStatusHandler(SilKit_Participant* participant,
                                                                           void* context,
                                                                           SilKit_ParticipantStatusHandler_t handler,
                                                                           SilKit_HandlerId* outHandlerId);

typedef SilKit_ReturnCode (*SilKit_Participant_AddParticipantStatusHandler_t)(SilKit_Participant* participant,
                                                                      void* context,
                                                                      SilKit_ParticipantStatusHandler_t handler,
                                                                      SilKit_HandlerId* outHandlerId);

/*! \brief  Remove a \ref SilKit_ParticipantStatusHandler_t by SilKit_HandlerId on this participant
 *
 * \param participant The simulation participant
 * \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
 */
SilKitAPI SilKit_ReturnCode SilKit_Participant_RemoveParticipantStatusHandler(SilKit_Participant* participant,
                                                                              SilKit_HandlerId handlerId);

typedef SilKit_ReturnCode (*SilKit_Participant_RemoveParticipantStatusHandler_t)(SilKit_Participant* participant,
                                                                         SilKit_HandlerId handlerId);

/*! \brief Configures details of the simulation workflow regarding lifecycle and participant coordination.
  *
  * Only these participants are taken into account to define the system state.
  * Further, the simulation time propagation also relies on the required participants.
  * This information is distributed to other participants, so it must only be set once by a single 
  * single member of the simulation.
  *
  * \param workflowConfigration The desired configuration, currently containing a list of required participants
  */
SilKitAPI SilKit_ReturnCode SilKit_Participant_SetWorkflowConfiguration(
    SilKit_Participant* participant, const SilKit_WorkflowConfiguration* workflowConfigration);

typedef SilKit_ReturnCode (*SilKit_Participant_SetWorkflowConfiguration_t)(
    SilKit_Participant* participant, const SilKit_WorkflowConfiguration* workflowConfigration);

//!< The LifecycleLifecycle options
typedef struct SilKit_LifecycleConfiguration
{
    SilKit_InterfaceIdentifier interfaceId;
    SilKit_Bool coordinatedStart;
    SilKit_Bool coordinatedStop;
} SilKit_LifecycleConfiguration;

/*! \brief Start the lifecycle with the given parameters without simulation time synchronization.
*  Requires a call to SilKit_Participant_WaitForLifecycleToComplete to retrieve the final state.
* 
* \param participant the instance of the participant.
* \param startConfiguration contains the desired start configuration of the lifecycle.
* 
*/

typedef SilKit_ReturnCode (*SilKit_Participant_StartLifecycleNoSyncTime_t)(
    SilKit_Participant*, SilKit_LifecycleConfiguration*);

SilKitAPI SilKit_ReturnCode SilKit_Participant_StartLifecycleNoSyncTime(
    SilKit_Participant* participant, SilKit_LifecycleConfiguration* startconfiguration);

/*! \brief Start the lifecycle with the given parameters with simulation time synchronization.
* 
* \param participant the instance of the participant.
* \param startConfiguration contains the desired start configuration of the lifecycle.
*/

typedef SilKit_ReturnCode (*SilKit_Participant_StartLifecycleWithSyncTime_t)(
    SilKit_Participant*, SilKit_LifecycleConfiguration*);

SilKitAPI SilKit_ReturnCode SilKit_Participant_StartLifecycleWithSyncTime(
    SilKit_Participant* participant, SilKit_LifecycleConfiguration* startConfiguration);


/*! \brief Wait for to asynchronous run operation to complete and return the final participant state
 *
 * Blocks until the simulation is shutdown. Prior to this method,
 * \ref SilKit_Participant_StartLifecycle{With,No}Sync has to be called.
 *
 * \param participant The simulation participant to wait for completing the asynchronous run operation
 * \param outParticipantState Pointer for storing the final participant state (out parameter)
 */
SilKitAPI SilKit_ReturnCode SilKit_Participant_WaitForLifecycleToComplete(
    SilKit_Participant* participant, SilKit_ParticipantState* outParticipantState);

typedef SilKit_ReturnCode (*SilKit_Participant_WaitForLifecycleToComplete_t)(
    SilKit_Participant* participant, SilKit_ParticipantState* outParticipantState);
SILKIT_END_DECLS

#pragma pack(pop)
