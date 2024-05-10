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

#include "MockCapi.hpp"

namespace {

SilKitHourglassTests::MockCapi* globalCapi{nullptr};

} // namespace

namespace SilKitHourglassTests {

void MockCapi::SetUpGlobalCapi()
{
    ASSERT_EQ(globalCapi, nullptr);
    globalCapi = this;
}

void MockCapi::TearDownGlobalCapi()
{
    ASSERT_EQ(globalCapi, this);
    globalCapi = nullptr;
}

} // namespace SilKitHourglassTests

extern "C"
{
    // Common

    SilKit_ReturnCode SilKitCALL SilKit_ReturnCodeToString(const char** outString, SilKit_ReturnCode returnCode)
    {
        return globalCapi->SilKit_ReturnCodeToString(outString, returnCode);
    }

    const char* SilKitCALL SilKit_GetLastErrorString()
    {
        return globalCapi->SilKit_GetLastErrorString();
    }

    // CanController

    SilKit_ReturnCode SilKitCALL SilKit_CanController_Create(SilKit_CanController** outCanController,
                                                             SilKit_Participant* participant, const char* name,
                                                             const char* network)
    {
        return globalCapi->SilKit_CanController_Create(outCanController, participant, name, network);
    }

    SilKit_ReturnCode SilKitCALL SilKit_CanController_Start(SilKit_CanController* controller)
    {
        return globalCapi->SilKit_CanController_Start(controller);
    }

    SilKit_ReturnCode SilKitCALL SilKit_CanController_Stop(SilKit_CanController* controller)
    {
        return globalCapi->SilKit_CanController_Stop(controller);
    }

    SilKit_ReturnCode SilKitCALL SilKit_CanController_Reset(SilKit_CanController* controller)
    {
        return globalCapi->SilKit_CanController_Reset(controller);
    }

    SilKit_ReturnCode SilKitCALL SilKit_CanController_Sleep(SilKit_CanController* controller)
    {
        return globalCapi->SilKit_CanController_Sleep(controller);
    }

    SilKit_ReturnCode SilKitCALL SilKit_CanController_SendFrame(SilKit_CanController* controller,
                                                                SilKit_CanFrame* frame, void* userContext)
    {
        return globalCapi->SilKit_CanController_SendFrame(controller, frame, userContext);
    }

    SilKit_ReturnCode SilKitCALL SilKit_CanController_SetBaudRate(SilKit_CanController* controller, uint32_t rate,
                                                                  uint32_t fdRate, uint32_t xlRate)
    {
        return globalCapi->SilKit_CanController_SetBaudRate(controller, rate, fdRate, xlRate);
    }

    SilKit_ReturnCode SilKitCALL SilKit_CanController_AddFrameTransmitHandler(SilKit_CanController* controller,
                                                                              void* context,
                                                                              SilKit_CanFrameTransmitHandler_t handler,
                                                                              SilKit_CanTransmitStatus statusMask,
                                                                              SilKit_HandlerId* outHandlerId)
    {
        return globalCapi->SilKit_CanController_AddFrameTransmitHandler(controller, context, handler, statusMask,
                                                                        outHandlerId);
    }

    SilKit_ReturnCode SilKitCALL SilKit_CanController_RemoveFrameTransmitHandler(SilKit_CanController* controller,
                                                                                 SilKit_HandlerId handlerId)
    {
        return globalCapi->SilKit_CanController_RemoveFrameTransmitHandler(controller, handlerId);
    }

    SilKit_ReturnCode SilKitCALL SilKit_CanController_AddFrameHandler(SilKit_CanController* controller, void* context,
                                                                      SilKit_CanFrameHandler_t handler,
                                                                      SilKit_Direction directionMask,
                                                                      SilKit_HandlerId* outHandlerId)
    {
        return globalCapi->SilKit_CanController_AddFrameHandler(controller, context, handler, directionMask,
                                                                outHandlerId);
    }

    SilKit_ReturnCode SilKitCALL SilKit_CanController_RemoveFrameHandler(SilKit_CanController* controller,
                                                                         SilKit_HandlerId handlerId)
    {
        return globalCapi->SilKit_CanController_RemoveFrameHandler(controller, handlerId);
    }

    SilKit_ReturnCode SilKitCALL SilKit_CanController_AddStateChangeHandler(SilKit_CanController* controller,
                                                                            void* context,
                                                                            SilKit_CanStateChangeHandler_t handler,
                                                                            SilKit_HandlerId* outHandlerId)
    {
        return globalCapi->SilKit_CanController_AddStateChangeHandler(controller, context, handler, outHandlerId);
    }

    SilKit_ReturnCode SilKitCALL SilKit_CanController_RemoveStateChangeHandler(SilKit_CanController* controller,
                                                                               SilKit_HandlerId handlerId)
    {
        return globalCapi->SilKit_CanController_RemoveStateChangeHandler(controller, handlerId);
    }

    SilKit_ReturnCode SilKitCALL SilKit_CanController_AddErrorStateChangeHandler(
        SilKit_CanController* controller, void* context, SilKit_CanErrorStateChangeHandler_t handler,
        SilKit_HandlerId* outHandlerId)
    {
        return globalCapi->SilKit_CanController_AddErrorStateChangeHandler(controller, context, handler, outHandlerId);
    }

    SilKit_ReturnCode SilKitCALL SilKit_CanController_RemoveErrorStateChangeHandler(SilKit_CanController* controller,
                                                                                    SilKit_HandlerId handlerId)
    {
        return globalCapi->SilKit_CanController_RemoveErrorStateChangeHandler(controller, handlerId);
    }

    // EthernetController

    SilKit_ReturnCode SilKitCALL SilKit_EthernetController_Create(SilKit_EthernetController** outController,
                                                                  SilKit_Participant* participant, const char* name,
                                                                  const char* network)
    {
        return globalCapi->SilKit_EthernetController_Create(outController, participant, name, network);
    }

    SilKit_ReturnCode SilKitCALL SilKit_EthernetController_Activate(SilKit_EthernetController* controller)
    {
        return globalCapi->SilKit_EthernetController_Activate(controller);
    }

    SilKit_ReturnCode SilKitCALL SilKit_EthernetController_Deactivate(SilKit_EthernetController* controller)
    {
        return globalCapi->SilKit_EthernetController_Deactivate(controller);
    }

    SilKit_ReturnCode SilKitCALL SilKit_EthernetController_AddFrameHandler(SilKit_EthernetController* controller,
                                                                           void* context,
                                                                           SilKit_EthernetFrameHandler_t handler,
                                                                           SilKit_Direction directionMask,
                                                                           SilKit_HandlerId* outHandlerId)
    {
        return globalCapi->SilKit_EthernetController_AddFrameHandler(controller, context, handler, directionMask,
                                                                     outHandlerId);
    }

    SilKit_ReturnCode SilKitCALL SilKit_EthernetController_RemoveFrameHandler(SilKit_EthernetController* controller,
                                                                              SilKit_HandlerId handlerId)
    {
        return globalCapi->SilKit_EthernetController_RemoveFrameHandler(controller, handlerId);
    }

    SilKit_ReturnCode SilKitCALL SilKit_EthernetController_AddFrameTransmitHandler(
        SilKit_EthernetController* controller, void* context, SilKit_EthernetFrameTransmitHandler_t handler,
        SilKit_EthernetTransmitStatus transmitStatusMask, SilKit_HandlerId* outHandlerId)
    {
        return globalCapi->SilKit_EthernetController_AddFrameTransmitHandler(controller, context, handler,
                                                                             transmitStatusMask, outHandlerId);
    }

    SilKit_ReturnCode SilKitCALL SilKit_EthernetController_RemoveFrameTransmitHandler(
        SilKit_EthernetController* controller, SilKit_HandlerId handlerId)
    {
        return globalCapi->SilKit_EthernetController_RemoveFrameTransmitHandler(controller, handlerId);
    }

    SilKit_ReturnCode SilKitCALL SilKit_EthernetController_AddStateChangeHandler(
        SilKit_EthernetController* controller, void* context, SilKit_EthernetStateChangeHandler_t handler,
        SilKit_HandlerId* outHandlerId)
    {
        return globalCapi->SilKit_EthernetController_AddStateChangeHandler(controller, context, handler, outHandlerId);
    }

    SilKit_ReturnCode SilKitCALL SilKit_EthernetController_RemoveStateChangeHandler(
        SilKit_EthernetController* controller, SilKit_HandlerId handlerId)
    {
        return globalCapi->SilKit_EthernetController_RemoveStateChangeHandler(controller, handlerId);
    }

    SilKit_ReturnCode SilKitCALL SilKit_EthernetController_AddBitrateChangeHandler(
        SilKit_EthernetController* controller, void* context, SilKit_EthernetBitrateChangeHandler_t handler,
        SilKit_HandlerId* outHandlerId)
    {
        return globalCapi->SilKit_EthernetController_AddBitrateChangeHandler(controller, context, handler,
                                                                             outHandlerId);
    }

    SilKit_ReturnCode SilKitCALL SilKit_EthernetController_RemoveBitrateChangeHandler(
        SilKit_EthernetController* controller, SilKit_HandlerId handlerId)
    {
        return globalCapi->SilKit_EthernetController_RemoveBitrateChangeHandler(controller, handlerId);
    }

    SilKit_ReturnCode SilKitCALL SilKit_EthernetController_SendFrame(SilKit_EthernetController* controller,
                                                                     SilKit_EthernetFrame* frame, void* userContext)
    {
        return globalCapi->SilKit_EthernetController_SendFrame(controller, frame, userContext);
    }

    // FlexrayController

    SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_Create(SilKit_FlexrayController** outController,
                                                                 SilKit_Participant* participant, const char* name,
                                                                 const char* network)
    {
        return globalCapi->SilKit_FlexrayController_Create(outController, participant, name, network);
    }

    SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_Configure(SilKit_FlexrayController* controller,
                                                                    const SilKit_FlexrayControllerConfig* config)
    {
        return globalCapi->SilKit_FlexrayController_Configure(controller, config);
    }

    SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_ReconfigureTxBuffer(
        SilKit_FlexrayController* controller, uint16_t txBufferIdx, const SilKit_FlexrayTxBufferConfig* config)
    {
        return globalCapi->SilKit_FlexrayController_ReconfigureTxBuffer(controller, txBufferIdx, config);
    }

    SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_UpdateTxBuffer(SilKit_FlexrayController* controller,
                                                                         const SilKit_FlexrayTxBufferUpdate* update)
    {
        return globalCapi->SilKit_FlexrayController_UpdateTxBuffer(controller, update);
    }

    SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_ExecuteCmd(SilKit_FlexrayController* controller,
                                                                     SilKit_FlexrayChiCommand cmd)
    {
        return globalCapi->SilKit_FlexrayController_ExecuteCmd(controller, cmd);
    }

    SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_AddFrameHandler(SilKit_FlexrayController* controller,
                                                                          void* context,
                                                                          SilKit_FlexrayFrameHandler_t handler,
                                                                          SilKit_HandlerId* outHandlerId)
    {
        return globalCapi->SilKit_FlexrayController_AddFrameHandler(controller, context, handler, outHandlerId);
    }

    SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_RemoveFrameHandler(SilKit_FlexrayController* controller,
                                                                             SilKit_HandlerId handlerId)
    {
        return globalCapi->SilKit_FlexrayController_RemoveFrameHandler(controller, handlerId);
    }

    SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_AddFrameTransmitHandler(
        SilKit_FlexrayController* controller, void* context, SilKit_FlexrayFrameTransmitHandler_t handler,
        SilKit_HandlerId* outHandlerId)
    {
        return globalCapi->SilKit_FlexrayController_AddFrameTransmitHandler(controller, context, handler, outHandlerId);
    }

    SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_RemoveFrameTransmitHandler(
        SilKit_FlexrayController* controller, SilKit_HandlerId handlerId)
    {
        return globalCapi->SilKit_FlexrayController_RemoveFrameTransmitHandler(controller, handlerId);
    }

    SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_AddWakeupHandler(SilKit_FlexrayController* controller,
                                                                           void* context,
                                                                           SilKit_FlexrayWakeupHandler_t handler,
                                                                           SilKit_HandlerId* outHandlerId)
    {
        return globalCapi->SilKit_FlexrayController_AddWakeupHandler(controller, context, handler, outHandlerId);
    }

    SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_RemoveWakeupHandler(SilKit_FlexrayController* controller,
                                                                              SilKit_HandlerId handlerId)
    {
        return globalCapi->SilKit_FlexrayController_RemoveWakeupHandler(controller, handlerId);
    }

    SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_AddPocStatusHandler(SilKit_FlexrayController* controller,
                                                                              void* context,
                                                                              SilKit_FlexrayPocStatusHandler_t handler,
                                                                              SilKit_HandlerId* outHandlerId)
    {
        return globalCapi->SilKit_FlexrayController_AddPocStatusHandler(controller, context, handler, outHandlerId);
    }

    SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_RemovePocStatusHandler(SilKit_FlexrayController* controller,
                                                                                 SilKit_HandlerId handlerId)
    {
        return globalCapi->SilKit_FlexrayController_RemovePocStatusHandler(controller, handlerId);
    }

    SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_AddSymbolHandler(SilKit_FlexrayController* controller,
                                                                           void* context,
                                                                           SilKit_FlexraySymbolHandler_t handler,
                                                                           SilKit_HandlerId* outHandlerId)
    {
        return globalCapi->SilKit_FlexrayController_AddSymbolHandler(controller, context, handler, outHandlerId);
    }

    SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_RemoveSymbolHandler(SilKit_FlexrayController* controller,
                                                                              SilKit_HandlerId handlerId)
    {
        return globalCapi->SilKit_FlexrayController_RemoveSymbolHandler(controller, handlerId);
    }

    SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_AddSymbolTransmitHandler(
        SilKit_FlexrayController* controller, void* context, SilKit_FlexraySymbolTransmitHandler_t handler,
        SilKit_HandlerId* outHandlerId)
    {
        return globalCapi->SilKit_FlexrayController_AddSymbolTransmitHandler(controller, context, handler,
                                                                             outHandlerId);
    }

    SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_RemoveSymbolTransmitHandler(
        SilKit_FlexrayController* controller, SilKit_HandlerId handlerId)
    {
        return globalCapi->SilKit_FlexrayController_RemoveSymbolTransmitHandler(controller, handlerId);
    }

    SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_AddCycleStartHandler(
        SilKit_FlexrayController* controller, void* context, SilKit_FlexrayCycleStartHandler_t handler,
        SilKit_HandlerId* outHandlerId)
    {
        return globalCapi->SilKit_FlexrayController_AddCycleStartHandler(controller, context, handler, outHandlerId);
    }

    SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_RemoveCycleStartHandler(SilKit_FlexrayController* controller,
                                                                                  SilKit_HandlerId handlerId)
    {
        return globalCapi->SilKit_FlexrayController_RemoveCycleStartHandler(controller, handlerId);
    }

    // Lin

    SilKit_ReturnCode SilKitCALL SilKit_LinController_Create(SilKit_LinController** outLinController,
                                                             SilKit_Participant* participant, const char* name,
                                                             const char* network)
    {
        return globalCapi->SilKit_LinController_Create(outLinController, participant, name, network);
    }

    SilKit_ReturnCode SilKitCALL SilKit_LinController_Init(SilKit_LinController* controller,
                                                           const SilKit_LinControllerConfig* config)
    {
        return globalCapi->SilKit_LinController_Init(controller, config);
    }

    SilKit_ReturnCode SilKitCALL SilKit_Experimental_LinController_InitDynamic(
        SilKit_LinController* controller, const SilKit_Experimental_LinControllerDynamicConfig* config)
    {
        return globalCapi->SilKit_Experimental_LinController_InitDynamic(controller, config);
    }

    SilKit_ReturnCode SilKitCALL SilKit_LinController_SetFrameResponse(SilKit_LinController* controller,
                                                                       const SilKit_LinFrameResponse* response)
    {
        return globalCapi->SilKit_LinController_SetFrameResponse(controller, response);
    }

    SilKit_ReturnCode SilKitCALL SilKit_LinController_Status(SilKit_LinController* controller,
                                                             SilKit_LinControllerStatus* outStatus)
    {
        return globalCapi->SilKit_LinController_Status(controller, outStatus);
    }

    SilKit_ReturnCode SilKitCALL SilKit_LinController_SendFrame(SilKit_LinController* controller,
                                                                const SilKit_LinFrame* frame,
                                                                SilKit_LinFrameResponseType responseType)
    {
        return globalCapi->SilKit_LinController_SendFrame(controller, frame, responseType);
    }

    SilKit_ReturnCode SilKitCALL SilKit_LinController_SendFrameHeader(SilKit_LinController* controller,
                                                                      SilKit_LinId linId)
    {
        return globalCapi->SilKit_LinController_SendFrameHeader(controller, linId);
    }

    SilKit_ReturnCode SilKitCALL SilKit_LinController_UpdateTxBuffer(SilKit_LinController* controller,
                                                                     const SilKit_LinFrame* frame)
    {
        return globalCapi->SilKit_LinController_UpdateTxBuffer(controller, frame);
    }

    SilKit_ReturnCode SilKitCALL SilKit_LinController_GoToSleep(SilKit_LinController* controller)
    {
        return globalCapi->SilKit_LinController_GoToSleep(controller);
    }

    SilKit_ReturnCode SilKitCALL SilKit_LinController_GoToSleepInternal(SilKit_LinController* controller)
    {
        return globalCapi->SilKit_LinController_GoToSleepInternal(controller);
    }

    SilKit_ReturnCode SilKitCALL SilKit_LinController_Wakeup(SilKit_LinController* controller)
    {
        return globalCapi->SilKit_LinController_Wakeup(controller);
    }

    SilKit_ReturnCode SilKitCALL SilKit_LinController_WakeupInternal(SilKit_LinController* controller)
    {
        return globalCapi->SilKit_LinController_WakeupInternal(controller);
    }

    SilKit_ReturnCode SilKitCALL SilKit_Experimental_LinController_GetSlaveConfiguration(
        SilKit_LinController* controller, SilKit_Experimental_LinSlaveConfiguration* outLinSlaveConfiguration)
    {
        return globalCapi->SilKit_Experimental_LinController_GetSlaveConfiguration(controller,
                                                                                   outLinSlaveConfiguration);
    }

    SilKit_ReturnCode SilKitCALL SilKit_LinController_AddFrameStatusHandler(SilKit_LinController* controller,
                                                                            void* context,
                                                                            SilKit_LinFrameStatusHandler_t handler,
                                                                            SilKit_HandlerId* outHandlerId)
    {
        return globalCapi->SilKit_LinController_AddFrameStatusHandler(controller, context, handler, outHandlerId);
    }

    SilKit_ReturnCode SilKitCALL SilKit_LinController_RemoveFrameStatusHandler(SilKit_LinController* controller,
                                                                               SilKit_HandlerId handlerId)
    {
        return globalCapi->SilKit_LinController_RemoveFrameStatusHandler(controller, handlerId);
    }

    SilKit_ReturnCode SilKitCALL SilKit_LinController_AddGoToSleepHandler(SilKit_LinController* controller,
                                                                          void* context,
                                                                          SilKit_LinGoToSleepHandler_t handler,
                                                                          SilKit_HandlerId* outHandlerId)
    {
        return globalCapi->SilKit_LinController_AddGoToSleepHandler(controller, context, handler, outHandlerId);
    }

    SilKit_ReturnCode SilKitCALL SilKit_LinController_RemoveGoToSleepHandler(SilKit_LinController* controller,
                                                                             SilKit_HandlerId handlerId)
    {
        return globalCapi->SilKit_LinController_RemoveGoToSleepHandler(controller, handlerId);
    }

    SilKit_ReturnCode SilKitCALL SilKit_LinController_AddWakeupHandler(SilKit_LinController* controller, void* context,
                                                                       SilKit_LinWakeupHandler_t handler,
                                                                       SilKit_HandlerId* outHandlerId)
    {
        return globalCapi->SilKit_LinController_AddWakeupHandler(controller, context, handler, outHandlerId);
    }

    SilKit_ReturnCode SilKitCALL SilKit_LinController_RemoveWakeupHandler(SilKit_LinController* controller,
                                                                          SilKit_HandlerId handlerId)
    {
        return globalCapi->SilKit_LinController_RemoveWakeupHandler(controller, handlerId);
    }

    SilKit_ReturnCode SilKitCALL SilKit_Experimental_LinController_AddLinSlaveConfigurationHandler(
        SilKit_LinController* controller, void* context, SilKit_Experimental_LinSlaveConfigurationHandler_t handler,
        SilKit_HandlerId* outHandlerId)
    {
        return globalCapi->SilKit_Experimental_LinController_AddLinSlaveConfigurationHandler(controller, context,
                                                                                             handler, outHandlerId);
    }

    SilKit_ReturnCode SilKitCALL SilKit_Experimental_LinController_RemoveLinSlaveConfigurationHandler(
        SilKit_LinController* controller, SilKit_HandlerId handlerId)
    {
        return globalCapi->SilKit_Experimental_LinController_RemoveLinSlaveConfigurationHandler(controller, handlerId);
    }

    SilKit_ReturnCode SilKitCALL SilKit_Experimental_LinController_AddFrameHeaderHandler(
        SilKit_LinController* controller, void* context, SilKit_Experimental_LinFrameHeaderHandler_t handler,
        SilKit_HandlerId* outHandlerId)
    {
        return globalCapi->SilKit_Experimental_LinController_AddFrameHeaderHandler(controller, context, handler,
                                                                                   outHandlerId);
    }

    SilKit_ReturnCode SilKitCALL SilKit_Experimental_LinController_RemoveFrameHeaderHandler(
        SilKit_LinController* controller, SilKit_HandlerId handlerId)
    {
        return globalCapi->SilKit_Experimental_LinController_RemoveFrameHeaderHandler(controller, handlerId);
    }

    SilKit_ReturnCode SilKitCALL SilKit_Experimental_LinController_SendDynamicResponse(SilKit_LinController* controller,
                                                                                       const SilKit_LinFrame* frame)
    {
        return globalCapi->SilKit_Experimental_LinController_SendDynamicResponse(controller, frame);
    }

    // LifecycleService

    SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_Create(SilKit_LifecycleService** outLifecycleService,
                                                                SilKit_Participant* participant,
                                                                const SilKit_LifecycleConfiguration* startConfiguration)
    {
        return globalCapi->SilKit_LifecycleService_Create(outLifecycleService, participant, startConfiguration);
    }

    SilKit_ReturnCode SilKitCALL
    SilKit_LifecycleService_SetCommunicationReadyHandler(SilKit_LifecycleService* lifecycleService, void* context,
                                                         SilKit_LifecycleService_CommunicationReadyHandler_t handler)
    {
        return globalCapi->SilKit_LifecycleService_SetCommunicationReadyHandler(lifecycleService, context, handler);
    }

    SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_SetCommunicationReadyHandlerAsync(
        SilKit_LifecycleService* lifecycleService, void* context,
        SilKit_LifecycleService_CommunicationReadyHandler_t handler)
    {
        return globalCapi->SilKit_LifecycleService_SetCommunicationReadyHandlerAsync(lifecycleService, context,
                                                                                     handler);
    }

    SilKit_ReturnCode SilKitCALL
    SilKit_LifecycleService_CompleteCommunicationReadyHandlerAsync(SilKit_LifecycleService* lifecycleService)
    {
        return globalCapi->SilKit_LifecycleService_CompleteCommunicationReadyHandlerAsync(lifecycleService);
    }

    SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_SetStartingHandler(
        SilKit_LifecycleService* lifecycleService, void* context, SilKit_LifecycleService_StartingHandler_t handler)
    {
        return globalCapi->SilKit_LifecycleService_SetStartingHandler(lifecycleService, context, handler);
    }

    SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_SetStopHandler(SilKit_LifecycleService* lifecycleService,
                                                                        void* context,
                                                                        SilKit_LifecycleService_StopHandler_t handler)
    {
        return globalCapi->SilKit_LifecycleService_SetStopHandler(lifecycleService, context, handler);
    }

    SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_SetShutdownHandler(
        SilKit_LifecycleService* lifecycleService, void* context, SilKit_LifecycleService_ShutdownHandler_t handler)
    {
        return globalCapi->SilKit_LifecycleService_SetShutdownHandler(lifecycleService, context, handler);
    }

    SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_SetAbortHandler(SilKit_LifecycleService* lifecycleService,
                                                                         void* context,
                                                                         SilKit_LifecycleService_AbortHandler_t handler)
    {
        return globalCapi->SilKit_LifecycleService_SetAbortHandler(lifecycleService, context, handler);
    }

    SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_StartLifecycle(SilKit_LifecycleService* lifecycleService)
    {
        return globalCapi->SilKit_LifecycleService_StartLifecycle(lifecycleService);
    }

    SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_WaitForLifecycleToComplete(
        SilKit_LifecycleService* lifecycleService, SilKit_ParticipantState* outParticipantState)
    {
        return globalCapi->SilKit_LifecycleService_WaitForLifecycleToComplete(lifecycleService, outParticipantState);
    }

    SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_ReportError(SilKit_LifecycleService* lifecycleService,
                                                                     const char* reason)
    {
        return globalCapi->SilKit_LifecycleService_ReportError(lifecycleService, reason);
    }

    SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_Pause(SilKit_LifecycleService* lifecycleService,
                                                               const char* reason)
    {
        return globalCapi->SilKit_LifecycleService_Pause(lifecycleService, reason);
    }

    SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_Continue(SilKit_LifecycleService* lifecycleService)
    {
        return globalCapi->SilKit_LifecycleService_Continue(lifecycleService);
    }

    SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_Stop(SilKit_LifecycleService* lifecycleService,
                                                              const char* reason)
    {
        return globalCapi->SilKit_LifecycleService_Stop(lifecycleService, reason);
    }

    SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_State(SilKit_ParticipantState* outParticipantState,
                                                               SilKit_LifecycleService* lifecycleService)
    {
        return globalCapi->SilKit_LifecycleService_State(outParticipantState, lifecycleService);
    }

    SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_Status(SilKit_ParticipantStatus* outParticipantStatus,
                                                                SilKit_LifecycleService* lifecycleService)
    {
        return globalCapi->SilKit_LifecycleService_Status(outParticipantStatus, lifecycleService);
    }

    // TimeSyncService

    SilKit_ReturnCode SilKitCALL SilKit_TimeSyncService_Create(SilKit_TimeSyncService** outTimeSyncService,
                                                               SilKit_LifecycleService* lifecycleService)
    {
        return globalCapi->SilKit_TimeSyncService_Create(outTimeSyncService, lifecycleService);
    }

    SilKit_ReturnCode SilKitCALL SilKit_TimeSyncService_SetSimulationStepHandler(
        SilKit_TimeSyncService* timeSyncService, void* context, SilKit_TimeSyncService_SimulationStepHandler_t handler,
        SilKit_NanosecondsTime initialStepSize)
    {
        return globalCapi->SilKit_TimeSyncService_SetSimulationStepHandler(timeSyncService, context, handler,
                                                                           initialStepSize);
    }

    SilKit_ReturnCode SilKitCALL SilKit_TimeSyncService_SetSimulationStepHandlerAsync(
        SilKit_TimeSyncService* timeSyncService, void* context, SilKit_TimeSyncService_SimulationStepHandler_t handler,
        SilKit_NanosecondsTime initialStepSize)
    {
        return globalCapi->SilKit_TimeSyncService_SetSimulationStepHandlerAsync(timeSyncService, context, handler,
                                                                                initialStepSize);
    }

    SilKit_ReturnCode SilKitCALL SilKit_TimeSyncService_CompleteSimulationStep(SilKit_TimeSyncService* timeSyncService)
    {
        return globalCapi->SilKit_TimeSyncService_CompleteSimulationStep(timeSyncService);
    }

    SilKit_ReturnCode SilKitCALL SilKit_TimeSyncService_Now(SilKit_TimeSyncService* timeSyncService,
                                                            SilKit_NanosecondsTime* outNanosecondsTime)
    {
        return globalCapi->SilKit_TimeSyncService_Now(timeSyncService, outNanosecondsTime);
    }

    // SystemMonitor

    SilKit_ReturnCode SilKitCALL SilKit_SystemMonitor_Create(SilKit_SystemMonitor** outSystemMonitor,
                                                             SilKit_Participant* participant)
    {
        return globalCapi->SilKit_SystemMonitor_Create(outSystemMonitor, participant);
    }

    SilKit_ReturnCode SilKitCALL SilKit_SystemMonitor_GetParticipantStatus(
        SilKit_ParticipantStatus* outParticipantState, SilKit_SystemMonitor* systemMonitor, const char* participantName)
    {
        return globalCapi->SilKit_SystemMonitor_GetParticipantStatus(outParticipantState, systemMonitor,
                                                                     participantName);
    }

    SilKit_ReturnCode SilKitCALL SilKit_SystemMonitor_GetSystemState(SilKit_SystemState* outSystemState,
                                                                     SilKit_SystemMonitor* systemMonitor)
    {
        return globalCapi->SilKit_SystemMonitor_GetSystemState(outSystemState, systemMonitor);
    }

    SilKit_ReturnCode SilKitCALL SilKit_SystemMonitor_AddSystemStateHandler(SilKit_SystemMonitor* systemMonitor,
                                                                            void* context,
                                                                            SilKit_SystemStateHandler_t handler,
                                                                            SilKit_HandlerId* outHandlerId)
    {
        return globalCapi->SilKit_SystemMonitor_AddSystemStateHandler(systemMonitor, context, handler, outHandlerId);
    }

    SilKit_ReturnCode SilKitCALL SilKit_SystemMonitor_RemoveSystemStateHandler(SilKit_SystemMonitor* systemMonitor,
                                                                               SilKit_HandlerId handlerId)
    {
        return globalCapi->SilKit_SystemMonitor_RemoveSystemStateHandler(systemMonitor, handlerId);
    }

    SilKit_ReturnCode SilKitCALL SilKit_SystemMonitor_AddParticipantStatusHandler(
        SilKit_SystemMonitor* systemMonitor, void* context, SilKit_ParticipantStatusHandler_t handler,
        SilKit_HandlerId* outHandlerId)
    {
        return globalCapi->SilKit_SystemMonitor_AddParticipantStatusHandler(systemMonitor, context, handler,
                                                                            outHandlerId);
    }

    SilKit_ReturnCode SilKitCALL
    SilKit_SystemMonitor_RemoveParticipantStatusHandler(SilKit_SystemMonitor* systemMonitor, SilKit_HandlerId handlerId)
    {
        return globalCapi->SilKit_SystemMonitor_RemoveParticipantStatusHandler(systemMonitor, handlerId);
    }

    SilKit_ReturnCode SilKitCALL SilKit_SystemMonitor_SetParticipantConnectedHandler(
        SilKit_SystemMonitor* systemMonitor, void* context, SilKit_SystemMonitor_ParticipantConnectedHandler_t handler)
    {
        return globalCapi->SilKit_SystemMonitor_SetParticipantConnectedHandler(systemMonitor, context, handler);
    }

    SilKit_ReturnCode SilKitCALL SilKit_SystemMonitor_SetParticipantDisconnectedHandler(
        SilKit_SystemMonitor* systemMonitor, void* context,
        SilKit_SystemMonitor_ParticipantDisconnectedHandler_t handler)
    {
        return globalCapi->SilKit_SystemMonitor_SetParticipantDisconnectedHandler(systemMonitor, context, handler);
    }

    SilKit_ReturnCode SilKitCALL SilKit_SystemMonitor_IsParticipantConnected(SilKit_SystemMonitor* systemMonitor,
                                                                             const char* participantName,
                                                                             SilKit_Bool* out)
    {
        return globalCapi->SilKit_SystemMonitor_IsParticipantConnected(systemMonitor, participantName, out);
    }

    // SystemController

    SilKit_ReturnCode SilKitCALL SilKit_Experimental_SystemController_Create(
        SilKit_Experimental_SystemController** outSystemController, SilKit_Participant* participant)
    {
        return globalCapi->SilKit_Experimental_SystemController_Create(outSystemController, participant);
    }

    SilKit_ReturnCode SilKitCALL
    SilKit_Experimental_SystemController_AbortSimulation(SilKit_Experimental_SystemController* systemController)
    {
        return globalCapi->SilKit_Experimental_SystemController_AbortSimulation(systemController);
    }

    SilKit_ReturnCode SilKitCALL SilKit_Experimental_SystemController_SetWorkflowConfiguration(
        SilKit_Experimental_SystemController* systemController,
        const SilKit_WorkflowConfiguration* workflowConfiguration)
    {
        return globalCapi->SilKit_Experimental_SystemController_SetWorkflowConfiguration(systemController,
                                                                                         workflowConfiguration);
    }

    // Participant

    SilKit_ReturnCode SilKitCALL SilKit_Participant_Create(SilKit_Participant** outParticipant,
                                                           SilKit_ParticipantConfiguration* participantConfiguration,
                                                           const char* participantName, const char* registryUri)
    {
        return globalCapi->SilKit_Participant_Create(outParticipant, participantConfiguration, participantName,
                                                     registryUri);
    }

    SilKit_ReturnCode SilKitCALL SilKit_Participant_Destroy(SilKit_Participant* participant)
    {
        return globalCapi->SilKit_Participant_Destroy(participant);
    }

    SilKit_ReturnCode SilKitCALL SilKit_Participant_GetLogger(SilKit_Logger** outLogger,
                                                              SilKit_Participant* participant)
    {
        return globalCapi->SilKit_Participant_GetLogger(outLogger, participant);
    }

    // ParticipantConfiguration

    SilKit_ReturnCode SilKitCALL SilKit_ParticipantConfiguration_FromString(
        SilKit_ParticipantConfiguration** outParticipantConfiguration, const char* participantConfigurationString)
    {
        return globalCapi->SilKit_ParticipantConfiguration_FromString(outParticipantConfiguration,
                                                                      participantConfigurationString);
    }

    SilKit_ReturnCode SilKitCALL SilKit_ParticipantConfiguration_FromFile(
        SilKit_ParticipantConfiguration** outParticipantConfiguration, const char* participantConfigurationFilename)
    {
        return globalCapi->SilKit_ParticipantConfiguration_FromFile(outParticipantConfiguration,
                                                                      participantConfigurationFilename);
    }

    SilKit_ReturnCode SilKitCALL
    SilKit_ParticipantConfiguration_Destroy(SilKit_ParticipantConfiguration* participantConfiguration)
    {
        return globalCapi->SilKit_ParticipantConfiguration_Destroy(participantConfiguration);
    }

    // Logger

    SilKit_ReturnCode SilKitCALL SilKit_Logger_Log(SilKit_Logger* logger, SilKit_LoggingLevel level,
                                                   const char* message)
    {
        return globalCapi->SilKit_Logger_Log(logger, level, message);
    }

    SilKit_ReturnCode SilKitCALL SilKit_Logger_GetLogLevel(SilKit_Logger* logger, SilKit_LoggingLevel* outLevel)
    {
        return globalCapi->SilKit_Logger_GetLogLevel(logger, outLevel);
    }

    // DataPublisher

    SilKit_ReturnCode SilKitCALL SilKit_DataPublisher_Create(SilKit_DataPublisher** outPublisher,
                                                             SilKit_Participant* participant,
                                                             const char* controllerName, SilKit_DataSpec* dataSpec,
                                                             uint8_t history)
    {
        return globalCapi->SilKit_DataPublisher_Create(outPublisher, participant, controllerName, dataSpec, history);
    }

    SilKit_ReturnCode SilKitCALL SilKit_DataPublisher_Publish(SilKit_DataPublisher* self, const SilKit_ByteVector* data)
    {
        return globalCapi->SilKit_DataPublisher_Publish(self, data);
    }

    // DataSubscriber

    SilKit_ReturnCode SilKitCALL SilKit_DataSubscriber_Create(SilKit_DataSubscriber** outSubscriber,
                                                              SilKit_Participant* participant,
                                                              const char* controllerName, SilKit_DataSpec* dataSpec,
                                                              void* dataHandlerContext,
                                                              SilKit_DataMessageHandler_t dataHandler)
    {
        return globalCapi->SilKit_DataSubscriber_Create(outSubscriber, participant, controllerName, dataSpec,
                                                        dataHandlerContext, dataHandler);
    }

    SilKit_ReturnCode SilKitCALL SilKit_DataSubscriber_SetDataMessageHandler(SilKit_DataSubscriber* self, void* context,
                                                                             SilKit_DataMessageHandler_t dataHandler)
    {
        return globalCapi->SilKit_DataSubscriber_SetDataMessageHandler(self, context, dataHandler);
    }

    // RpcServer

    SilKit_ReturnCode SilKitCALL SilKit_RpcServer_Create(SilKit_RpcServer** outServer, SilKit_Participant* participant,
                                                         const char* controllerName, SilKit_RpcSpec* rpcSpec,
                                                         void* callHandlerContext, SilKit_RpcCallHandler_t callHandler)
    {
        return globalCapi->SilKit_RpcServer_Create(outServer, participant, controllerName, rpcSpec, callHandlerContext,
                                                   callHandler);
    }

    SilKit_ReturnCode SilKitCALL SilKit_RpcServer_SubmitResult(SilKit_RpcServer* self, SilKit_RpcCallHandle* callHandle,
                                                               const SilKit_ByteVector* returnData)
    {
        return globalCapi->SilKit_RpcServer_SubmitResult(self, callHandle, returnData);
    }

    SilKit_ReturnCode SilKitCALL SilKit_RpcServer_SetCallHandler(SilKit_RpcServer* self, void* context,
                                                                 SilKit_RpcCallHandler_t handler)
    {
        return globalCapi->SilKit_RpcServer_SetCallHandler(self, context, handler);
    }

    // RpcClient

    SilKit_ReturnCode SilKitCALL SilKit_RpcClient_Create(SilKit_RpcClient** outClient, SilKit_Participant* participant,
                                                         const char* controllerName, SilKit_RpcSpec* rpcSpec,
                                                         void* resultHandlerContext,
                                                         SilKit_RpcCallResultHandler_t resultHandler)
    {
        return globalCapi->SilKit_RpcClient_Create(outClient, participant, controllerName, rpcSpec,
                                                   resultHandlerContext, resultHandler);
    }

    SilKit_ReturnCode SilKitCALL SilKit_RpcClient_Call(SilKit_RpcClient* self, const SilKit_ByteVector* argumentData,
                                                       void* userContext)
    {
        return globalCapi->SilKit_RpcClient_Call(self, argumentData, userContext);
    }

    SilKit_ReturnCode SilKitCALL SilKit_RpcClient_CallWithTimeout(SilKit_RpcClient* self,
                                                                  const SilKit_ByteVector* argumentData,
                                                                  SilKit_NanosecondsTime timeout, void* userContext)
    {
        return globalCapi->SilKit_RpcClient_CallWithTimeout(self, argumentData, timeout, userContext);
    }

    SilKit_ReturnCode SilKitCALL SilKit_RpcClient_SetCallResultHandler(SilKit_RpcClient* self, void* context,
                                                                       SilKit_RpcCallResultHandler_t handler)
    {
        return globalCapi->SilKit_RpcClient_SetCallResultHandler(self, context, handler);
    }

    // SilKitRegistry

    SilKit_ReturnCode SilKitCALL SilKit_Vendor_Vector_SilKitRegistry_Create(
        SilKit_Vendor_Vector_SilKitRegistry** outRegistry, SilKit_ParticipantConfiguration* participantConfiguration)
    {
        return globalCapi->SilKit_Vendor_Vector_SilKitRegistry_Create(outRegistry, participantConfiguration);
    }

    SilKit_ReturnCode SilKitCALL
    SilKit_Vendor_Vector_SilKitRegistry_Destroy(SilKit_Vendor_Vector_SilKitRegistry* registry)
    {
        return globalCapi->SilKit_Vendor_Vector_SilKitRegistry_Destroy(registry);
    }

    SilKit_ReturnCode SilKitCALL SilKit_Vendor_Vector_SilKitRegistry_SetAllDisconnectedHandler(
        SilKit_Vendor_Vector_SilKitRegistry* registry, void* context,
        SilKit_Vendor_Vector_SilKitRegistry_AllDisconnectedHandler_t handler)
    {
        return globalCapi->SilKit_Vendor_Vector_SilKitRegistry_SetAllDisconnectedHandler(registry, context, handler);
    }

    SilKit_ReturnCode SilKitCALL SilKit_Vendor_Vector_SilKitRegistry_GetLogger(
        SilKit_Logger** outLogger, SilKit_Vendor_Vector_SilKitRegistry* registry)
    {
        return globalCapi->SilKit_Vendor_Vector_SilKitRegistry_GetLogger(outLogger, registry);
    }

    SilKit_ReturnCode SilKitCALL SilKit_Vendor_Vector_SilKitRegistry_StartListening(
        SilKit_Vendor_Vector_SilKitRegistry* registry, const char* listenUri, const char** outRegistryUri)
    {
        return globalCapi->SilKit_Vendor_Vector_SilKitRegistry_StartListening(registry, listenUri, outRegistryUri);
    }

    // Version

    SilKit_ReturnCode SilKitCALL SilKit_Version_Major(uint32_t* outVersionMajor)
    {
        return globalCapi->SilKit_Version_Major(outVersionMajor);
    }

    SilKit_ReturnCode SilKitCALL SilKit_Version_Minor(uint32_t* outVersionMinor)
    {
        return globalCapi->SilKit_Version_Minor(outVersionMinor);
    }

    SilKit_ReturnCode SilKitCALL SilKit_Version_Patch(uint32_t* outVersionPatch)
    {
        return globalCapi->SilKit_Version_Patch(outVersionPatch);
    }

    SilKit_ReturnCode SilKitCALL SilKit_Version_BuildNumber(uint32_t* outVersionBuildNumber)
    {
        return globalCapi->SilKit_Version_BuildNumber(outVersionBuildNumber);
    }

    SilKit_ReturnCode SilKitCALL SilKit_Version_String(const char** outVersionString)
    {
        return globalCapi->SilKit_Version_String(outVersionString);
    }

    SilKit_ReturnCode SilKitCALL SilKit_Version_VersionSuffix(const char** outVersionVersionSuffix)
    {
        return globalCapi->SilKit_Version_VersionSuffix(outVersionVersionSuffix);
    }

    SilKit_ReturnCode SilKitCALL SilKit_Version_GitHash(const char** outVersionGitHash)
    {
        return globalCapi->SilKit_Version_GitHash(outVersionGitHash);
    }

    // Network Simulator

    SilKit_ReturnCode SilKitCALL SilKit_Experimental_NetworkSimulator_Create(
        SilKit_Experimental_NetworkSimulator** outNetworkSimulator, SilKit_Participant* participant)
    {
        return globalCapi->SilKit_Experimental_NetworkSimulator_Create(outNetworkSimulator, participant);
    }

    SilKit_ReturnCode SilKitCALL SilKit_Experimental_NetworkSimulator_SimulateNetwork(
        SilKit_Experimental_NetworkSimulator* networkSimulator, const char* networkName,
        SilKit_Experimental_SimulatedNetworkType networkType, void* simulatedNetwork,
        const SilKit_Experimental_SimulatedNetworkFunctions* simulatedNetworkFunctions)
    {
        return globalCapi->SilKit_Experimental_NetworkSimulator_SimulateNetwork(
            networkSimulator, networkName, networkType, simulatedNetwork, simulatedNetworkFunctions);
    }

    SilKit_ReturnCode SilKitCALL
    SilKit_Experimental_NetworkSimulator_Start(SilKit_Experimental_NetworkSimulator* networkSimulator)
    {
        return globalCapi->SilKit_Experimental_NetworkSimulator_Start(networkSimulator);
    }

    // CanEventProducer

    SilKit_ReturnCode SilKitCALL SilKit_Experimental_CanEventProducer_Produce(
        SilKit_Experimental_CanEventProducer* eventProducer, SilKit_StructHeader* cEvent,
        const SilKit_Experimental_EventReceivers* receivers)
    {
        return globalCapi->SilKit_Experimental_CanEventProducer_Produce(eventProducer, cEvent, receivers);
    }

    // FlexRayEventProducer

    SilKit_ReturnCode SilKitCALL SilKit_Experimental_FlexRayEventProducer_Produce(
        SilKit_Experimental_FlexRayEventProducer* eventProducer, SilKit_StructHeader* cEvent,
        const SilKit_Experimental_EventReceivers* receivers)
    {
        return globalCapi->SilKit_Experimental_FlexRayEventProducer_Produce(eventProducer, cEvent, receivers);
    }

    // EthEventProducer

    SilKit_ReturnCode SilKitCALL SilKit_Experimental_EthernetEventProducer_Produce(
        SilKit_Experimental_EthernetEventProducer* eventProducer, SilKit_StructHeader* cEvent,
        const SilKit_Experimental_EventReceivers* receivers)
    {
        return globalCapi->SilKit_Experimental_EthernetEventProducer_Produce(eventProducer, cEvent, receivers);
    }


    // LinEventProducer

    SilKit_ReturnCode SilKitCALL SilKit_Experimental_LinEventProducer_Produce(
        SilKit_Experimental_LinEventProducer* eventProducer, SilKit_StructHeader* cEvent,
        const SilKit_Experimental_EventReceivers* receivers)
    {
        return globalCapi->SilKit_Experimental_LinEventProducer_Produce(eventProducer, cEvent, receivers);
    }
}
