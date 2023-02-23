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

#include <string>
#include <cstring>

#include "silkit/capi/SilKit.h"
#include "silkit/SilKit.hpp"
#include "silkit/services/flexray/all.hpp"

#include "IParticipantInternal.hpp"
#include "CapiImpl.hpp"


namespace {

void assign(SilKit::Services::Flexray::FlexrayTxBufferConfig& cppConfig, const SilKit_FlexrayTxBufferConfig* config)
{
    cppConfig.channels = (SilKit::Services::Flexray::FlexrayChannel)config->channels;
    cppConfig.slotId = config->slotId;
    cppConfig.offset = config->offset;
    cppConfig.repetition = config->repetition;
    cppConfig.hasPayloadPreambleIndicator = config->hasPayloadPreambleIndicator == SilKit_True;
    cppConfig.headerCrc = config->headerCrc;
    cppConfig.transmissionMode = (SilKit::Services::Flexray::FlexrayTransmissionMode)config->transmissionMode;
}

void assign(SilKit::Services::Flexray::FlexrayClusterParameters& cppClusterParameters,
                   const SilKit_FlexrayClusterParameters* clusterParameters)
{
    cppClusterParameters.gColdstartAttempts = clusterParameters->gColdstartAttempts;
    cppClusterParameters.gCycleCountMax = clusterParameters->gCycleCountMax;
    cppClusterParameters.gdActionPointOffset = clusterParameters->gdActionPointOffset;
    cppClusterParameters.gdDynamicSlotIdlePhase = clusterParameters->gdDynamicSlotIdlePhase;
    cppClusterParameters.gdMiniSlot = clusterParameters->gdMiniSlot;
    cppClusterParameters.gdMiniSlotActionPointOffset = clusterParameters->gdMiniSlotActionPointOffset;
    cppClusterParameters.gdStaticSlot = clusterParameters->gdStaticSlot;
    cppClusterParameters.gdSymbolWindow = clusterParameters->gdSymbolWindow;
    cppClusterParameters.gdSymbolWindowActionPointOffset = clusterParameters->gdSymbolWindowActionPointOffset;
    cppClusterParameters.gdTSSTransmitter = clusterParameters->gdTSSTransmitter;
    cppClusterParameters.gdWakeupTxActive = clusterParameters->gdWakeupTxActive;
    cppClusterParameters.gdWakeupTxIdle = clusterParameters->gdWakeupTxIdle;
    cppClusterParameters.gListenNoise = clusterParameters->gListenNoise;
    cppClusterParameters.gMacroPerCycle = clusterParameters->gMacroPerCycle;
    cppClusterParameters.gMaxWithoutClockCorrectionFatal = clusterParameters->gMaxWithoutClockCorrectionFatal;
    cppClusterParameters.gMaxWithoutClockCorrectionPassive = clusterParameters->gMaxWithoutClockCorrectionPassive;
    cppClusterParameters.gNumberOfMiniSlots = clusterParameters->gNumberOfMiniSlots;
    cppClusterParameters.gNumberOfStaticSlots = clusterParameters->gNumberOfStaticSlots;
    cppClusterParameters.gPayloadLengthStatic = clusterParameters->gPayloadLengthStatic;
    cppClusterParameters.gSyncFrameIDCountMax = clusterParameters->gSyncFrameIDCountMax;
}

void assign(SilKit::Services::Flexray::FlexrayNodeParameters& cppNodeParameters,
                   const SilKit_FlexrayNodeParameters* nodeParameters)
{
    cppNodeParameters.pAllowHaltDueToClock = nodeParameters->pAllowHaltDueToClock;
    cppNodeParameters.pAllowPassiveToActive = nodeParameters->pAllowPassiveToActive;
    cppNodeParameters.pChannels = (SilKit::Services::Flexray::FlexrayChannel)nodeParameters->pChannels;
    cppNodeParameters.pClusterDriftDamping = nodeParameters->pClusterDriftDamping;
    cppNodeParameters.pdAcceptedStartupRange = nodeParameters->pdAcceptedStartupRange;
    cppNodeParameters.pdListenTimeout = nodeParameters->pdListenTimeout;
    cppNodeParameters.pKeySlotId = nodeParameters->pKeySlotId;
    cppNodeParameters.pKeySlotOnlyEnabled = nodeParameters->pKeySlotOnlyEnabled;
    cppNodeParameters.pKeySlotUsedForStartup = nodeParameters->pKeySlotUsedForStartup;
    cppNodeParameters.pKeySlotUsedForSync = nodeParameters->pKeySlotUsedForSync;
    cppNodeParameters.pLatestTx = nodeParameters->pLatestTx;
    cppNodeParameters.pMacroInitialOffsetA = nodeParameters->pMacroInitialOffsetA;
    cppNodeParameters.pMacroInitialOffsetB = nodeParameters->pMacroInitialOffsetB;
    cppNodeParameters.pMicroInitialOffsetA = nodeParameters->pMicroInitialOffsetA;
    cppNodeParameters.pMicroInitialOffsetB = nodeParameters->pMicroInitialOffsetB;
    cppNodeParameters.pMicroPerCycle = nodeParameters->pMicroPerCycle;
    cppNodeParameters.pOffsetCorrectionOut = nodeParameters->pOffsetCorrectionOut;
    cppNodeParameters.pOffsetCorrectionStart = nodeParameters->pOffsetCorrectionStart;
    cppNodeParameters.pRateCorrectionOut = nodeParameters->pRateCorrectionOut;
    cppNodeParameters.pWakeupChannel = (SilKit::Services::Flexray::FlexrayChannel)nodeParameters->pWakeupChannel;
    cppNodeParameters.pWakeupPattern = nodeParameters->pWakeupPattern;
    cppNodeParameters.pdMicrotick = (SilKit::Services::Flexray::FlexrayClockPeriod)nodeParameters->pdMicrotick;
    cppNodeParameters.pSamplesPerMicrotick = nodeParameters->pSamplesPerMicrotick;
}

void assign(SilKit::Services::Flexray::FlexrayControllerConfig& cppConfig, const SilKit_FlexrayControllerConfig* config)
{
    assign(cppConfig.clusterParams, config->clusterParams);
    assign(cppConfig.nodeParams, config->nodeParams);

    for (uint32_t i = 0; i < config->numBufferConfigs; i++)
    {
        SilKit::Services::Flexray::FlexrayTxBufferConfig txBufferConfig{};
        assign(txBufferConfig, &config->bufferConfigs[i]);
        cppConfig.bufferConfigs.push_back(std::move(txBufferConfig));
    }
}

}//namespace


SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_Create(SilKit_FlexrayController** outController, SilKit_Participant* participant,
                                           const char* name, const char* network)
try
{
    ASSERT_VALID_OUT_PARAMETER(outController);
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_POINTER_PARAMETER(name);
    ASSERT_VALID_POINTER_PARAMETER(network);

    auto cppParticipant = reinterpret_cast<SilKit::IParticipant*>(participant);
    auto controller = cppParticipant->CreateFlexrayController(name, network);
    if (controller == nullptr)
    {
        return SilKit_ReturnCode_UNSPECIFIEDERROR;
    }
    *outController = reinterpret_cast<SilKit_FlexrayController*>(controller);
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_Configure(SilKit_FlexrayController* controller,
                                              const SilKit_FlexrayControllerConfig* config)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_POINTER_PARAMETER(config);
    ASSERT_VALID_POINTER_PARAMETER(config->clusterParams);
    ASSERT_VALID_POINTER_PARAMETER(config->nodeParams);
    // Versioning checks
    ASSERT_VALID_STRUCT_HEADER(config);
    ASSERT_VALID_STRUCT_HEADER(config->clusterParams);
    ASSERT_VALID_STRUCT_HEADER(config->nodeParams);

    const auto cppController = reinterpret_cast<SilKit::Services::Flexray::IFlexrayController*>(controller);
    SilKit::Services::Flexray::FlexrayControllerConfig cppConfig;
    assign(cppConfig, config);

    cppController->Configure(cppConfig);
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_ReconfigureTxBuffer(SilKit_FlexrayController* controller, uint16_t txBufferIdx,
                                                        const SilKit_FlexrayTxBufferConfig* config)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_POINTER_PARAMETER(config);
    ASSERT_VALID_STRUCT_HEADER(config);
    ASSERT_VALID_BOOL_PARAMETER(config->hasPayloadPreambleIndicator);

    const auto cppController = reinterpret_cast<SilKit::Services::Flexray::IFlexrayController*>(controller);
    SilKit::Services::Flexray::FlexrayTxBufferConfig cppConfig{};
    assign(cppConfig, config);

    cppController->ReconfigureTxBuffer(txBufferIdx, cppConfig);
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_UpdateTxBuffer(SilKit_FlexrayController* controller,
                                                   const SilKit_FlexrayTxBufferUpdate* update)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_POINTER_PARAMETER(update);
    ASSERT_VALID_BOOL_PARAMETER(update->payloadDataValid);

    const auto cppController = reinterpret_cast<SilKit::Services::Flexray::IFlexrayController*>(controller);
    SilKit::Services::Flexray::FlexrayTxBufferUpdate cppUpdate;
    cppUpdate.txBufferIndex = update->txBufferIndex;
    cppUpdate.payloadDataValid = update->payloadDataValid == SilKit_True;
    if (update->payloadDataValid)
    {
        ASSERT_VALID_POINTER_PARAMETER(update->payload.data);
        cppUpdate.payload = SilKit::Util::ToSpan(update->payload);
    }
    cppController->UpdateTxBuffer(cppUpdate);
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_ExecuteCmd(SilKit_FlexrayController* controller, SilKit_FlexrayChiCommand cmd)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);

    const auto cppController = reinterpret_cast<SilKit::Services::Flexray::IFlexrayController*>(controller);
    switch (cmd)
    {
    case SilKit_FlexrayChiCommand_RUN:
        cppController->Run();
        break;
    case SilKit_FlexrayChiCommand_DEFERRED_HALT:
        cppController->DeferredHalt();
        break;
    case SilKit_FlexrayChiCommand_FREEZE:
        cppController->Freeze();
        break;
    case SilKit_FlexrayChiCommand_ALLOW_COLDSTART:
        cppController->AllowColdstart();
        break;
    case SilKit_FlexrayChiCommand_ALL_SLOTS:
        cppController->AllSlots();
        break;
    case SilKit_FlexrayChiCommand_WAKEUP:
        cppController->Wakeup();
        break;
    default:
        return SilKit_ReturnCode_BADPARAMETER;
    }
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_AddFrameHandler(SilKit_FlexrayController* controller, void* context,
                                                    SilKit_FlexrayFrameHandler_t handler, SilKit_HandlerId* outHandlerId)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_HANDLER_PARAMETER(handler);
    ASSERT_VALID_OUT_PARAMETER(outHandlerId);

    const auto cppController = reinterpret_cast<SilKit::Services::Flexray::IFlexrayController*>(controller);
    *outHandlerId = (SilKit_HandlerId)cppController->AddFrameHandler(
        [context, handler](SilKit::Services::Flexray::IFlexrayController* ctrl, const SilKit::Services::Flexray::FlexrayFrameEvent& msg) {
            SilKit_FlexrayFrameEvent message;
            SilKit_FlexrayFrame frame;
            SilKit_FlexrayHeader header;

            SilKit_Struct_Init(SilKit_FlexrayFrameEvent, message);
            SilKit_Struct_Init(SilKit_FlexrayFrame, frame);
            SilKit_Struct_Init(SilKit_FlexrayHeader, header);

            header.cycleCount = msg.frame.header.cycleCount;
            header.frameId = msg.frame.header.frameId;
            header.flags = msg.frame.header.flags;
            header.headerCrc = msg.frame.header.headerCrc;
            header.payloadLength = msg.frame.header.payloadLength;

            frame.header = &header;
            frame.payload.data = (uint8_t*)msg.frame.payload.data();
            frame.payload.size = (uint32_t)msg.frame.payload.size();

            message.timestamp = msg.timestamp.count();
            message.channel = (SilKit_FlexrayChannel)msg.channel;
            message.frame = &frame;

            handler(context, ctrl, &message);
        });
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_RemoveFrameHandler(SilKit_FlexrayController* controller, SilKit_HandlerId handlerId)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);

    auto cppController = reinterpret_cast<SilKit::Services::Flexray::IFlexrayController*>(controller);
    cppController->RemoveFrameHandler(static_cast<SilKit::Util::HandlerId>(handlerId));
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_AddFrameTransmitHandler(SilKit_FlexrayController* controller, void* context,
                                                            SilKit_FlexrayFrameTransmitHandler_t handler,
                                                            SilKit_HandlerId* outHandlerId)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_HANDLER_PARAMETER(handler);
    ASSERT_VALID_OUT_PARAMETER(outHandlerId);

    const auto cppController = reinterpret_cast<SilKit::Services::Flexray::IFlexrayController*>(controller);
    *outHandlerId = (SilKit_HandlerId)cppController->AddFrameTransmitHandler(
        [context, handler](SilKit::Services::Flexray::IFlexrayController* ctrl,
                           const SilKit::Services::Flexray::FlexrayFrameTransmitEvent& msg) {
            SilKit_FlexrayFrameTransmitEvent message;
            SilKit_FlexrayFrame frame;
            SilKit_FlexrayHeader header;

            SilKit_Struct_Init(SilKit_FlexrayFrameEvent, message);
            SilKit_Struct_Init(SilKit_FlexrayFrame, frame);
            SilKit_Struct_Init(SilKit_FlexrayHeader, header);

            header.cycleCount = msg.frame.header.cycleCount;
            header.frameId = msg.frame.header.frameId;
            header.flags = msg.frame.header.flags;
            header.headerCrc = msg.frame.header.headerCrc;
            header.payloadLength = msg.frame.header.payloadLength;

            frame.payload.data = (uint8_t*)msg.frame.payload.data();
            frame.payload.size = (uint32_t)msg.frame.payload.size();
            frame.header = &header;

            message.timestamp = msg.timestamp.count();
            message.txBufferIndex = msg.txBufferIndex;
            message.channel = (SilKit_FlexrayChannel)msg.channel;
            message.frame = &frame;
            handler(context, ctrl, &message);
        });
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_RemoveFrameTransmitHandler(SilKit_FlexrayController* controller,
                                                               SilKit_HandlerId handlerId)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);

    auto cppController = reinterpret_cast<SilKit::Services::Flexray::IFlexrayController*>(controller);
    cppController->RemoveFrameTransmitHandler(static_cast<SilKit::Util::HandlerId>(handlerId));
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_AddWakeupHandler(SilKit_FlexrayController* controller, void* context,
                                                     SilKit_FlexrayWakeupHandler_t handler, SilKit_HandlerId* outHandlerId)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_HANDLER_PARAMETER(handler);
    ASSERT_VALID_OUT_PARAMETER(outHandlerId);

    const auto cppController = reinterpret_cast<SilKit::Services::Flexray::IFlexrayController*>(controller);
    *outHandlerId = (SilKit_HandlerId)cppController->AddWakeupHandler(
        [context, handler](SilKit::Services::Flexray::IFlexrayController* ctrl, const SilKit::Services::Flexray::FlexrayWakeupEvent& msg) {
            SilKit_FlexrayWakeupEvent message;
            SilKit_Struct_Init(SilKit_FlexrayWakeupEvent, message);

            message.timestamp = msg.timestamp.count();
            message.channel = (SilKit_FlexrayChannel)msg.channel;
            message.pattern = (SilKit_FlexraySymbolPattern)msg.pattern;
            handler(context, ctrl, &message);
        });
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_RemoveWakeupHandler(SilKit_FlexrayController* controller, SilKit_HandlerId handlerId)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);

    auto cppController = reinterpret_cast<SilKit::Services::Flexray::IFlexrayController*>(controller);
    cppController->RemoveWakeupHandler(static_cast<SilKit::Util::HandlerId>(handlerId));
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_AddPocStatusHandler(SilKit_FlexrayController* controller, void* context,
                                                        SilKit_FlexrayPocStatusHandler_t handler,
                                                        SilKit_HandlerId* outHandlerId)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_HANDLER_PARAMETER(handler);
    ASSERT_VALID_OUT_PARAMETER(outHandlerId);

    const auto cppController = reinterpret_cast<SilKit::Services::Flexray::IFlexrayController*>(controller);
    *outHandlerId = (SilKit_HandlerId)cppController->AddPocStatusHandler(
        [context, handler](SilKit::Services::Flexray::IFlexrayController* ctrl,
                           const SilKit::Services::Flexray::FlexrayPocStatusEvent& msg) {
            SilKit_FlexrayPocStatusEvent message;
            SilKit_Struct_Init(SilKit_FlexrayPocStatusEvent, message);
            message.timestamp = msg.timestamp.count();
            message.state = (SilKit_FlexrayPocState)msg.state;
            message.chiHaltRequest = msg.chiHaltRequest;
            message.coldstartNoise = msg.coldstartNoise;
            message.freeze = msg.freeze;
            message.chiReadyRequest = msg.chiReadyRequest;
            message.errorMode = (SilKit_FlexrayErrorModeType)msg.errorMode;
            message.slotMode = (SilKit_FlexraySlotModeType)msg.slotMode;
            message.startupState = (SilKit_FlexrayStartupStateType)msg.startupState;
            message.wakeupStatus = (SilKit_FlexrayWakeupStatusType)msg.wakeupStatus;
            handler(context, ctrl, &message);
        });
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_RemovePocStatusHandler(SilKit_FlexrayController* controller,
                                                           SilKit_HandlerId handlerId)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);

    auto cppController = reinterpret_cast<SilKit::Services::Flexray::IFlexrayController*>(controller);
    cppController->RemovePocStatusHandler(static_cast<SilKit::Util::HandlerId>(handlerId));
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_AddSymbolHandler(SilKit_FlexrayController* controller, void* context,
                                                     SilKit_FlexraySymbolHandler_t handler, SilKit_HandlerId* outHandlerId)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_HANDLER_PARAMETER(handler);
    ASSERT_VALID_OUT_PARAMETER(outHandlerId);

    const auto cppController = reinterpret_cast<SilKit::Services::Flexray::IFlexrayController*>(controller);
    *outHandlerId = (SilKit_HandlerId)cppController->AddSymbolHandler(
        [context, handler](SilKit::Services::Flexray::IFlexrayController* ctrl, const SilKit::Services::Flexray::FlexraySymbolEvent& msg) {
            SilKit_FlexraySymbolEvent message;
            SilKit_Struct_Init(SilKit_FlexraySymbolEvent, message);
            message.timestamp = msg.timestamp.count();
            message.channel = (SilKit_FlexrayChannel)msg.channel;
            message.pattern = (SilKit_FlexraySymbolPattern)msg.pattern;
            handler(context, ctrl, &message);
        });
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_RemoveSymbolHandler(SilKit_FlexrayController* controller, SilKit_HandlerId handlerId)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);

    auto cppController = reinterpret_cast<SilKit::Services::Flexray::IFlexrayController*>(controller);
    cppController->RemoveSymbolHandler(static_cast<SilKit::Util::HandlerId>(handlerId));
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_AddSymbolTransmitHandler(SilKit_FlexrayController* controller, void* context,
                                                             SilKit_FlexraySymbolTransmitHandler_t handler,
                                                             SilKit_HandlerId* outHandlerId)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_HANDLER_PARAMETER(handler);
    ASSERT_VALID_OUT_PARAMETER(outHandlerId);

    const auto cppController = reinterpret_cast<SilKit::Services::Flexray::IFlexrayController*>(controller);
    *outHandlerId = (SilKit_HandlerId)cppController->AddSymbolTransmitHandler(
        [context, handler](SilKit::Services::Flexray::IFlexrayController* ctrl,
                           const SilKit::Services::Flexray::FlexraySymbolTransmitEvent& msg) {
            SilKit_FlexraySymbolTransmitEvent message;
            SilKit_Struct_Init(SilKit_FlexraySymbolTransmitEvent, message);
            message.timestamp = msg.timestamp.count();
            message.channel = (SilKit_FlexrayChannel)msg.channel;
            message.pattern = (SilKit_FlexraySymbolPattern)msg.pattern;
            handler(context, ctrl, &message);
        });
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_RemoveSymbolTransmitHandler(SilKit_FlexrayController* controller,
                                                                SilKit_HandlerId handlerId)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);

    auto cppController = reinterpret_cast<SilKit::Services::Flexray::IFlexrayController*>(controller);
    cppController->RemoveSymbolTransmitHandler(static_cast<SilKit::Util::HandlerId>(handlerId));
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_AddCycleStartHandler(SilKit_FlexrayController* controller, void* context,
                                                         SilKit_FlexrayCycleStartHandler_t handler,
                                                         SilKit_HandlerId* outHandlerId)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_HANDLER_PARAMETER(handler);
    ASSERT_VALID_OUT_PARAMETER(outHandlerId);

    const auto cppController = reinterpret_cast<SilKit::Services::Flexray::IFlexrayController*>(controller);
    *outHandlerId = (SilKit_HandlerId)cppController->AddCycleStartHandler(
        [context, handler](SilKit::Services::Flexray::IFlexrayController* ctrl,
                           const SilKit::Services::Flexray::FlexrayCycleStartEvent& msg) {
            SilKit_FlexrayCycleStartEvent message;
            SilKit_Struct_Init(SilKit_FlexrayCycleStartEvent, message);
            message.timestamp = msg.timestamp.count();
            message.cycleCounter = msg.cycleCounter;
            handler(context, ctrl, &message);
        });
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_RemoveCycleStartHandler(SilKit_FlexrayController* controller,
                                                            SilKit_HandlerId handlerId)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);

    auto cppController = reinterpret_cast<SilKit::Services::Flexray::IFlexrayController*>(controller);
    cppController->RemoveCycleStartHandler(static_cast<SilKit::Util::HandlerId>(handlerId));
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS
