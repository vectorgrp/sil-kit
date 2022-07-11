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
#define SilKit_ParticipantCommand_Kind_Invalid ((SilKit_ParticipantCommand_Kind)0) //!< An invalid command
#define SilKit_ParticipantCommand_Kind_Restart ((SilKit_ParticipantCommand_Kind)1) //!< The restart command
#define SilKit_ParticipantCommand_Kind_Shutdown ((SilKit_ParticipantCommand_Kind)2) //!< The shutdown command

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
#define SilKit_ParticipantState_Invalid ((SilKit_ParticipantState)0) //!< An invalid participant state
#define SilKit_ParticipantState_ServicesCreated ((SilKit_ParticipantState)10) //!< The controllers created state
#define SilKit_ParticipantState_CommunicationInitializing \
    ((SilKit_ParticipantState)20) //!< The communication initializing state
#define SilKit_ParticipantState_CommunicationInitialized \
    ((SilKit_ParticipantState)30) //!< The communication initialized state
#define SilKit_ParticipantState_ReadyToRun ((SilKit_ParticipantState)40) //!< The initialized state
#define SilKit_ParticipantState_Running ((SilKit_ParticipantState)50) //!< The running state
#define SilKit_ParticipantState_Paused ((SilKit_ParticipantState)60) //!< The paused state
#define SilKit_ParticipantState_Stopping ((SilKit_ParticipantState)70) //!< The stopping state
#define SilKit_ParticipantState_Stopped ((SilKit_ParticipantState)80) //!< The stopped state
#define SilKit_ParticipantState_Error ((SilKit_ParticipantState)90) //!< The error state
#define SilKit_ParticipantState_ShuttingDown ((SilKit_ParticipantState)100) //!< The shutting down state
#define SilKit_ParticipantState_Shutdown ((SilKit_ParticipantState)110) //!< The shutdown state
#define SilKit_ParticipantState_Reinitializing ((SilKit_ParticipantState)120) //!< The reinitializing state

//!< The state of a system, deduced by states of the required participants.
typedef int8_t SilKit_SystemState;
#define SilKit_SystemState_Invalid ((SilKit_SystemState)0) //!< An invalid participant state
#define SilKit_SystemState_ServicesCreated ((SilKit_SystemState)10) //!< The controllers created state
#define SilKit_SystemState_CommunicationInitializing ((SilKit_SystemState)20) //!< The communication initializing state
#define SilKit_SystemState_CommunicationInitialized ((SilKit_SystemState)30) //!< The communication initialized state
#define SilKit_SystemState_ReadyToRun ((SilKit_SystemState)40) //!< The initialized state
#define SilKit_SystemState_Running ((SilKit_SystemState)50) //!< The running state
#define SilKit_SystemState_Paused ((SilKit_SystemState)60) //!< The paused state
#define SilKit_SystemState_Stopping ((SilKit_SystemState)70) //!< The stopping state
#define SilKit_SystemState_Stopped ((SilKit_SystemState)80) //!< The stopped state
#define SilKit_SystemState_Error ((SilKit_SystemState)90) //!< The error state
#define SilKit_SystemState_ShuttingDown ((SilKit_SystemState)100) //!< The shutting down state
#define SilKit_SystemState_Shutdown ((SilKit_SystemState)110) //!< The shutdown state
#define SilKit_SystemState_Reinitializing ((SilKit_SystemState)120) //!< The reinitializing state

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
    SilKit_StringList*
        requiredParticipantNames; //!< Participants that are waited for when coordinating the simulation start/stop.

} SilKit_WorkflowConfiguration;

typedef struct SilKit_SystemMonitor SilKit_SystemMonitor;
typedef struct SilKit_SystemController SilKit_SystemController;
typedef struct SilKit_LifecycleService SilKit_LifecycleService;
typedef struct SilKit_TimeSyncService SilKit_TimeSyncService;

/*! \brief Create a system monitor at this SilKit simulation participant.
 * \param outSystemMonitor Pointer that refers to the resulting sytem monitor (out parameter).
 * \param participant The simulation participant at which the system monitor should be created.
 *
 * The object returned must not be deallocated using free()!
 */
