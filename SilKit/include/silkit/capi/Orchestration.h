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

/*! Simulation time */
typedef uint64_t SilKit_NanosecondsTime;

/*! Wall clock time since epoch */
typedef uint64_t SilKit_NanosecondsWallclockTime;


/*! The state of a participant. */
typedef int16_t SilKit_ParticipantState;

/*! An invalid participant state */
#define SilKit_ParticipantState_Invalid                     ((SilKit_ParticipantState)   0)
/*! The controllers created state */
#define SilKit_ParticipantState_ServicesCreated             ((SilKit_ParticipantState)  10)
/*! The communication initializing state */
#define SilKit_ParticipantState_CommunicationInitializing   ((SilKit_ParticipantState)  20)
/*! The communication initialized state */
#define SilKit_ParticipantState_CommunicationInitialized    ((SilKit_ParticipantState)  30)
/*! The initialized state */
#define SilKit_ParticipantState_ReadyToRun                  ((SilKit_ParticipantState)  40)
/*! The running state */
#define SilKit_ParticipantState_Running                     ((SilKit_ParticipantState)  50)
/*! The paused state */
#define SilKit_ParticipantState_Paused                      ((SilKit_ParticipantState)  60)
/*! The stopping state */
#define SilKit_ParticipantState_Stopping                    ((SilKit_ParticipantState)  70)
/*! The stopped state */
#define SilKit_ParticipantState_Stopped                     ((SilKit_ParticipantState)  80)
/*! The error state */
#define SilKit_ParticipantState_Error                       ((SilKit_ParticipantState)  90)
/*! The shutting down state */
#define SilKit_ParticipantState_ShuttingDown                ((SilKit_ParticipantState) 100)
/*! The shutdown state */
#define SilKit_ParticipantState_Shutdown                    ((SilKit_ParticipantState) 110)
/*! The aborting state */
#define SilKit_ParticipantState_Aborting                    ((SilKit_ParticipantState) 120)


/*! The state of a system, deduced by states of the required participants. */
typedef int16_t SilKit_SystemState;

/*! An invalid participant state */
#define SilKit_SystemState_Invalid                      ((SilKit_SystemState)   0)
/*! The controllers created state */
#define SilKit_SystemState_ServicesCreated              ((SilKit_SystemState)  10)
/*! The communication initializing state */
#define SilKit_SystemState_CommunicationInitializing    ((SilKit_SystemState)  20)
/*! The communication initialized state */
#define SilKit_SystemState_CommunicationInitialized     ((SilKit_SystemState)  30)
/*! The initialized state */
#define SilKit_SystemState_ReadyToRun                   ((SilKit_SystemState)  40)
/*! The running state */
#define SilKit_SystemState_Running                      ((SilKit_SystemState)  50)
/*! The paused state */
#define SilKit_SystemState_Paused                       ((SilKit_SystemState)  60)
/*! The stopping state */
#define SilKit_SystemState_Stopping                     ((SilKit_SystemState)  70)
/*! The stopped state */
#define SilKit_SystemState_Stopped                      ((SilKit_SystemState)  80)
/*! The error state */
#define SilKit_SystemState_Error                        ((SilKit_SystemState)  90)
/*! The shutting down state */
#define SilKit_SystemState_ShuttingDown                 ((SilKit_SystemState) 100)
/*! The shutdown state */
#define SilKit_SystemState_Shutdown                     ((SilKit_SystemState) 110)
/*! The aborting state */
#define SilKit_SystemState_Aborting                     ((SilKit_SystemState) 120)


/*! The OperationMode for lifecycle service. */
typedef int8_t SilKit_OperationMode;

/*! An invalid operation mode */
#define SilKit_OperationMode_Invalid        ((SilKit_OperationMode)  0)
/*! The coordinated operation mode */
#define SilKit_OperationMode_Coordinated    ((SilKit_OperationMode) 10)
/*! The autonomous operation mode */
#define SilKit_OperationMode_Autonomous     ((SilKit_OperationMode) 20)


/*! Details about a status change of a participant. */
typedef struct
{
    SilKit_StructHeader structHeader;
    const char* participantName; /*!< Name of the participant. */
    SilKit_ParticipantState participantState; /*!< The new state of the participant. */
    const char* enterReason; /*!< The reason for the participant to enter the new state. */
    SilKit_NanosecondsWallclockTime enterTime; /*!< The enter time of the participant. */
    SilKit_NanosecondsWallclockTime refreshTime; /*!< The refresh time. */
} SilKit_ParticipantStatus;


/*! Configuration of the simulation workflow */
typedef struct
{
    SilKit_StructHeader structHeader;
    /*! Participants that are waited for when coordinating the simulation start/stop. */
    SilKit_StringList* requiredParticipantNames;
} SilKit_WorkflowConfiguration;


/*! The LifecycleLifecycle options */
typedef struct SilKit_LifecycleConfiguration
{
    SilKit_StructHeader structHeader;
    SilKit_OperationMode operationMode;
} SilKit_LifecycleConfiguration;


typedef struct SilKit_SystemMonitor SilKit_SystemMonitor;
typedef struct SilKit_LifecycleService SilKit_LifecycleService;
typedef struct SilKit_TimeSyncService SilKit_TimeSyncService;


/*
 *
 * Lifecycle Service
 *
 */


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

/*! \brief  The handler to be called on initialization
 *
 * \param context The user provided context passed in \ref SilKit_LifecycleService_SetCommunicationReadyHandler
 * \param lifecycleService The lifecycle service receiving the update.
 */
typedef void(SilKitFPTR* SilKit_LifecycleService_CommunicationReadyHandler_t)(
    void* context, SilKit_LifecycleService* lifecycleService);

/*! \brief Register a callback that is executed once communication with controllers is possible.
 *
 * The handler is called after \ref SilKit_ParticipantState_CommunicationInitialized is reached.
 * This handler is invoked on the SilKit I/O worker thread, and receiving messages during the handler invocation is not possible.
 * For an asynchronous invocation, see \ref SilKit_LifecycleService_SetCommunicationReadyHandlerAsync and 
 * \ref SilKit_LifecycleService_CompleteCommunicationReadyHandlerAsync.
 * After the handler has been processed, the participant switches to the \ref SilKit_ParticipantState_ReadyToRun state.
 * 
 * \param lifecycleService The lifecycle service that switched to the CommunicationInitialized participant state.
 * \param context A user provided context accessible in the handler
 * \param handler The handler to be called on initialization
 */
SilKitAPI SilKit_ReturnCode SilKitCALL
SilKit_LifecycleService_SetCommunicationReadyHandler(SilKit_LifecycleService* lifecycleService, void* context,
                                                     SilKit_LifecycleService_CommunicationReadyHandler_t handler);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_LifecycleService_SetCommunicationReadyHandler_t)(
    SilKit_LifecycleService* lifecycleService, void* context,
    SilKit_LifecycleService_CommunicationReadyHandler_t handler);

/*! \brief Register a callback that is executed once all communication channels between participants
 *          with a lifecycle have been set up and are ready for communication.
 *
 * The handler is called after \ref SilKit_ParticipantState_CommunicationInitialized is reached.
 * The API user has to signal the completion of the handler by invoking
 * \ref SilKit_LifecycleService_CompleteCommunicationReadyHandlerAsync.
 * Note that \ref SilKit_LifecycleService_CompleteCommunicationReadyHandlerAsync may not be called from within any SilKit_LifecycleService_CommunicationReadyHandler_t.
 * The participant remains in its state until \ref SilKit_LifecycleService_CompleteCommunicationReadyHandlerAsync is
 * invoked and then switches to the \ref SilKit_ParticipantState_ReadyToRun.
 *
 * \param lifecycleService The lifecycle service receiving the (re-)initialization command
 * \param context A user provided context accessible in the handler
 * \param handler The handler to be called on initialization
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_SetCommunicationReadyHandlerAsync(
    SilKit_LifecycleService* lifecycleService, void* context, SilKit_LifecycleService_CommunicationReadyHandler_t handler);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_LifecycleService_SetCommunicationReadyHandlerAsync_t)(
    SilKit_LifecycleService* lifecycleService, void* context, SilKit_LifecycleService_CommunicationReadyHandler_t handler);

/*! \brief Notify that the async \ref SilKit_LifecycleService_CommunicationReadyHandler_t is completed.
 *
 * This method must not be invoked from within a SilKit_LifecycleService_CommunicationReadyHandler_t.
 * \param lifecycleService The lifecycle service receiving the update.
 * \return \ref SilKit_ReturnCode
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_CompleteCommunicationReadyHandlerAsync(
    SilKit_LifecycleService* lifecycleService);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_LifecycleService_CompleteCommunicationReadyHandlerAsync_t)(
    SilKit_LifecycleService* lifecycleService);

/*! \brief  This handler is triggered just before the lifecycle service changes to SilKit_ParticipantState_Running.
 * It is only triggered if the participant does NOT use virtual time synchronization.
 * It does not block other participants from changing to SilKit_ParticipantState_Running and should only be used for lightweight operations such as starting timers.
 *
 * \param context The user provided context passed in \ref SilKit_LifecycleService_SetCommunicationReadyHandler.
 * \param lifecycleService The lifecycle service receiving the update.
 */
