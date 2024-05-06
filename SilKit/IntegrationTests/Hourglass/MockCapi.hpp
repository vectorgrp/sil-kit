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

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "silkit/capi/SilKit.h"

#include "silkit/util/Span.hpp"

#include <algorithm>

namespace SilKitHourglassTests {

class MockCapi
{
public:
    void SetUpGlobalCapi();

    void TearDownGlobalCapi();

public:
    // Common

    MOCK_METHOD(SilKit_ReturnCode, SilKit_ReturnCodeToString, (const char** outString, SilKit_ReturnCode returnCode));

    MOCK_METHOD(const char*, SilKit_GetLastErrorString, ());

    // CanController

    MOCK_METHOD(SilKit_ReturnCode, SilKit_CanController_Create,
                (SilKit_CanController * *outCanController, SilKit_Participant* participant, const char* name,
                 const char* network));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_CanController_Start, (SilKit_CanController * controller));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_CanController_Stop, (SilKit_CanController * controller));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_CanController_Reset, (SilKit_CanController * controller));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_CanController_Sleep, (SilKit_CanController * controller));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_CanController_SendFrame,
                (SilKit_CanController * controller, SilKit_CanFrame* frame, void* userContext));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_CanController_SetBaudRate,
                (SilKit_CanController * controller, uint32_t rate, uint32_t fdRate, uint32_t xlRate));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_CanController_AddFrameTransmitHandler,
                (SilKit_CanController * controller, void* context, SilKit_CanFrameTransmitHandler_t handler,
                 SilKit_CanTransmitStatus statusMask, SilKit_HandlerId* outHandlerId));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_CanController_RemoveFrameTransmitHandler,
                (SilKit_CanController * controller, SilKit_HandlerId handlerId));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_CanController_AddFrameHandler,
                (SilKit_CanController * controller, void* context, SilKit_CanFrameHandler_t handler,
                 SilKit_Direction directionMask, SilKit_HandlerId* outHandlerId));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_CanController_RemoveFrameHandler,
                (SilKit_CanController * controller, SilKit_HandlerId handlerId));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_CanController_AddStateChangeHandler,
                (SilKit_CanController * controller, void* context, SilKit_CanStateChangeHandler_t handler,
                 SilKit_HandlerId* outHandlerId));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_CanController_RemoveStateChangeHandler,
                (SilKit_CanController * controller, SilKit_HandlerId handlerId));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_CanController_AddErrorStateChangeHandler,
                (SilKit_CanController * controller, void* context, SilKit_CanErrorStateChangeHandler_t handler,
                 SilKit_HandlerId* outHandlerId));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_CanController_RemoveErrorStateChangeHandler,
                (SilKit_CanController * controller, SilKit_HandlerId handlerId));

    // EthernetController

    MOCK_METHOD(SilKit_ReturnCode, SilKit_EthernetController_Create,
                (SilKit_EthernetController * *outController, SilKit_Participant* participant, const char* name,
                 const char* network));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_EthernetController_Activate, (SilKit_EthernetController * controller));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_EthernetController_Deactivate, (SilKit_EthernetController * controller));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_EthernetController_AddFrameHandler,
                (SilKit_EthernetController * controller, void* context, SilKit_EthernetFrameHandler_t handler,
                 SilKit_Direction directionMask, SilKit_HandlerId* outHandlerId));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_EthernetController_RemoveFrameHandler,
                (SilKit_EthernetController * controller, SilKit_HandlerId handlerId));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_EthernetController_AddFrameTransmitHandler,
                (SilKit_EthernetController * controller, void* context, SilKit_EthernetFrameTransmitHandler_t handler,
                 SilKit_EthernetTransmitStatus transmitStatusMask, SilKit_HandlerId* outHandlerId));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_EthernetController_RemoveFrameTransmitHandler,
                (SilKit_EthernetController * controller, SilKit_HandlerId handlerId));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_EthernetController_AddStateChangeHandler,
                (SilKit_EthernetController * controller, void* context, SilKit_EthernetStateChangeHandler_t handler,
                 SilKit_HandlerId* outHandlerId));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_EthernetController_RemoveStateChangeHandler,
                (SilKit_EthernetController * controller, SilKit_HandlerId handlerId));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_EthernetController_AddBitrateChangeHandler,
                (SilKit_EthernetController * controller, void* context, SilKit_EthernetBitrateChangeHandler_t handler,
                 SilKit_HandlerId* outHandlerId));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_EthernetController_RemoveBitrateChangeHandler,
                (SilKit_EthernetController * controller, SilKit_HandlerId handlerId));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_EthernetController_SendFrame,
                (SilKit_EthernetController * controller, SilKit_EthernetFrame* frame, void* userContext));

    // FlexrayController

    MOCK_METHOD(SilKit_ReturnCode, SilKit_FlexrayController_Create,
                (SilKit_FlexrayController * *outFlexrayController, SilKit_Participant* participant, const char* name,
                 const char* network));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_FlexrayController_Configure,
                (SilKit_FlexrayController * controller, const SilKit_FlexrayControllerConfig* config));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_FlexrayController_ReconfigureTxBuffer,
                (SilKit_FlexrayController * controller, uint16_t txBufferIdx,
                 const SilKit_FlexrayTxBufferConfig* config));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_FlexrayController_UpdateTxBuffer,
                (SilKit_FlexrayController * controller, const SilKit_FlexrayTxBufferUpdate* update));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_FlexrayController_ExecuteCmd,
                (SilKit_FlexrayController * controller, SilKit_FlexrayChiCommand cmd));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_FlexrayController_AddFrameHandler,
                (SilKit_FlexrayController * controller, void* context, SilKit_FlexrayFrameHandler_t handler,
                 SilKit_HandlerId* outHandlerId));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_FlexrayController_RemoveFrameHandler,
                (SilKit_FlexrayController * controller, SilKit_HandlerId handlerId));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_FlexrayController_AddFrameTransmitHandler,
                (SilKit_FlexrayController * controller, void* context, SilKit_FlexrayFrameTransmitHandler_t handler,
                 SilKit_HandlerId* outHandlerId));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_FlexrayController_RemoveFrameTransmitHandler,
                (SilKit_FlexrayController * controller, SilKit_HandlerId handlerId));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_FlexrayController_AddWakeupHandler,
                (SilKit_FlexrayController * controller, void* context, SilKit_FlexrayWakeupHandler_t handler,
                 SilKit_HandlerId* outHandlerId));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_FlexrayController_RemoveWakeupHandler,
                (SilKit_FlexrayController * controller, SilKit_HandlerId handlerId));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_FlexrayController_AddPocStatusHandler,
                (SilKit_FlexrayController * controller, void* context, SilKit_FlexrayPocStatusHandler_t handler,
                 SilKit_HandlerId* outHandlerId));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_FlexrayController_RemovePocStatusHandler,
                (SilKit_FlexrayController * controller, SilKit_HandlerId handlerId));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_FlexrayController_AddSymbolHandler,
                (SilKit_FlexrayController * controller, void* context, SilKit_FlexraySymbolHandler_t handler,
                 SilKit_HandlerId* outHandlerId));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_FlexrayController_RemoveSymbolHandler,
                (SilKit_FlexrayController * controller, SilKit_HandlerId handlerId));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_FlexrayController_AddSymbolTransmitHandler,
                (SilKit_FlexrayController * controller, void* context, SilKit_FlexraySymbolTransmitHandler_t handler,
                 SilKit_HandlerId* outHandlerId));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_FlexrayController_RemoveSymbolTransmitHandler,
                (SilKit_FlexrayController * controller, SilKit_HandlerId handlerId));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_FlexrayController_AddCycleStartHandler,
                (SilKit_FlexrayController * controller, void* context, SilKit_FlexrayCycleStartHandler_t handler,
                 SilKit_HandlerId* outHandlerId));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_FlexrayController_RemoveCycleStartHandler,
                (SilKit_FlexrayController * controller, SilKit_HandlerId handlerId));

    // LinController

    MOCK_METHOD(SilKit_ReturnCode, SilKit_LinController_Create,
                (SilKit_LinController * *outLinController, SilKit_Participant* participant, const char* name,
                 const char* network));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_LinController_Init,
                (SilKit_LinController * controller, const SilKit_LinControllerConfig* config));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_LinController_SetFrameResponse,
                (SilKit_LinController * controller, const SilKit_LinFrameResponse* response));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_LinController_Status,
                (SilKit_LinController * controller, SilKit_LinControllerStatus* outStatus));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_LinController_SendFrame,
                (SilKit_LinController * controller, const SilKit_LinFrame* frame,
                 SilKit_LinFrameResponseType responseType));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_LinController_SendFrameHeader,
                (SilKit_LinController * controller, SilKit_LinId linId));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_LinController_UpdateTxBuffer,
                (SilKit_LinController * controller, const SilKit_LinFrame* frame));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_LinController_GoToSleep, (SilKit_LinController * controller));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_LinController_GoToSleepInternal, (SilKit_LinController * controller));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_LinController_Wakeup, (SilKit_LinController * controller));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_LinController_WakeupInternal, (SilKit_LinController * controller));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_Experimental_LinController_GetSlaveConfiguration,
                (SilKit_LinController * controller,
                 SilKit_Experimental_LinSlaveConfiguration* outLinSlaveConfiguration));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_LinController_AddFrameStatusHandler,
                (SilKit_LinController * controller, void* context, SilKit_LinFrameStatusHandler_t handler,
                 SilKit_HandlerId* outHandlerId));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_LinController_RemoveFrameStatusHandler,
                (SilKit_LinController * controller, SilKit_HandlerId handlerId));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_LinController_AddGoToSleepHandler,
                (SilKit_LinController * controller, void* context, SilKit_LinGoToSleepHandler_t handler,
                 SilKit_HandlerId* outHandlerId));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_LinController_RemoveGoToSleepHandler,
                (SilKit_LinController * controller, SilKit_HandlerId handlerId));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_LinController_AddWakeupHandler,
                (SilKit_LinController * controller, void* context, SilKit_LinWakeupHandler_t handler,
                 SilKit_HandlerId* outHandlerId));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_LinController_RemoveWakeupHandler,
                (SilKit_LinController * controller, SilKit_HandlerId handlerId));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_Experimental_LinController_AddLinSlaveConfigurationHandler,
                (SilKit_LinController * controller, void* context,
                 SilKit_Experimental_LinSlaveConfigurationHandler_t handler, SilKit_HandlerId* outHandlerId));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_Experimental_LinController_RemoveLinSlaveConfigurationHandler,
                (SilKit_LinController * controller, SilKit_HandlerId handlerId));

    // Lin RespondeDynamic

    MOCK_METHOD(SilKit_ReturnCode, SilKit_Experimental_LinController_InitDynamic,
                (SilKit_LinController * controller, const SilKit_Experimental_LinControllerDynamicConfig* config));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_Experimental_LinController_SendDynamicResponse,
                (SilKit_LinController * controller, const SilKit_LinFrame* frame));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_Experimental_LinController_AddFrameHeaderHandler,
                (SilKit_LinController * controller, void* context, SilKit_Experimental_LinFrameHeaderHandler_t handler,
                 SilKit_HandlerId* outHandlerId));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_Experimental_LinController_RemoveFrameHeaderHandler,
                (SilKit_LinController * controller, SilKit_HandlerId handlerId));

    // LifecycleService

    MOCK_METHOD(SilKit_ReturnCode, SilKit_LifecycleService_Create,
                (SilKit_LifecycleService * *outLifecycleService, SilKit_Participant* participant,
                 const SilKit_LifecycleConfiguration* startConfiguration));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_LifecycleService_SetCommunicationReadyHandler,
                (SilKit_LifecycleService * lifecycleService, void* context,
                 SilKit_LifecycleService_CommunicationReadyHandler_t handler));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_LifecycleService_SetCommunicationReadyHandlerAsync,
                (SilKit_LifecycleService * lifecycleService, void* context,
                 SilKit_LifecycleService_CommunicationReadyHandler_t handler));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_LifecycleService_CompleteCommunicationReadyHandlerAsync,
                (SilKit_LifecycleService * lifecycleService));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_LifecycleService_SetStartingHandler,
                (SilKit_LifecycleService * lifecycleService, void* context,
                 SilKit_LifecycleService_StartingHandler_t handler));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_LifecycleService_SetStopHandler,
                (SilKit_LifecycleService * lifecycleService, void* context,
                 SilKit_LifecycleService_StopHandler_t handler));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_LifecycleService_SetShutdownHandler,
                (SilKit_LifecycleService * lifecycleService, void* context,
                 SilKit_LifecycleService_ShutdownHandler_t handler));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_LifecycleService_SetAbortHandler,
                (SilKit_LifecycleService * lifecycleService, void* context,
                 SilKit_LifecycleService_AbortHandler_t handler));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_LifecycleService_StartLifecycle,
                (SilKit_LifecycleService * lifecycleService));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_LifecycleService_WaitForLifecycleToComplete,
                (SilKit_LifecycleService * lifecycleService, SilKit_ParticipantState* outParticipantState));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_LifecycleService_ReportError,
                (SilKit_LifecycleService * lifecycleService, const char* reason));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_LifecycleService_Pause,
                (SilKit_LifecycleService * lifecycleService, const char* reason));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_LifecycleService_Continue, (SilKit_LifecycleService * lifecycleService));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_LifecycleService_Stop,
                (SilKit_LifecycleService * lifecycleService, const char* reason));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_LifecycleService_State,
                (SilKit_ParticipantState * outParticipantState, SilKit_LifecycleService* lifecycleService));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_LifecycleService_Status,
                (SilKit_ParticipantStatus * outParticipantStatus, SilKit_LifecycleService* lifecycleService));

    // TimeSyncService

    MOCK_METHOD(SilKit_ReturnCode, SilKit_TimeSyncService_Create,
                (SilKit_TimeSyncService * *outTimeSyncService, SilKit_LifecycleService* lifecycleService));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_TimeSyncService_SetSimulationStepHandler,
                (SilKit_TimeSyncService * timeSyncService, void* context,
                 SilKit_TimeSyncService_SimulationStepHandler_t handler, SilKit_NanosecondsTime initialStepSize));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_TimeSyncService_SetSimulationStepHandlerAsync,
                (SilKit_TimeSyncService * timeSyncService, void* context,
                 SilKit_TimeSyncService_SimulationStepHandler_t handler, SilKit_NanosecondsTime initialStepSize));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_TimeSyncService_CompleteSimulationStep,
                (SilKit_TimeSyncService * timeSyncService));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_TimeSyncService_Now,
                (SilKit_TimeSyncService * timeSyncService, SilKit_NanosecondsTime* outNanosecondsTime));

    // SystemMonitor

    MOCK_METHOD(SilKit_ReturnCode, SilKit_SystemMonitor_Create,
                (SilKit_SystemMonitor * *outSystemMonitor, SilKit_Participant* participant));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_SystemMonitor_GetParticipantStatus,
                (SilKit_ParticipantStatus * outParticipantState, SilKit_SystemMonitor* systemMonitor,
                 const char* participantName));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_SystemMonitor_GetSystemState,
                (SilKit_SystemState * outSystemState, SilKit_SystemMonitor* systemMonitor));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_SystemMonitor_AddSystemStateHandler,
                (SilKit_SystemMonitor * systemMonitor, void* context, SilKit_SystemStateHandler_t handler,
                 SilKit_HandlerId* outHandlerId));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_SystemMonitor_RemoveSystemStateHandler,
                (SilKit_SystemMonitor * systemMonitor, SilKit_HandlerId handlerId));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_SystemMonitor_AddParticipantStatusHandler,
                (SilKit_SystemMonitor * systemMonitor, void* context, SilKit_ParticipantStatusHandler_t handler,
                 SilKit_HandlerId* outHandlerId));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_SystemMonitor_RemoveParticipantStatusHandler,
                (SilKit_SystemMonitor * systemMonitor, SilKit_HandlerId handlerId));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_SystemMonitor_SetParticipantConnectedHandler,
                (SilKit_SystemMonitor * systemMonitor, void* context,
                 SilKit_SystemMonitor_ParticipantConnectedHandler_t handler));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_SystemMonitor_SetParticipantDisconnectedHandler,
                (SilKit_SystemMonitor * systemMonitor, void* context,
                 SilKit_SystemMonitor_ParticipantDisconnectedHandler_t handler));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_SystemMonitor_IsParticipantConnected,
                (SilKit_SystemMonitor * systemMonitor, const char* participantName, SilKit_Bool* out));

    // SystemController

    MOCK_METHOD(SilKit_ReturnCode, SilKit_Experimental_SystemController_Create,
                (SilKit_Experimental_SystemController * *outSystemController, SilKit_Participant* participant));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_Experimental_SystemController_AbortSimulation,
                (SilKit_Experimental_SystemController * systemController));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_Experimental_SystemController_SetWorkflowConfiguration,
                (SilKit_Experimental_SystemController * systemController,
                 const SilKit_WorkflowConfiguration* workflowConfiguration));

    // Participant

    MOCK_METHOD(SilKit_ReturnCode, SilKit_Participant_Create,
                (SilKit_Participant * *outParticipant, SilKit_ParticipantConfiguration* participantConfiguration,
                 const char* participantName, const char* registryUri));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_Participant_Destroy, (SilKit_Participant * participant));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_Participant_GetLogger,
                (SilKit_Logger * *outLogger, SilKit_Participant* participant));

    // ParticipantConfiguration

    MOCK_METHOD(SilKit_ReturnCode, SilKit_ParticipantConfiguration_FromString,
                (SilKit_ParticipantConfiguration * *outParticipantConfiguration,
                 const char* participantConfigurationString));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_ParticipantConfiguration_FromFile,
                (SilKit_ParticipantConfiguration * *outParticipantConfiguration,
                 const char* participantConfigurationFilename));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_ParticipantConfiguration_Destroy,
                (SilKit_ParticipantConfiguration * participantConfiguration));

    // Logger

    MOCK_METHOD(SilKit_ReturnCode, SilKit_Logger_Log,
                (SilKit_Logger * logger, SilKit_LoggingLevel level, const char* message));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_Logger_GetLogLevel, (SilKit_Logger * logger, SilKit_LoggingLevel* outLevel));

    // DataPublisher

    MOCK_METHOD(SilKit_ReturnCode, SilKit_DataPublisher_Create,
                (SilKit_DataPublisher * *outPublisher, SilKit_Participant* participant, const char* controllerName,
                 SilKit_DataSpec* dataSpec, uint8_t history));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_DataPublisher_Publish,
                (SilKit_DataPublisher * self, const SilKit_ByteVector* data));

    // DataSubscriber

    MOCK_METHOD(SilKit_ReturnCode, SilKit_DataSubscriber_Create,
                (SilKit_DataSubscriber * *outSubscriber, SilKit_Participant* participant, const char* controllerName,
                 SilKit_DataSpec* dataSpec, void* dataHandlerContext, SilKit_DataMessageHandler_t dataHandler));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_DataSubscriber_SetDataMessageHandler,
                (SilKit_DataSubscriber * self, void* context, SilKit_DataMessageHandler_t dataHandler));

    // RpcServer

    MOCK_METHOD(SilKit_ReturnCode, SilKit_RpcServer_Create,
                (SilKit_RpcServer * *outServer, SilKit_Participant* participant, const char* controllerName,
                 SilKit_RpcSpec* rpcSpec, void* callHandlerContext, SilKit_RpcCallHandler_t callHandler));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_RpcServer_SubmitResult,
                (SilKit_RpcServer * self, SilKit_RpcCallHandle* callHandle, const SilKit_ByteVector* returnData));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_RpcServer_SetCallHandler,
                (SilKit_RpcServer * self, void* context, SilKit_RpcCallHandler_t handler));

    // RpcClient

    MOCK_METHOD(SilKit_ReturnCode, SilKit_RpcClient_Create,
                (SilKit_RpcClient * *outClient, SilKit_Participant* participant, const char* controllerName,
                 SilKit_RpcSpec* rpcSpec, void* resultHandlerContext, SilKit_RpcCallResultHandler_t resultHandler));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_RpcClient_Call,
                (SilKit_RpcClient * self, const SilKit_ByteVector* argumentData, void* userContext));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_RpcClient_CallWithTimeout,
                (SilKit_RpcClient * self, const SilKit_ByteVector* argumentData, SilKit_NanosecondsTime timeout,
                 void* userContext));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_RpcClient_SetCallResultHandler,
                (SilKit_RpcClient * self, void* context, SilKit_RpcCallResultHandler_t handler));

    // SilKitRegistry

    MOCK_METHOD(SilKit_ReturnCode, SilKit_Vendor_Vector_SilKitRegistry_Create,
                (SilKit_Vendor_Vector_SilKitRegistry * *outRegistry,
                 SilKit_ParticipantConfiguration* participantConfiguration));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_Vendor_Vector_SilKitRegistry_Destroy,
                (SilKit_Vendor_Vector_SilKitRegistry * registry));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_Vendor_Vector_SilKitRegistry_SetAllDisconnectedHandler,
                (SilKit_Vendor_Vector_SilKitRegistry * registry, void* context,
                 SilKit_Vendor_Vector_SilKitRegistry_AllDisconnectedHandler_t handler));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_Vendor_Vector_SilKitRegistry_GetLogger,
                (SilKit_Logger * *outLogger, SilKit_Vendor_Vector_SilKitRegistry* registry));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_Vendor_Vector_SilKitRegistry_StartListening,
                (SilKit_Vendor_Vector_SilKitRegistry * registry, const char* listenUri, const char** outRegistryUri));

    // Version

    MOCK_METHOD(SilKit_ReturnCode, SilKit_Version_Major, (uint32_t * outVersionMajor));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_Version_Minor, (uint32_t * outVersionMinor));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_Version_Patch, (uint32_t * outVersionPatch));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_Version_BuildNumber, (uint32_t * outVersionBuildNumber));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_Version_String, (const char** outVersionString));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_Version_VersionSuffix, (const char** outVersionVersionSuffix));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_Version_GitHash, (const char** outVersionGitHash));


    // NetworkSimulator

    MOCK_METHOD(SilKit_ReturnCode, SilKit_Experimental_NetworkSimulator_Create,
                (SilKit_Experimental_NetworkSimulator * *outNetworkSimulator, SilKit_Participant* participant));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_Experimental_NetworkSimulator_Start,
                (SilKit_Experimental_NetworkSimulator * networkSimulator));

    MOCK_METHOD(SilKit_ReturnCode, SilKit_Experimental_NetworkSimulator_SimulateNetwork,
                (SilKit_Experimental_NetworkSimulator * networkSimulator, const char* networkName,
                 SilKit_Experimental_SimulatedNetworkType networkType, void* simulatedNetwork,
                 const SilKit_Experimental_SimulatedNetworkFunctions* simulatedNetworkFunctions));

    // CanEventProducer

    MOCK_METHOD(SilKit_ReturnCode, SilKit_Experimental_CanEventProducer_Produce,
                (SilKit_Experimental_CanEventProducer * eventProducer, SilKit_StructHeader* cEvent,
                 const SilKit_Experimental_EventReceivers* receivers));

    // FlexRayEventProducer

    MOCK_METHOD(SilKit_ReturnCode, SilKit_Experimental_FlexRayEventProducer_Produce,
                (SilKit_Experimental_FlexRayEventProducer * eventProducer, SilKit_StructHeader* cEvent,
                 const SilKit_Experimental_EventReceivers* receivers));

    // EthEventProducer

    MOCK_METHOD(SilKit_ReturnCode, SilKit_Experimental_EthernetEventProducer_Produce,
                (SilKit_Experimental_EthernetEventProducer * eventProducer, SilKit_StructHeader* cEvent,
                 const SilKit_Experimental_EventReceivers* receivers));
    // LinEventProducer

    MOCK_METHOD(SilKit_ReturnCode, SilKit_Experimental_LinEventProducer_Produce,
                (SilKit_Experimental_LinEventProducer * eventProducer, SilKit_StructHeader* cEvent,
                 const SilKit_Experimental_EventReceivers* receivers));
};

} // namespace SilKitHourglassTests
