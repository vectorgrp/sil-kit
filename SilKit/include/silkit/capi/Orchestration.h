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
#include <stdint.h>
#include <limits.h>
#include "silkit/capi/SilKitMacros.h"
#include "silkit/capi/Types.h"
#include "silkit/capi/InterfaceIdentifiers.h"

#pragma pack(push)
#pragma pack(8)

SILKIT_BEGIN_DECLS

//!< The state of a participant.
typedef int16_t SilKit_ParticipantState;
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
#define SilKit_ParticipantState_Aborting ((SilKit_ParticipantState)120) //!< The aborting state

//!< The state of a system, deduced by states of the required participants.
typedef int16_t SilKit_SystemState;
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
#define SilKit_SystemState_Aborting ((SilKit_SystemState)120) //!< The aborting state


//!< The OperationMode for lifecycle service.
typedef int8_t SilKit_OperationMode;
#define SilKit_OperationMode_Invalid ((SilKit_OperationMode)0) //!< An invalid operation mode
#define SilKit_OperationMode_Coordinated ((SilKit_OperationMode)10) //!< The coordinated operation mode
#define SilKit_OperationMode_Autonomous ((SilKit_OperationMode)20) //!< The autonomous operation mode


typedef uint64_t SilKit_NanosecondsTime; //!< Simulation time

typedef uint64_t SilKit_NanosecondsWallclockTime; //!< Wall clock time since epoch

//!< Details about a status change of a participant.
typedef struct
{
    SilKit_StructHeader structHeader;
    const char* participantName; //!< Name of the participant.
    SilKit_ParticipantState participantState; //!< The new state of the participant.
    const char* enterReason; //!< The reason for the participant to enter the new state.
    SilKit_NanosecondsWallclockTime enterTime; //!< The enter time of the participant.
    SilKit_NanosecondsWallclockTime refreshTime; //!< The refresh time.
} SilKit_ParticipantStatus;

//!< Configuration of the simulation workflow
typedef struct
{
    SilKit_StructHeader structHeader;
    SilKit_StringList*
        requiredParticipantNames; //!< Participants that are waited for when coordinating the simulation start/stop.

} SilKit_WorkflowConfiguration;

typedef struct SilKit_SystemMonitor SilKit_SystemMonitor;
typedef struct SilKit_LifecycleService SilKit_LifecycleService;
typedef struct SilKit_TimeSyncService SilKit_TimeSyncService;

/*! \brief Create a system monitor at this SIL Kit simulation participant.
 * \param outSystemMonitor Pointer that refers to the resulting sytem monitor (out parameter).
 * \param participant The simulation participant at which the system monitor should be created.
 *
 * The object returned must not be deallocated using free()!
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_SystemMonitor_Create(SilKit_SystemMonitor** outSystemMonitor,
                                                        SilKit_Participant* participant);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_SystemMonitor_Create_t)(SilKit_SystemMonitor** outCanController,
                                                           SilKit_Participant* participant);

//!< The LifecycleLifecycle options
typedef struct SilKit_LifecycleConfiguration
{
    SilKit_StructHeader structHeader;
    SilKit_OperationMode operationMode;
} SilKit_LifecycleConfiguration;


/*! \brief Create a lifecycle service at this SIL Kit simulation participant.
 * \param outLifecycleService Pointer that refers to the resulting lifecycle service (out parameter).
 * \param participant The simulation participant at which the lifecycle service should be created.
 * \param startConfiguration The desired start configuration of the lifecycle.
 *
 * The object returned must not be deallocated using free()!
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_Create(SilKit_LifecycleService** outLifecycleService,
                                                           SilKit_Participant* participant,
                                                           const SilKit_LifecycleConfiguration* startConfiguration);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_LifecycleService_Create_t)(SilKit_LifecycleService** outLifecycleService,
                                                              SilKit_Participant* participant,
                                                              const SilKit_LifecycleConfiguration* startConfiguration);

/*! \brief Create a time sync service at this SIL Kit simulation participant.
 * \param outTimeSyncService Pointer that refers to the resulting time sync service (out parameter).
 * \param lifecycleService The lifecyle service at which the time sync service should be created.
 *
 * The object returned must not be deallocated using free()!
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_TimeSyncService_Create(SilKit_TimeSyncService** outTimeSyncService,
                                                          SilKit_LifecycleService* lifecycleService);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_TimeSyncService_Create_t)(SilKit_TimeSyncService** outTimeSyncService,
                                                             SilKit_Participant* lifecycleService);

/*! \brief  The handler to be called on initialization
 *
 * \param context The user provided context passed in \ref SilKit_LifecycleService_SetCommunicationReadyHandler
 * \param participant The simulation participant entering the initialized state
 */
typedef void (SilKitFPTR *SilKit_LifecycleService_CommunicationReadyHandler_t)(void* context,
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
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_SetCommunicationReadyHandler(
    SilKit_LifecycleService* lifecycleService, void* context, SilKit_LifecycleService_CommunicationReadyHandler_t handler);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_LifecycleService_SetCommunicationReadyHandler_t)(
    SilKit_LifecycleService* lifecycleService, void* context, SilKit_LifecycleService_CommunicationReadyHandler_t handler);

/*! \brief Register a callback that is executed once all communication channels between participants
*          with a lifecycle have been set up and are ready for communication.
*
* The handler is called after \ref SystemState::CommunicationReady is reached.
* The API user has to signal the completion of the handler by invoking CompleteCommunicationReadyHandlerAsync().
* Note that \ref SilKit_LifecycleService_CompleteCommunicationReadyHandlerAsync may not be called from within any SilKit_LifecycleService_CommunicationReadyHandler.
* The participant remains in its state until \ref CompleteCommunicationReadyHandlerAsync() is invoked,
* in another thread and then switches to the \ref SilKit_ParticipantState_ReadyToRun.
* 
* \param lifecycleService The lifecycle service receiving the (re-)initialization command
* \param context A user provided context accessible in the handler
* \param handler The handler to be called on initialization
*/ 

SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_SetCommunicationReadyHandlerAsync(
    SilKit_LifecycleService* lifecycleService, void* context, SilKit_LifecycleService_CommunicationReadyHandler_t handler);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_LifecycleService_SetCommunicationReadyHandlerAsync_t)(
    SilKit_LifecycleService* lifecycleService, void* context, SilKit_LifecycleService_CommunicationReadyHandler_t handler);

/*!< Notify that the async \ref SilKit_LifecycleService_CommunicationReadyHandler is completed.
* 
* This method must not be invoked from within a SilKit_LifecycleService_CommunicationReadyHandler.
* \param lifecycleService The lifecycle service receiving the update
* \return \ref SilKit_ReturnCode
*/

SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_CompleteCommunicationReadyHandlerAsync(
    SilKit_LifecycleService* lifecycleService);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_LifecycleService_CompleteCommunicationReadyHandlerAsync_t)(
    SilKit_LifecycleService* lifecycleService);

/*! \brief  This handler is triggered just before the lifecylce service changes to ParticipantState::Running.
 * It is only triggered if the participant does NOT use virtual time synchronization.
 * It does not block other participants from changing to ParticipantState::Running and should only be used for lightweight operations such as starting timers.
 *
 * \param context The user provided context passed in \ref SilKit_LifecycleService_SetCommunicationReadyHandler
 * \param lifecycleService The lifecycle service receiving the update
 */
typedef void (SilKitFPTR *SilKit_LifecycleService_StartingHandler_t)(void* context, SilKit_LifecycleService* lifecycleService);

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

SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_SetStartingHandler(
    SilKit_LifecycleService* lifecycleService, void* context, SilKit_LifecycleService_CommunicationReadyHandler_t handler);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_LifecycleService_SetStartingHandler_t)(
    SilKit_LifecycleService* lifecycleService, void* context, SilKit_LifecycleService_StartingHandler_t handler);

/*! \brief The handler to be called on a simulation stop
 *
 * \param context The user provided context passed in \ref SilKit_LifecycleService_SetStopHandler
 * \param lifecycleService The lifecycle service receiving the stop command
 */
typedef void (SilKitFPTR *SilKit_LifecycleService_StopHandler_t)(void* context, SilKit_LifecycleService* lifecycleService);

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
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_SetStopHandler(SilKit_LifecycleService* lifecycleService,
                                                                   void* context,
                                                                   SilKit_LifecycleService_StopHandler_t handler);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_LifecycleService_SetStopHandler_t)(SilKit_LifecycleService* lifecycleService,
                                                                      void* context,
                                                                      SilKit_LifecycleService_StopHandler_t handler);

/*! \brief The handler to be called on a simulation shutdown
 *
 * \param context The user provided context passed in \ref SilKit_LifecycleService_SetShutdownHandler
 * \param participant The simulation participant receiving the shutdown command
 */