typedef void (SilKitFPTR *SilKit_LifecycleService_StartingHandler_t)(void* context, SilKit_LifecycleService* lifecycleService);

/*! \brief (Asynchronous participants only) Register a callback that is executed once directly before the participant enters SilKit_ParticipantState_Running.
 *
 * This handler is triggered just before the participant changes to
 * \ref SilKit::Services::Orchestration::ParticipantState::Running.
 * It is only triggered if the participant does NOT use virtual time synchronization.
 * It does not block other participants from changing to ParticipantState::Running and should only be used for lightweight operations such as starting timers.
 * It is executed in the context of an internal thread that received the command.
 * After the handler has been processed, the participant
 * switches to the \ref SilKit_ParticipantState_Running state.
 *
 * \param lifecycleService The lifecycle service that triggers the StartingHandler.
 * \param context A user provided context accessible in the handler.
 * \param handler The handler to be called when starting.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_SetStartingHandler(
    SilKit_LifecycleService* lifecycleService, void* context, SilKit_LifecycleService_StartingHandler_t handler);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_LifecycleService_SetStartingHandler_t)(
    SilKit_LifecycleService* lifecycleService, void* context, SilKit_LifecycleService_StartingHandler_t handler);

/*! \brief The handler to be called on a simulation stop
 *
 * \param context The user provided context passed in \ref SilKit_LifecycleService_SetStopHandler
 * \param lifecycleService The lifecycle service receiving the stop command
 */
typedef void (SilKitFPTR *SilKit_LifecycleService_StopHandler_t)(void* context, SilKit_LifecycleService* lifecycleService);

/*! \brief Register a callback that is executed on simulation stop.
 *
 * The handler is called when the participant has entered the
 * \ref SilKit_ParticipantState_Stopping state.
 * It is executed in the context of an internal
 * thread that received the command. After the handler has been
 * processed, the participant switches to the
 * \ref SilKit_ParticipantState_Stopped state.
 *
 * Throwing an error inside the handler will cause a call to
 * ReportError().
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
 * \param lifecycleService The lifecycleService receiving the shutdown
 */
typedef void (SilKitFPTR *SilKit_LifecycleService_ShutdownHandler_t)(void* context, SilKit_LifecycleService* lifecycleService);

/*! \brief Register a callback that is executed on simulation shutdown.
 *
 * The handler is called when the participant is entering the
 * \ref SilKit_ParticipantState_ShuttingDown state.
 * It is executed in the context of the middleware
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
 * \param lifecycleService The lifecycle service receiving the abort command
 * \param lastParticipantState The last participant state before the simulation was aborted.
 */
typedef void(SilKitFPTR* SilKit_LifecycleService_AbortHandler_t)(void* context,
                                                                 SilKit_LifecycleService* lifecycleService,
                                                                 SilKit_ParticipantState lastParticipantState);

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

/*! \brief Start the lifecycle.
 *
 * \param lifecycleService The instance of the lifecycleService.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_StartLifecycle(
    SilKit_LifecycleService* lifecycleService);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_LifecycleService_StartLifecycle_t)(
    SilKit_LifecycleService* lifecycleService);

/*! \brief Wait for to asynchronous run operation to complete and return the final participant state
 *
 * Blocks until the simulation is shutdown. Prior to this method,
 * \ref SilKit_LifecycleService_StartLifecycle has to be called.
 *
 * \param lifecycleService The lifecycle service to wait for completing the asynchronous run operation.
 * \param outParticipantState Pointer for storing the final participant state (out parameter).
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_WaitForLifecycleToComplete(
    SilKit_LifecycleService* lifecycleService, SilKit_ParticipantState* outParticipantState);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_LifecycleService_WaitForLifecycleToComplete_t)(
    SilKit_LifecycleService* lifecycleService, SilKit_ParticipantState* outParticipantState);

/*! \brief Abort current simulation run due to an error.
 *
 * Switch to the \ref SilKit_ParticipantState_Error state and
 * report the error message in the SIL Kit system.
 *
 * \param lifecycleService The lifecycle service of the simulation.
 * \param reason A string describing the error.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_ReportError(SilKit_LifecycleService* lifecycleService,
                                                                           const char* reason);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_LifecycleService_ReportError_t)(SilKit_LifecycleService* lifecycleService,
                                                                              const char* reason);

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

/*! \brief Stop execution of the participant.
 *
 * Allows the participant to exit the RunAsync loop, e.g., if it
 * is unable to further progress its simulation.
 *
 * Calls the StopHandler and then switches to the
 * \ref SilKit_ParticipantState_Stopped state.
 *
 * NB: In general, Stop should not be called by the participants
 * as the end of simulation is governed by the central execution
 * controller. This method should only be used if the client
 * cannot participate in the system simulation anymore.
 *
 * Precondition: State() == \ref SilKit_ParticipantState_Running
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_Stop(SilKit_LifecycleService* lifecycleService, const char* reason);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_LifecycleService_Stop_t)(SilKit_LifecycleService* lifecycleService, const char* reason);

/*! \brief Get the current participant state.
 *
 * @param outParticipantState The current participant state will be written to the pointee.
 * @param lifecycleService The lifecycle service obtained by \ref SilKit_LifecycleService_Create.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_State(SilKit_ParticipantState* outParticipantState,
                                                                     SilKit_LifecycleService* lifecycleService);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_LifecycleService_State_t)(SilKit_ParticipantState* outParticipantState,
                                                                       SilKit_LifecycleService* lifecycleService);

/*! \brief Get the current participant status.
 *
 * @param outParticipantStatus The current participant status will be written to the pointee.
 * @param lifecycleService The lifecycle service obtained by \ref SilKit_LifecycleService_Create.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_Status(SilKit_ParticipantStatus* outParticipantStatus,
                                                                      SilKit_LifecycleService* lifecycleService);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_LifecycleService_Status_t)(SilKit_ParticipantStatus* outParticipantStatus,
                                                                         SilKit_LifecycleService* lifecycleService);

/*
 *
 * Time-Sync Service
 *
 */


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
 * \param timeSyncService The time-sync service obtained via \ref SilKit_TimeSyncService_Create.
 * \param context A user provided context accessible in the handler
 * \param handler The handler to be called if the simulation task is due
 * \param initialStepSize The initial size of the simulation step of this participant in nanoseconds
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
 * \param timeSyncService The time-sync service obtained via \ref SilKit_TimeSyncService_Create.
 * \param context A user provided context accessible in the handler
 * \param handler The handler to be called if the simulation task is due
 * \param initialStepSize The initial size of the simulation step of this participant in nanoseconds
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

/*! \brief Get the current simulation time
 *
 * \param timeSyncService The time sync service obtained via \ref SilKit_TimeSyncService_Create.
 * \param outNanosecondsTime The simulation time in nanoseconds.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_TimeSyncService_Now(SilKit_TimeSyncService* timeSyncService,
    SilKit_NanosecondsTime* outNanosecondsTime);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_TimeSyncService_Now_t)(SilKit_TimeSyncService* timeSyncService,
    SilKit_NanosecondsTime* outNanosecondsTime);


/*
 *
 * System Monitor
 *
 */


/*! \brief Create a system monitor at this SIL Kit simulation participant.
 * \param outSystemMonitor Pointer that refers to the resulting sytem monitor (out parameter).
 * \param participant The simulation participant at which the system monitor should be created.
 *
 * The object returned must not be deallocated using free()!
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_SystemMonitor_Create(SilKit_SystemMonitor** outSystemMonitor,
    SilKit_Participant* participant);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_SystemMonitor_Create_t)(SilKit_SystemMonitor** outSystemMonitor,
    SilKit_Participant* participant);

/*! \brief Get the current participant state of the participant given by participantName
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_SystemMonitor_GetParticipantStatus(SilKit_ParticipantStatus* outParticipantState,
                                                                      SilKit_SystemMonitor* systemMonitor,
                                                                      const char* participantName);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_SystemMonitor_GetParticipantStatus_t)(SilKit_ParticipantStatus* outParticipantState,
                                                                         SilKit_SystemMonitor* systemMonitor,
                                                                         const char* participantName);

/*! \brief Get the current \ref SilKit_SystemState */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_SystemMonitor_GetSystemState(SilKit_SystemState* outSystemState,
                                                                SilKit_SystemMonitor* systemMonitor);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_SystemMonitor_GetSystemState_t)(SilKit_SystemState* outSystemState,
                                                                   SilKit_SystemMonitor* systemMonitor);

typedef void (SilKitFPTR *SilKit_SystemStateHandler_t)(void* context, SilKit_SystemMonitor* systemMonitor,
                                            SilKit_SystemState state);

/*! \brief Register a callback for system state changes
 *
 * If the current SystemState is not \ref SilKit_SystemState_Invalid,
 * the handler will be called immediately.
 *
 * \param systemMonitor The system monitor obtained via \ref SilKit_SystemMonitor_Create.
 * \param context The user context pointer made available to the handler.
 * \param handler The handler to be called to be called when the \ref SilKit_SystemState changes.
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
 * \param systemMonitor The system monitor obtained via \ref SilKit_SystemMonitor_Create.
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
 * \param systemMonitor The system monitor obtained via \ref SilKit_SystemMonitor_Create.
 * \param context The user context pointer made available to the handler.
 * \param handler The handler to be called to be called when the participant status changes.
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
 * \param systemMonitor The system monitor obtained via \ref SilKit_SystemMonitor_Create.
 * \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_SystemMonitor_RemoveParticipantStatusHandler(SilKit_SystemMonitor* systemMonitor,
                                                                                SilKit_HandlerId handlerId);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_SystemMonitor_RemoveParticipantStatusHandler_t)(SilKit_SystemMonitor* systemMonitor,
                                                                                   SilKit_HandlerId handlerId);

/*! \brief Information about a participant connection in the \ref SilKit_SystemMonitor_ParticipantConnectedHandler_t. */
typedef struct SilKit_ParticipantConnectionInformation
{
    SilKit_StructHeader structHeader;
    /*! \brief Name of the remote participant. */
    const char* participantName;
} SilKit_ParticipantConnectionInformation;

/*! Callback type to indicate that a participant has been connected.
 * Cf., \ref SilKit_SystemMonitor_SetParticipantConnectedHandler
 */
typedef void (SilKitFPTR *SilKit_SystemMonitor_ParticipantConnectedHandler_t)(
    void* context, SilKit_SystemMonitor* systemMonitor,
    const SilKit_ParticipantConnectionInformation* participantConnectionInformation);

/*! \brief Set a callback for participants being connected.
 *
 * @param systemMonitor The system monitor obtained via \ref SilKit_SystemMonitor_Create.
 * @param context The user context pointer made available to the handler.
 * @param handler The handler to be called to be called when a participant has been connected.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_SystemMonitor_SetParticipantConnectedHandler(
    SilKit_SystemMonitor* systemMonitor, void* context, SilKit_SystemMonitor_ParticipantConnectedHandler_t handler);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_SystemMonitor_SetParticipantConnectedHandler_t)(
    SilKit_SystemMonitor* systemMonitor, void* context, SilKit_SystemMonitor_ParticipantConnectedHandler_t handler);

/*! Callback type to indicate that a participant has been disconnected.
 * Cf., \ref SilKit_SystemMonitor_SetParticipantDisconnectedHandler
 */