SilKitAPI SilKit_ReturnCode SilKit_SystemMonitor_Create(SilKit_SystemMonitor** outSystemMonitor,
                                                        SilKit_Participant* participant);

typedef SilKit_ReturnCode (*SilKit_SystemMonitor_Create_t)(SilKit_SystemMonitor** outCanController,
                                                           SilKit_Participant* participant);

/*! \brief Create a system controller at this SilKit simulation participant.
 * \param outSystemController Pointer that refers to the resulting system controller (out parameter).
 * \param participant The simulation participant at which the system controller should be created.
 *
 * The object returned must not be deallocated using free()!
 */
SilKitAPI SilKit_ReturnCode SilKit_SystemController_Create(SilKit_SystemController** outSystemController,
                                                           SilKit_Participant* participant);

typedef SilKit_ReturnCode (*SilKit_SystemController_Create_t)(SilKit_SystemController** outCanController,
                                                              SilKit_Participant* participant);

/*! \brief Create a lifecycle service at this SilKit simulation participant.
 * \param outLifecycleService Pointer that refers to the resulting lifecycle service (out parameter).
 * \param participant The simulation participant at which the lifecycle service should be created.
 *
 * The object returned must not be deallocated using free()!
 */
SilKitAPI SilKit_ReturnCode SilKit_LifecycleService_Create(SilKit_LifecycleService** outLifecycleService,
                                                           SilKit_Participant* participant);

typedef SilKit_ReturnCode (*SilKit_LifecycleService_Create_t)(SilKit_LifecycleService** outLifecycleService,
                                                              SilKit_Participant* participant);

/*! \brief Create a time sync service at this SilKit simulation participant.
 * \param outTimeSyncService Pointer that refers to the resulting time sync service (out parameter).
 * \param lifecycleService The lifecyle service at which the time sync service should be created.
 *
 * The object returned must not be deallocated using free()!
 */
SilKitAPI SilKit_ReturnCode SilKit_TimeSyncService_Create(SilKit_TimeSyncService** outTimeSyncService,
                                                          SilKit_LifecycleService* lifecycleService);

typedef SilKit_ReturnCode (*SilKit_TimeSyncService_Create_t)(SilKit_TimeSyncService** outTimeSyncService,
                                                             SilKit_Participant* lifecycleService);

/*! \brief  The handler to be called on initialization
 *
 * \param context The user provided context passed in \ref SilKit_LifecycleService_SetCommunicationReadyHandler
 * \param participant The simulation participant entering the initialized state
 */
typedef void (*SilKit_LifecycleService_CommunicationReadyHandler_t)(void* context,
                                                                    SilKit_LifecycleService* lifecycleService);

/*! \brief Register a callback to perform initialization
 *
 * The handler is called when an \ref SilKit_ParticipantCommand_Kind_Initialize
 * or \ref SilKit_ParticipantCommand_Kind_Restart has been received.
 * The callback is executed in the context of the middleware
 * thread that received the command.
 * After the handler has been processed, the participant
 * switches to the \ref SilKit_ParticipantState_Initialized state.
 *
 * \param lifecycleService The lifecycle service receiving the (re-)initialization command
 * \param context A user provided context accessible in the handler
 * \param handler The handler to be called on initialization
 */
SilKitAPI SilKit_ReturnCode SilKit_LifecycleService_SetCommunicationReadyHandler(
    SilKit_LifecycleService* lifecycleService, void* context, SilKit_LifecycleService_CommunicationReadyHandler_t handler);

typedef SilKit_ReturnCode (*SilKit_LifecycleService_SetCommunicationReadyHandler_t)(
    SilKit_LifecycleService* lifecycleService, void* context, SilKit_LifecycleService_CommunicationReadyHandler_t handler);

/*! \brief  This handler is triggered just before the lifecylce service changes to ParticipantState::Running.
 * It is only triggered if the participant does NOT use virtual time synchronization.
 * It does not block other participants from changing to ParticipantState::Running and should only be used for lightweight operations such as starting timers.
 *
 * \param context The user provided context passed in \ref SilKit_LifecycleService_SetCommunicationReadyHandler
 * \param lifecycleService The lifecycle service receiving the update
 */
typedef void (*SilKit_LifecycleService_StartingHandler_t)(void* context, SilKit_LifecycleService* lifecycleService);