typedef void (SilKitFPTR *SilKit_LifecycleService_ShutdownHandler_t)(void* context, SilKit_LifecycleService* lifecycleService);

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
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_SetShutdownHandler(
    SilKit_LifecycleService* lifecycleService, void* context, SilKit_LifecycleService_ShutdownHandler_t handler);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_LifecycleService_SetShutdownHandler_t)(
    SilKit_LifecycleService* lifecycleService, void* context, SilKit_LifecycleService_ShutdownHandler_t handler);

/*! \brief The handler to be called on a simulation abort.
 *
 * \param context The user provided context passed in \ref SilKit_LifecycleService_SetAbortHandler
 * \param participant The simulation participant receiving the abort command
 */
typedef void (SilKitFPTR *SilKit_LifecycleService_AbortHandler_t)(void* context, SilKit_LifecycleService* lifecycleService, SilKit_ParticipantState lastParticipantState);

/*! \brief Register a callback that is executed on simulation abort.
 *
 * \param lifecycleService The lifecycle service receiving the abort command
 * \param context A user provided context accessible in the handler
 * \param handler The handler to be called
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_SetAbortHandler(
    SilKit_LifecycleService* lifecycleService, void* context, SilKit_LifecycleService_AbortHandler_t handler);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_LifecycleService_SetAbortHandler_t)(
    SilKit_LifecycleService* lifecycleService, void* context, SilKit_LifecycleService_AbortHandler_t handler);

/*! \brief The handler to be called if the simulation task is due
 *
 * \param context The user provided context passed in \ref SilKit_TimeSyncService_SetSimulationStepHandler
 * \param timeSyncService The time sync service
 * \param now The current simulation time
 * \param duration The duration of the simulation step
 */
typedef void (SilKitFPTR *SilKit_TimeSyncService_SimulationStepHandler_t)(void* context, SilKit_TimeSyncService* timeSyncService,
                                                               SilKit_NanosecondsTime now, SilKit_NanosecondsTime duration);
/*! \brief Set the task to be executed with each grant / tick
 *
 * Can be changed at runtime. Execution context depends on the run type.
 *
 * \param participant The simulation participant to start running
 * \param context A user provided context accessible in the handler
 * \param handler The handler to be called if the simulation task is due
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_TimeSyncService_SetSimulationStepHandler(
    SilKit_TimeSyncService* timeSyncService, void* context, SilKit_TimeSyncService_SimulationStepHandler_t handler, SilKit_NanosecondsTime initialStepSize);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_TimeSyncService_SetSimulationStepHandler_t)(
    SilKit_TimeSyncService* timeSyncService, void* context, SilKit_TimeSyncService_SimulationStepHandler_t handler,
    SilKit_NanosecondsTime initialStepSize);

/*! \brief Set the task to be executed with each grant / tick
 *
 * Can be changed at runtime. Execution context depends on the run type.
 *
 * The difference to SetSimulationStepHandler is, that after execution of the simulation task
 * the advance in simulation time will NOT be signaled to other participants.
 * Progress in simulation time (including all other participants) will cease.
 * Instead, SilKit_TimeSyncService_CompleteSimulationStep must be called
 * FROM ANY OTHER THREAD to 'unlock' the thread executing the simulation task, and let it execute again.
 * Thus, a fine grained control over the whole simulation time progress can be achieved
 * by calling CompleteSimulationStep from an application thread.
 * Participants using 'regular' simulation tasks and non-blocking simulation tasks may be freely mixed.
 *
 * \param participant The simulation participant
 * \param context A user provided context accessible in the handler
 * \param handler The handler to be called if the simulation task is due
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_TimeSyncService_SetSimulationStepHandlerAsync(
    SilKit_TimeSyncService* timeSyncService, void* context, SilKit_TimeSyncService_SimulationStepHandler_t handler,
    SilKit_NanosecondsTime initialStepSize);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_TimeSyncService_SetSimulationStepHandlerAsync_t)(
    SilKit_TimeSyncService* timeSyncService, void* context, SilKit_TimeSyncService_SimulationStepHandler_t handler,
    SilKit_NanosecondsTime initialStepSize);

/*! \brief Complete the current step of a non-blocking simulation task.
 *
 * \param timeSyncService The time sync service
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_TimeSyncService_CompleteSimulationStep(SilKit_TimeSyncService* timeSyncService);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_TimeSyncService_CompleteSimulationStep_t)(SilKit_TimeSyncService* timeSyncService);

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
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_Pause(SilKit_LifecycleService* lifecycleService,
                                                          const char* reason);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_LifecycleService_Pause_t)(SilKit_LifecycleService* lifecycleService,
                                                             const char* reason);

/*! \brief Switch back to \ref SilKit_ParticipantState_Running
  * after having paused.
  *
  * Precondition: State() == \ref SilKit_ParticipantState_Paused
  */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_Continue(SilKit_LifecycleService* lifecycleService);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_LifecycleService_Continue_t)(SilKit_LifecycleService* lifecycleService);