typedef void (SilKitFPTR *SilKit_SystemMonitor_ParticipantDisconnectedHandler_t)(
    void* context, SilKit_SystemMonitor* systemMonitor,
    const SilKit_ParticipantConnectionInformation* participantConnectionInformation);

/*! \brief Set a callback for participants being disconnected.
 *
 * @param systemMonitor The system monitor obtained via \ref SilKit_SystemMonitor_Create.
 * @param context The user context pointer made available to the handler.
 * @param handler The handler to be called to be called when a participant has been disconnected.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_SystemMonitor_SetParticipantDisconnectedHandler(
    SilKit_SystemMonitor* systemMonitor, void* context, SilKit_SystemMonitor_ParticipantDisconnectedHandler_t handler);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_SystemMonitor_SetParticipantDisconnectedHandler_t)(
    SilKit_SystemMonitor* systemMonitor, void* context, SilKit_SystemMonitor_ParticipantDisconnectedHandler_t handler);

/*! \brief Check if a participant identified by the participantName is present.
 *
 * @param systemMonitor The system monitor obtained via \ref SilKit_SystemMonitor_Create.
 * @param participantName The name of the participant for which presence is queried.
 * @param out \ref SilKit_True is written to the pointee if the participant is present, otherwise \ref SilKit_False.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_SystemMonitor_IsParticipantConnected(SilKit_SystemMonitor* systemMonitor,
                                                                                   const char* participantName,
                                                                                   SilKit_Bool* out);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_SystemMonitor_IsParticipantConnected_t)(
    SilKit_SystemMonitor* systemMonitor, const char* participantName, SilKit_Bool* out);

/*! \brief Return the experimental ISystemController at a given SIL Kit participant.
 *
 * @warning This function is not part of the stable API and ABI of the SIL Kit. It may be removed at any time without
 *          prior notice.
 *
 * @param outSystemController Pointer that refers to the resulting system controller (out parameter).
 * @param participant The participant instance for which the system controller is created
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Experimental_SystemController_Create(
    SilKit_Experimental_SystemController** outSystemController, SilKit_Participant* participant);

typedef SilKit_ReturnCode(SilKitFPTR * SilKit_Experimental_SystemController_Create_t)(
    SilKit_Experimental_SystemController** outSystemController, SilKit_Participant* participant);

/*! \brief Sends a SilKit::Services::Orchestration::SystemCommand::Kind::AbortSimulation to all participants
 *
 *  The abort simulation command signals all participants to terminate their lifecycle, regardless of their current
 *  state.
 *
 *  The command is allowed at any time.
 *
 * @warning This function is not part of the stable API and ABI of the SIL Kit. It may be removed at any time without
 *          prior notice.
 *
 * @param systemController The system controller obtained via \ref SilKit_Experimental_SystemController_Create.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Experimental_SystemController_AbortSimulation(
    SilKit_Experimental_SystemController* systemController);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_Experimental_SystemController_AbortSimulation_t)(
    SilKit_Experimental_SystemController* systemController);

/*! \brief Configures details of the simulation workflow regarding lifecycle and participant coordination.
 *
 * Only the required participant defined in the \ref SilKit_WorkflowConfiguration are taken
 * into account to define the  system state. Further, the simulation time propagation also relies on the required
 * participants.
 *
 * The \ref SilKit_WorkflowConfiguration is distributed to other participants, so it must only be set once by a single
 * member of the simulation.
 *
 * @warning This function is not part of the stable API and ABI of the SIL Kit. It may be removed at any time without
 *          prior notice.
 *
 * @param systemController The system controller obtained via \ref SilKit_Experimental_SystemController_Create.
 * @param workflowConfiguration The desired configuration, currently containing a list of required participants
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Experimental_SystemController_SetWorkflowConfiguration(
    SilKit_Experimental_SystemController* systemController,
    const SilKit_WorkflowConfiguration* workflowConfiguration);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_Experimental_SystemController_SetWorkflowConfiguration_t)(
    SilKit_Experimental_SystemController* systemController,
    const SilKit_WorkflowConfiguration* workflowConfiguration);

SILKIT_END_DECLS

#pragma pack(pop)