/*! \brief (Asynchronous participants only) Register a callback that is executed once directly before the participant enters ParticipantState::Run.
 *
 * This handler is triggered just before the participant changes to ParticipantState::Running.
 * It is only triggered if the participant does NOT use virtual time synchronization.
 * It does not block other participants from changing to ParticipantState::Running and should only be used for lightweight operations such as starting timers.
 * TODO fill in on which thread this is executed.
 * After the handler has been processed, the participant
 * switches to the \ref ParticipantState::Running state.
 * 
 * \param lifecycleService The lifecycle service that triggers the StartingHandler
 * \param context A user provided context accessible in the handler
 * \param handler The handler to be called when starting
 */

SilKitAPI SilKit_ReturnCode SilKit_LifecycleService_SetStartingHandler(
    SilKit_LifecycleService* lifecycleService, void* context, SilKit_LifecycleService_CommunicationReadyHandler_t handler);

typedef SilKit_ReturnCode (*SilKit_LifecycleService_SetStartingHandler_t)(
    SilKit_LifecycleService* lifecycleService, void* context, SilKit_LifecycleService_StartingHandler_t handler);

/*! \brief The handler to be called on a simulation stop
 *
 * \param context The user provided context passed in \ref SilKit_LifecycleService_SetStopHandler
 * \param lifecycleService The lifecycle service receiving the stop command
 */
typedef void (*SilKit_LifecycleService_StopHandler_t)(void* context, SilKit_LifecycleService* lifecycleService);

/*! \brief Register a callback that is executed on simulation stop
 *
 * The handler is called when a \ref SystemCommand::Kind::Stop has been
 * received. It is executed in the context of the middleware
 * thread that received the command. After the handler has been
 * processed, the participant switches to the
 * \ref SilKit_ParticipantState_Stopped state.
 *
 * \param lifecycleService The lifecycle service receiving the stop command
 * \param context A user provided context accessible in the handler
 * \param handler The handler to be called
 */
SilKitAPI SilKit_ReturnCode SilKit_LifecycleService_SetStopHandler(SilKit_LifecycleService* lifecycleService,
                                                                   void* context,
                                                                   SilKit_LifecycleService_StopHandler_t handler);

typedef SilKit_ReturnCode (*SilKit_LifecycleService_SetStopHandler_t)(SilKit_LifecycleService* lifecycleService,
                                                                      void* context,
                                                                      SilKit_LifecycleService_StopHandler_t handler);

/*! \brief The handler to be called on a simulation shutdown
 *
 * \param context The user provided context passed in \ref SilKit_LifecycleService_SetShutdownHandler
 * \param participant The simulation participant receiving the shutdown command
 */
typedef void (*SilKit_LifecycleService_ShutdownHandler_t)(void* context, SilKit_LifecycleService* lifecycleService);

/*! \brief Register a callback that is executed on simulation shutdown.
 *
 * The handler is called when the \ref SystemCommand::Kind::Shutdown
 * has been received. It is executed in the context of the middleware
 * thread that received the command. After the handler has been
 * processed, the participant switches to the
 * \ref SilKit_ParticipantState_Shutdown state and is allowed to terminate.
 *
 * \param lifecycleService The lifecycle service receiving the shutdown command
 * \param context A user provided context accessible in the handler
 * \param handler The handler to be called
 */
SilKitAPI SilKit_ReturnCode SilKit_LifecycleService_SetShutdownHandler(
    SilKit_LifecycleService* lifecycleService, void* context, SilKit_LifecycleService_ShutdownHandler_t handler);

typedef SilKit_ReturnCode (*SilKit_LifecycleService_SetShutdownHandler_t)(
    SilKit_LifecycleService* lifecycleService, void* context, SilKit_LifecycleService_ShutdownHandler_t handler);

/*! \brief Set the simulation duration to be requested
 *
 * Can only be used with time quantum synchronization.
 * 
 * \param participant The simulation participant
 * \param period The cycle time of the simulation task
 */
SilKitAPI SilKit_ReturnCode SilKit_TimeSyncService_SetPeriod(SilKit_TimeSyncService* timeSyncService,
                                                             SilKit_NanosecondsTime period);

typedef SilKit_ReturnCode (*SilKit_TimeSyncService_SetPeriod_t)(SilKit_TimeSyncService* timeSyncService,
                                                                SilKit_NanosecondsTime period);

/*! \brief The handler to be called if the simulation task is due
 *
 * \param context The user provided context passed in \ref SilKit_TimeSyncService_SetSimulationTask
 * \param timeSyncService The time sync service
 * \param now The current simulation time
 */
typedef void (*SilKit_TimeSyncService_SimulationTaskHandler_t)(void* context, SilKit_TimeSyncService* timeSyncService,
                                                               SilKit_NanosecondsTime now);
/*! \brief Set the task to be executed with each grant / tick
 *
 * Can be changed at runtime. Execution context depends on the run type.
 *
 * \param participant The simulation participant to start running
 * \param context A user provided context accessible in the handler
 * \param handler The handler to be called if the simulation task is due
 */
SilKitAPI SilKit_ReturnCode SilKit_TimeSyncService_SetSimulationTask(
    SilKit_TimeSyncService* timeSyncService, void* context, SilKit_TimeSyncService_SimulationTaskHandler_t handler);

typedef SilKit_ReturnCode (*SilKit_TimeSyncService_SetSimulationTask_t)(
    SilKit_TimeSyncService* timeSyncService, void* context, SilKit_TimeSyncService_SimulationTaskHandler_t handler);

/*! \brief Set the task to be executed with each grant / tick
 *
 * Can be changed at runtime. Execution context depends on the run type.
 *
 * The difference to SetSimulationTask is, that after execution of the simulation task
 * the advance in simulation time will NOT be signaled to other participants.
 * Progress in simulation time (including all other participants) will cease.
 * Instead, SilKit_TimeSyncService_CompleteSimulationTask must be called
 * FROM ANY OTHER THREAD to 'unlock' the thread executing the simulation task, and let it execute again.
 * Thus, a fine grained control over the whole simulation time progress can be achieved
 * by calling CompleteSimulationTask from an application thread.
 * Participants using 'regular' simulation tasks and non-blocking simulation tasks may be freely mixed.
 *
 * \param participant The simulation participant
 * \param context A user provided context accessible in the handler
 * \param handler The handler to be called if the simulation task is due
 */
SilKitAPI SilKit_ReturnCode SilKit_TimeSyncService_SetSimulationTaskAsync(
    SilKit_TimeSyncService* timeSyncService, void* context, SilKit_TimeSyncService_SimulationTaskHandler_t handler);

typedef SilKit_ReturnCode (*SilKit_TimeSyncService_SetSimulationTaskAsync_t)(
    SilKit_TimeSyncService* timeSyncService, void* context, SilKit_TimeSyncService_SimulationTaskHandler_t handler);

/*! \brief Complete the current step of a non-blocking simulation task.
 *
 * \param timeSyncService The time sync service
 */
SilKitAPI SilKit_ReturnCode SilKit_TimeSyncService_CompleteSimulationTask(SilKit_TimeSyncService* timeSyncService);

typedef SilKit_ReturnCode (*SilKit_TimeSyncService_CompleteSimulationTask_t)(SilKit_TimeSyncService* timeSyncService);

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
SilKitAPI SilKit_ReturnCode SilKit_SystemController_Restart(SilKit_SystemController* systemController,
                                                            const char* participantName);

typedef SilKit_ReturnCode (*SilKit_SystemController_Restart_t)(SilKit_SystemController* systemController,
                                                               const char* participantName);

/*! \brief Send \ref the Run command to all participants
  *
  *  The command is only allowed if system is in state SilKit_SystemState_Initialized.
  */
SilKitAPI SilKit_ReturnCode SilKit_SystemController_Run(SilKit_SystemController* systemController);

typedef SilKit_ReturnCode (*SilKit_SystemController_Run_t)(SilKit_Participant* participant);

/*! \brief Send \ref the Stop command to all participants
  *
  *  The command is only allowed if system is in SilKit_SystemState_Running.
  */
SilKitAPI SilKit_ReturnCode SilKit_SystemController_Stop(SilKit_SystemController* systemController);

typedef SilKit_ReturnCode (*SilKit_SystemController_Stop_t)(SilKit_SystemController* systemController);