/*! \brief Get the current participant state of the participant given by participantName
  */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_SystemMonitor_GetParticipantStatus(SilKit_ParticipantStatus* outParticipantState,
                                                                      SilKit_SystemMonitor* systemMonitor,
                                                                      const char* participantName);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_SystemMonitor_GetParticipantStatus_t)(SilKit_ParticipantStatus* outParticipantState,
                                                                         SilKit_SystemMonitor* systemMonitor,
                                                                         const char* participantName);

//! \brief Get the current ::SystemState
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_SystemMonitor_GetSystemState(SilKit_SystemState* outSystemState,
                                                                SilKit_SystemMonitor* systemMonitor);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_SystemMonitor_GetSystemState_t)(SilKit_SystemState* outSystemState,
                                                                   SilKit_SystemMonitor* systemMonitor);

typedef void (SilKitFPTR *SilKit_SystemStateHandler_t)(void* context, SilKit_SystemMonitor* systemMonitor,
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
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_SystemMonitor_AddSystemStateHandler(SilKit_SystemMonitor* systemMonitor,
                                                                       void* context,
                                                                       SilKit_SystemStateHandler_t handler,
                                                                       SilKit_HandlerId* outHandlerId);
typedef SilKit_ReturnCode (SilKitFPTR *SilKit_SystemMonitor_AddSystemStateHandler_t)(SilKit_SystemMonitor* systemMonitor,
                                                                          void* context,
                                                                          SilKit_SystemStateHandler_t handler,
                                                                          SilKit_HandlerId* outHandlerId);

/*! \brief  Remove a \ref SilKit_SystemStateHandler_t by SilKit_HandlerId on this participant
 *
 * \param participant The simulation participant
 * \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_SystemMonitor_RemoveSystemStateHandler(SilKit_SystemMonitor* systemMonitor,
                                                                          SilKit_HandlerId handlerId);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_SystemMonitor_RemoveSystemStateHandler_t)(SilKit_SystemMonitor* systemMonitor,
                                                                             SilKit_HandlerId handlerId);

typedef void (SilKitFPTR *SilKit_ParticipantStatusHandler_t)(void* context, SilKit_SystemMonitor* systemMonitor,
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
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_SystemMonitor_AddParticipantStatusHandler(SilKit_SystemMonitor* systemMonitor,
                                                                             void* context,
                                                                             SilKit_ParticipantStatusHandler_t handler,
                                                                             SilKit_HandlerId* outHandlerId);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_SystemMonitor_AddParticipantStatusHandler_t)(
    SilKit_SystemMonitor* systemMonitor, void* context, SilKit_ParticipantStatusHandler_t handler,
    SilKit_HandlerId* outHandlerId);

/*! \brief  Remove a \ref SilKit_ParticipantStatusHandler_t by SilKit_HandlerId on this participant
 *
 * \param participant The simulation participant
 * \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_SystemMonitor_RemoveParticipantStatusHandler(SilKit_SystemMonitor* systemMonitor,
                                                                                SilKit_HandlerId handlerId);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_SystemMonitor_RemoveParticipantStatusHandler_t)(SilKit_SystemMonitor* systemMonitor,
                                                                                   SilKit_HandlerId handlerId);

/*! \brief Start the lifecycle.
* 
* \param lifecycleService the instance of the lifecycleService.
*/

SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_StartLifecycle(
    SilKit_LifecycleService* lifecycleService);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_LifecycleService_StartLifecycle_t)(
    SilKit_LifecycleService* lifecycleService);

/*! \brief Wait for to asynchronous run operation to complete and return the final participant state
 *
 * Blocks until the simulation is shutdown. Prior to this method,
 * \ref SilKit_Participant_StartLifecycle{With,No}Sync has to be called.
 *
 * \param lifecycleService The lifecycle service to wait for completing the asynchronous run operation
 * \param outParticipantState Pointer for storing the final participant state (out parameter)
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_WaitForLifecycleToComplete(
    SilKit_LifecycleService* lifecycleService, SilKit_ParticipantState* outParticipantState);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_LifecycleService_WaitForLifecycleToComplete_t)(
    SilKit_LifecycleService* lifecycleService, SilKit_ParticipantState* outParticipantState);
SILKIT_END_DECLS

#pragma pack(pop)
