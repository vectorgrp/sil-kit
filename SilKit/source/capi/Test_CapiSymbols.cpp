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

#include "silkit/capi/SilKit.h"

namespace {

// Link all SilKitAPI symbols into this test case without using them properly
// 	   Keep this list up to date! e.g. `grep -h  ^SilKitAPI  SilKit/include/silkit/capi/*` and compare
// 	   the output to the symbols used here.
// should expose missing symbols at compile time, no need to execute this ==> Disabled
TEST(Test_CapiSymbols, DISABLED_link_all_public_symbols)
{
SilKit_HandlerId id;

(void) SilKit_CanController_Create(nullptr, nullptr, "", "");
(void) SilKit_CanController_Start(nullptr);
(void) SilKit_CanController_Stop(nullptr);
(void) SilKit_CanController_Reset(nullptr);
(void) SilKit_CanController_Sleep(nullptr);
(void) SilKit_CanController_SendFrame(nullptr, nullptr, nullptr);
(void) SilKit_CanController_SetBaudRate(nullptr, 0,0,0);
(void) SilKit_CanController_AddFrameTransmitHandler(nullptr, nullptr, nullptr,0,&id);
(void) SilKit_CanController_RemoveFrameTransmitHandler(nullptr,0);
(void) SilKit_CanController_AddFrameHandler(nullptr, nullptr, nullptr, 0, &id);
(void) SilKit_CanController_RemoveFrameHandler(nullptr, id);
(void) SilKit_CanController_AddStateChangeHandler(nullptr, nullptr, nullptr, &id);
(void) SilKit_CanController_RemoveStateChangeHandler(nullptr, id);
(void) SilKit_CanController_AddErrorStateChangeHandler(nullptr, nullptr, nullptr, &id);
(void) SilKit_CanController_RemoveErrorStateChangeHandler(nullptr, id);
(void) SilKit_DataPublisher_Create(nullptr, nullptr,"",nullptr,0);
(void) SilKit_DataSubscriber_Create(nullptr, nullptr, "", nullptr, nullptr, nullptr);
(void) SilKit_DataPublisher_Publish(nullptr, nullptr);
(void) SilKit_DataSubscriber_SetDataMessageHandler(nullptr, nullptr, nullptr);
(void) SilKit_EthernetController_Create(nullptr, nullptr, "", "");
(void) SilKit_EthernetController_Activate(nullptr);
(void) SilKit_EthernetController_Deactivate(nullptr);
(void) SilKit_EthernetController_AddFrameHandler(nullptr,nullptr, nullptr, 0, &id);
(void) SilKit_EthernetController_RemoveFrameHandler(nullptr, id);
(void)
(void) SilKit_EthernetController_RemoveFrameTransmitHandler(nullptr, id);
(void) SilKit_EthernetController_AddStateChangeHandler(nullptr, nullptr, nullptr, &id);
(void) SilKit_EthernetController_RemoveStateChangeHandler(nullptr, id);
(void)
(void) SilKit_EthernetController_RemoveBitrateChangeHandler(nullptr, id);
(void) SilKit_EthernetController_SendFrame(nullptr, nullptr, nullptr);
(void) SilKit_FlexrayController_Create(nullptr,nullptr, nullptr, nullptr);
(void) SilKit_FlexrayController_Configure(nullptr, nullptr);
(void) SilKit_FlexrayController_ReconfigureTxBuffer(nullptr, 0, nullptr);
(void) SilKit_FlexrayController_UpdateTxBuffer(nullptr, nullptr);
(void) SilKit_FlexrayController_ExecuteCmd(nullptr, 0);
(void) SilKit_FlexrayController_AddFrameHandler(nullptr, nullptr, nullptr, &id);
(void) SilKit_FlexrayController_RemoveFrameHandler(nullptr, id);
(void) SilKit_FlexrayController_AddFrameTransmitHandler(nullptr, nullptr, nullptr, &id);
(void) SilKit_FlexrayController_RemoveFrameTransmitHandler(nullptr,id);
(void) SilKit_FlexrayController_AddWakeupHandler(nullptr, nullptr, nullptr, &id);
(void) SilKit_FlexrayController_RemoveWakeupHandler(nullptr,id);
(void) SilKit_FlexrayController_AddPocStatusHandler(nullptr, nullptr, nullptr, &id);
(void) SilKit_FlexrayController_RemovePocStatusHandler(nullptr, id);
(void) SilKit_FlexrayController_AddSymbolHandler(nullptr, nullptr, nullptr, &id);
(void) SilKit_FlexrayController_RemoveSymbolHandler(nullptr, id);
(void)
(void) SilKit_FlexrayController_RemoveSymbolTransmitHandler(nullptr, id);
(void) SilKit_FlexrayController_AddCycleStartHandler(nullptr,nullptr, nullptr, &id);
(void) SilKit_FlexrayController_RemoveCycleStartHandler(nullptr, id);
(void) SilKit_LinController_Create(nullptr, nullptr, nullptr, nullptr);
(void) SilKit_LinController_Init(nullptr, nullptr);
(void) SilKit_LinController_Status(nullptr, nullptr);
(void) SilKit_LinController_SendFrame(nullptr, nullptr, 0);
(void) SilKit_LinController_SendFrameHeader(nullptr, 0);
(void) SilKit_LinController_UpdateTxBuffer(nullptr, nullptr);
(void) SilKit_LinController_GoToSleep(nullptr);
(void) SilKit_LinController_GoToSleepInternal(nullptr);
(void) SilKit_LinController_Wakeup(nullptr);
(void) SilKit_LinController_WakeupInternal(nullptr);
(void) SilKit_Experimental_LinController_GetSlaveConfiguration(nullptr, nullptr);
(void) SilKit_LinController_AddFrameStatusHandler(nullptr, nullptr, nullptr, &id);
(void) SilKit_LinController_RemoveFrameStatusHandler(nullptr, id);
(void) SilKit_LinController_AddGoToSleepHandler(nullptr, nullptr, nullptr, &id);
(void) SilKit_LinController_RemoveGoToSleepHandler(nullptr, id);
(void) SilKit_LinController_AddWakeupHandler(nullptr, nullptr, nullptr, &id);
(void) SilKit_LinController_RemoveWakeupHandler(nullptr, id);
(void) SilKit_Experimental_LinController_AddLinSlaveConfigurationHandler(nullptr, nullptr, nullptr, &id);
(void) SilKit_Experimental_LinController_RemoveLinSlaveConfigurationHandler(nullptr, 0);
(void) SilKit_Logger_Log(nullptr, 0, "");
(void) SilKit_Logger_GetLogLevel(nullptr, nullptr);
(void) SilKit_SystemMonitor_Create(nullptr, nullptr);
(void) SilKit_LifecycleService_Create(nullptr, nullptr, nullptr);
(void) SilKit_TimeSyncService_Create(nullptr, nullptr);
(void) SilKit_LifecycleService_SetCommunicationReadyHandler(nullptr, nullptr, nullptr);
(void) SilKit_LifecycleService_SetCommunicationReadyHandlerAsync(nullptr, nullptr, nullptr);
(void) SilKit_LifecycleService_CompleteCommunicationReadyHandlerAsync(nullptr);
(void) SilKit_LifecycleService_SetStartingHandler(nullptr, nullptr, nullptr);
(void) SilKit_LifecycleService_SetStopHandler(nullptr, nullptr, nullptr);
(void) SilKit_LifecycleService_SetShutdownHandler(nullptr, nullptr, nullptr);
(void) SilKit_LifecycleService_SetAbortHandler(nullptr, nullptr, nullptr);
(void) SilKit_TimeSyncService_SetSimulationStepHandler(nullptr, nullptr, nullptr, 0);
(void) SilKit_TimeSyncService_SetSimulationStepHandlerAsync(nullptr, nullptr, nullptr, 0);
(void) SilKit_TimeSyncService_CompleteSimulationStep(nullptr);
(void) SilKit_LifecycleService_Pause(nullptr, "");
(void) SilKit_LifecycleService_Continue(nullptr);
(void) SilKit_LifecycleService_Stop(nullptr, "");
(void) SilKit_SystemMonitor_GetParticipantStatus(nullptr, nullptr, "");
(void) SilKit_SystemMonitor_GetSystemState(nullptr, nullptr);
(void) SilKit_SystemMonitor_AddSystemStateHandler(nullptr, nullptr, nullptr, &id);
(void) SilKit_SystemMonitor_RemoveSystemStateHandler(nullptr, 0);
(void) SilKit_SystemMonitor_AddParticipantStatusHandler(nullptr, nullptr, 0, nullptr);
(void) SilKit_SystemMonitor_RemoveParticipantStatusHandler(nullptr, 0);
(void) SilKit_LifecycleService_StartLifecycle(nullptr);
(void) SilKit_LifecycleService_WaitForLifecycleToComplete(nullptr, nullptr);
(void) SilKit_ParticipantConfiguration_FromString(nullptr, "");
(void) SilKit_ParticipantConfiguration_Destroy(nullptr);
(void) SilKit_Participant_Create(nullptr, nullptr, "", "");
(void) SilKit_Participant_Destroy(nullptr);
(void) SilKit_RpcServer_Create(nullptr, nullptr,"", nullptr, nullptr, nullptr);
(void) SilKit_RpcServer_SubmitResult(nullptr, nullptr, nullptr);
(void) SilKit_RpcServer_SetCallHandler(nullptr, nullptr, nullptr);
(void) SilKit_RpcClient_Create(nullptr, nullptr, "", nullptr, nullptr, nullptr);
(void) SilKit_RpcClient_Call(nullptr, nullptr, nullptr);
(void) SilKit_RpcClient_SetCallResultHandler(nullptr, nullptr, nullptr);
(void) SilKit_ReturnCodeToString(nullptr, SilKit_ReturnCode_BADPARAMETER);
(void) SilKit_Participant_GetLogger(nullptr, nullptr);
(void)SilKit_GetLastErrorString();
}

}