/*! \brief Send \ref the Shutdown command to all participants
  *
  *  The command is only allowed if system is in
  *  SilKit_SystemState_Stopped or SilKit_SystemState_Error.
  */
SilKitAPI SilKit_ReturnCode SilKit_SystemController_Shutdown(SilKit_SystemController* systemController,
                                                             const char* participantName);

typedef SilKit_ReturnCode (*SilKit_SystemController_Shutdown_t)(SilKit_SystemController* systemController,
                                                                const char* participantName);


/*! \brief Send \ref SystemCommand::Kind::AbortSimulation to all participants
 *
 *  The abort simulation command signals all participants to terminate their
 *  lifecycle, regardless of their current state.
 *
 *  The command is allowed at any time.
 */
SilKitAPI SilKit_ReturnCode SilKit_SystemController_AbortSimulation(SilKit_SystemController* systemController);

typedef SilKit_ReturnCode (*SilKit_SystemController_AbortSimulation_t)(SilKit_SystemController* systemController);

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
SilKitAPI SilKit_ReturnCode SilKit_LifecycleService_Pause(SilKit_LifecycleService* lifecycleService,
                                                          const char* reason);

typedef SilKit_ReturnCode (*SilKit_LifecycleService_Pause_t)(SilKit_LifecycleService* lifecycleService,
                                                             const char* reason);

/*! \brief Switch back to \ref SilKit_ParticipantState_Running
  * after having paused.
  *
  * Precondition: State() == \ref SilKit_ParticipantState_Paused
  */
SilKitAPI SilKit_ReturnCode SilKit_LifecycleService_Continue(SilKit_LifecycleService* lifecycleService);

typedef SilKit_ReturnCode (*SilKit_LifecycleService_Continue_t)(SilKit_LifecycleService* lifecycleService);

/*! \brief Get the current participant state of the participant given by participantName
  */
SilKitAPI SilKit_ReturnCode SilKit_SystemMonitor_GetParticipantStatus(SilKit_ParticipantStatus* outParticipantState,
                                                                      SilKit_SystemMonitor* systemMonitor,
                                                                      const char* participantName);

typedef SilKit_ReturnCode (*SilKit_SystemMonitor_GetParticipantStatus_t)(SilKit_ParticipantStatus* outParticipantState,
                                                                         SilKit_SystemMonitor* systemMonitor,
                                                                         const char* participantName);

//! \brief Get the current ::SystemState
SilKitAPI SilKit_ReturnCode SilKit_SystemMonitor_GetSystemState(SilKit_SystemState* outSystemState,
                                                                SilKit_SystemMonitor* systemMonitor);

typedef SilKit_ReturnCode (*SilKit_SystemMonitor_GetSystemState_t)(SilKit_SystemState* outSystemState,
                                                                   SilKit_SystemMonitor* systemMonitor);

typedef void (*SilKit_SystemStateHandler_t)(void* context, SilKit_SystemMonitor* systemMonitor,
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
SilKitAPI SilKit_ReturnCode SilKit_SystemMonitor_AddSystemStateHandler(SilKit_SystemMonitor* systemMonitor,
                                                                       void* context,
                                                                       SilKit_SystemStateHandler_t handler,
                                                                       SilKit_HandlerId* outHandlerId);
typedef SilKit_ReturnCode (*SilKit_SystemMonitor_AddSystemStateHandler_t)(SilKit_SystemMonitor* systemMonitor,
                                                                          void* context,
                                                                          SilKit_SystemStateHandler_t handler,
                                                                          SilKit_HandlerId* outHandlerId);

/*! \brief  Remove a \ref SilKit_SystemStateHandler_t by SilKit_HandlerId on this participant
 *
 * \param participant The simulation participant
 * \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
 */
SilKitAPI SilKit_ReturnCode SilKit_SystemMonitor_RemoveSystemStateHandler(SilKit_SystemMonitor* systemMonitor,
                                                                          SilKit_HandlerId handlerId);

typedef SilKit_ReturnCode (*SilKit_SystemMonitor_RemoveSystemStateHandler_t)(SilKit_SystemMonitor* systemMonitor,
                                                                             SilKit_HandlerId handlerId);

typedef void (*SilKit_ParticipantStatusHandler_t)(void* context, SilKit_SystemMonitor* systemMonitor,
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
SilKitAPI SilKit_ReturnCode SilKit_SystemMonitor_AddParticipantStatusHandler(SilKit_SystemMonitor* systemMonitor,
                                                                             void* context,
                                                                             SilKit_ParticipantStatusHandler_t handler,
                                                                             SilKit_HandlerId* outHandlerId);

typedef SilKit_ReturnCode (*SilKit_SystemMonitor_AddParticipantStatusHandler_t)(
    SilKit_SystemMonitor* systemMonitor, void* context, SilKit_ParticipantStatusHandler_t handler,
    SilKit_HandlerId* outHandlerId);

/*! \brief  Remove a \ref SilKit_ParticipantStatusHandler_t by SilKit_HandlerId on this participant
 *
 * \param participant The simulation participant
 * \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
 */
SilKitAPI SilKit_ReturnCode SilKit_SystemMonitor_RemoveParticipantStatusHandler(SilKit_SystemMonitor* systemMonitor,
                                                                                SilKit_HandlerId handlerId);

typedef SilKit_ReturnCode (*SilKit_SystemMonitor_RemoveParticipantStatusHandler_t)(SilKit_SystemMonitor* systemMonitor,
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
SilKitAPI SilKit_ReturnCode SilKit_SystemController_SetWorkflowConfiguration(
    SilKit_SystemController* systemController, const SilKit_WorkflowConfiguration* workflowConfigration);

typedef SilKit_ReturnCode (*SilKit_SystemController_SetWorkflowConfiguration_t)(
    SilKit_SystemController* systemController, const SilKit_WorkflowConfiguration* workflowConfigration);

//!< The LifecycleLifecycle options
typedef struct SilKit_LifecycleConfiguration
{
    SilKit_InterfaceIdentifier interfaceId;
    SilKit_Bool coordinatedStart;
    SilKit_Bool coordinatedStop;
} SilKit_LifecycleConfiguration;

/*! \brief Start the lifecycle with the given parameters without simulation time synchronization.
*  Requires a call to SilKit_LifecycleService_WaitForLifecycleToComplete to retrieve the final state.
* 
* \param participant the instance of the participant.
* \param startConfiguration contains the desired start configuration of the lifecycle.
* 
*/

SilKitAPI SilKit_ReturnCode SilKit_LifecycleService_StartLifecycleNoSyncTime(
    SilKit_LifecycleService* lifecycleService, SilKit_LifecycleConfiguration* startconfiguration);

typedef SilKit_ReturnCode (*SilKit_LifecycleService_StartLifecycleNoSyncTime_t)(
    SilKit_LifecycleService* lifecycleService, SilKit_LifecycleConfiguration* startconfiguration);

/*! \brief Start the lifecycle with the given parameters with simulation time synchronization.
* 
* \param lifecycleService the instance of the lifecycleService.
* \param startConfiguration contains the desired start configuration of the lifecycle.
*/

SilKitAPI SilKit_ReturnCode SilKit_LifecycleService_StartLifecycleWithSyncTime(
    SilKit_LifecycleService* lifecycleService, SilKit_LifecycleConfiguration* startConfiguration);

typedef SilKit_ReturnCode (*SilKit_LifecycleService_StartLifecycleWithSyncTime_t)(
    SilKit_LifecycleService* lifecycleService, SilKit_LifecycleConfiguration* startConfiguration);

/*! \brief Wait for to asynchronous run operation to complete and return the final participant state
 *
 * Blocks until the simulation is shutdown. Prior to this method,
 * \ref SilKit_Participant_StartLifecycle{With,No}Sync has to be called.
 *
 * \param lifecycleService The lifecycle service to wait for completing the asynchronous run operation
 * \param outParticipantState Pointer for storing the final participant state (out parameter)
 */
SilKitAPI SilKit_ReturnCode SilKit_LifecycleService_WaitForLifecycleToComplete(
    SilKit_LifecycleService* lifecycleService, SilKit_ParticipantState* outParticipantState);

typedef SilKit_ReturnCode (*SilKit_LifecycleService_WaitForLifecycleToComplete_t)(
    SilKit_LifecycleService* lifecycleService, SilKit_ParticipantState* outParticipantState);
SILKIT_END_DECLS

#pragma pack(pop)
