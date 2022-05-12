/* Copyright (c) Vector Informatik GmbH. All rights reserved. */

#include <string>
#include <string.h>

#include "ib/capi/IntegrationBus.h"
#include "ib/IntegrationBus.hpp"
#include "ib/sim/fr/all.hpp"

#include "IParticipantInternal.hpp"
#include "CapiImpl.h"
#include "ParticipantConfiguration.hpp"

static void assign(ib::sim::fr::FlexrayTxBufferConfig& cppConfig, const ib_Flexray_TxBufferConfig* config)
{
  cppConfig.channels = (ib::sim::fr::FlexrayChannel)config->channels;
  cppConfig.slotId = config->slotId;
  cppConfig.offset = config->offset;
  cppConfig.repetition = config->repetition;
  cppConfig.hasPayloadPreambleIndicator = config->hasPayloadPreambleIndicator == ib_True;
  cppConfig.headerCrc = config->headerCrc;
  cppConfig.transmissionMode = (ib::sim::fr::FlexrayTransmissionMode)config->transmissionMode;
}

static void assign(ib::sim::fr::FlexrayClusterParameters& cppClusterParameters, const ib_Flexray_ClusterParameters* clusterParameters)
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

static void assign(ib::sim::fr::FlexrayNodeParameters& cppNodeParameters, const ib_Flexray_NodeParameters* nodeParameters)
{
  cppNodeParameters.pAllowHaltDueToClock = nodeParameters->pAllowHaltDueToClock;
  cppNodeParameters.pAllowPassiveToActive = nodeParameters->pAllowPassiveToActive;
  cppNodeParameters.pChannels = (ib::sim::fr::FlexrayChannel)nodeParameters->pChannels;
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
  cppNodeParameters.pWakeupChannel = (ib::sim::fr::FlexrayChannel)nodeParameters->pWakeupChannel;
  cppNodeParameters.pWakeupPattern = nodeParameters->pWakeupPattern;
  cppNodeParameters.pdMicrotick = (ib::sim::fr::FlexrayClockPeriod)nodeParameters->pdMicrotick;
  cppNodeParameters.pSamplesPerMicrotick = nodeParameters->pSamplesPerMicrotick;
}

static void assign(ib::sim::fr::FlexrayControllerConfig& cppConfig, const ib_Flexray_ControllerConfig* config)
{
  assign(cppConfig.clusterParams, &config->clusterParams);
  assign(cppConfig.nodeParams, &config->nodeParams);

  for (uint32_t i = 0;i < config->numBufferConfigs; i++)
  {
    ib::sim::fr::FlexrayTxBufferConfig txBufferConfig;
    assign(txBufferConfig, &config->bufferConfigs[i]);
    cppConfig.bufferConfigs.push_back(std::move(txBufferConfig));
  }
}

extern "C" {

ib_ReturnCode ib_Flexray_Controller_Create(ib_Flexray_Controller** outController, ib_Participant* participant, const char* name, const char* network)
{
  ASSERT_VALID_OUT_PARAMETER(outController);
  ASSERT_VALID_POINTER_PARAMETER(participant);
  ASSERT_VALID_POINTER_PARAMETER(name);
  ASSERT_VALID_POINTER_PARAMETER(network);
  CAPI_ENTER
  {
    auto cppParticipant = reinterpret_cast<ib::mw::IParticipant*>(participant);
    std::string cppName(name);
    std::string cppNetwork(network);
    auto controller = cppParticipant->CreateFlexrayController(cppName, cppNetwork);
    if (controller == nullptr)
    {
      return ib_ReturnCode_UNSPECIFIEDERROR;
    }
    *outController = reinterpret_cast<ib_Flexray_Controller*>(controller);
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Flexray_Controller_Configure(ib_Flexray_Controller* controller, const ib_Flexray_ControllerConfig* config)
{
  ASSERT_VALID_POINTER_PARAMETER(controller);
  ASSERT_VALID_POINTER_PARAMETER(config);
  CAPI_ENTER
  {
    ib::sim::fr::IFlexrayController* cppController = reinterpret_cast<ib::sim::fr::IFlexrayController*>(controller);
    ib::sim::fr::FlexrayControllerConfig cppConfig;
    assign(cppConfig, config);

    cppController->Configure(cppConfig);
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Flexray_Controller_ReconfigureTxBuffer(ib_Flexray_Controller* controller, uint16_t txBufferIdx, const ib_Flexray_TxBufferConfig* config)
{
  ASSERT_VALID_POINTER_PARAMETER(controller);
  ASSERT_VALID_POINTER_PARAMETER(config);
  CAPI_ENTER
  {
    ib::sim::fr::IFlexrayController* cppController = reinterpret_cast<ib::sim::fr::IFlexrayController*>(controller);
    ib::sim::fr::FlexrayTxBufferConfig cppConfig;
    assign(cppConfig, config);

    cppController->ReconfigureTxBuffer(txBufferIdx, cppConfig);
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Flexray_Controller_UpdateTxBuffer(ib_Flexray_Controller* controller, const ib_Flexray_TxBufferUpdate* update)
{
  ASSERT_VALID_POINTER_PARAMETER(controller);
  ASSERT_VALID_POINTER_PARAMETER(update);
  CAPI_ENTER
  {
    ib::sim::fr::IFlexrayController* cppController = reinterpret_cast<ib::sim::fr::IFlexrayController*>(controller);
    ib::sim::fr::FlexrayTxBufferUpdate cppUpdate;
    cppUpdate.txBufferIndex = update->txBufferIndex;
    cppUpdate.payloadDataValid = update->payloadDataValid == ib_True;
    if (update->payloadDataValid)
    {
      ASSERT_VALID_POINTER_PARAMETER(update->payload.data);
      cppUpdate.payload.assign(update->payload.data, update->payload.data+update->payload.size);
    }
    cppController->UpdateTxBuffer(cppUpdate);
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}


ib_ReturnCode ib_Flexray_Controller_ExecuteCmd(ib_Flexray_Controller* controller, ib_Flexray_ChiCommand cmd)
{
  ASSERT_VALID_POINTER_PARAMETER(controller);
  CAPI_ENTER
  {
    ib::sim::fr::IFlexrayController* cppController = reinterpret_cast<ib::sim::fr::IFlexrayController*>(controller);
    switch (cmd)
    {
    case ib_Flexray_ChiCommand_RUN:
      cppController->Run();
      break;
    case ib_Flexray_ChiCommand_DEFERRED_HALT:
      cppController->DeferredHalt();
      break;
    case ib_Flexray_ChiCommand_FREEZE:
      cppController->Freeze();
      break;
    case ib_Flexray_ChiCommand_ALLOW_COLDSTART:
      cppController->AllowColdstart();
      break;
    case ib_Flexray_ChiCommand_ALL_SLOTS:
      cppController->AllSlots();
      break;
    case ib_Flexray_ChiCommand_WAKEUP:
      cppController->Wakeup();
      break;
    default:
      return ib_ReturnCode_BADPARAMETER;
    }
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Flexray_Controller_AddFrameHandler(ib_Flexray_Controller* controller, void* context, ib_Flexray_FrameHandler_t handler)
{
  ASSERT_VALID_POINTER_PARAMETER(controller);
  ASSERT_VALID_HANDLER_PARAMETER(handler);
  CAPI_ENTER
  {
    ib::sim::fr::IFlexrayController* cppController = reinterpret_cast<ib::sim::fr::IFlexrayController*>(controller);
    cppController->AddFrameHandler(
        [context, handler](ib::sim::fr::IFlexrayController* ctrl, const ib::sim::fr::FlexrayFrameEvent& msg) {
            ib_Flexray_FrameEvent message;
            ib_Flexray_Frame frame;
            message.interfaceId = ib_InterfaceIdentifier_FlexrayFrameEvent;
            message.timestamp = msg.timestamp.count();
            message.channel = (ib_Flexray_Channel)msg.channel;
            message.frame = &frame;
            frame.header.cycleCount = msg.frame.header.cycleCount;
            frame.header.frameId = msg.frame.header.frameId;
            frame.header.flags = msg.frame.header.flags;
            frame.header.headerCrc = msg.frame.header.headerCrc;
            frame.header.payloadLength = msg.frame.header.payloadLength;
            frame.payload.data = (uint8_t*)msg.frame.payload.data();
            frame.payload.size = (uint32_t)msg.frame.payload.size();
            handler(context, ctrl, &message);
        });
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}


ib_ReturnCode ib_Flexray_Controller_AddFrameTransmitHandler(ib_Flexray_Controller* controller, void* context, ib_Flexray_FrameTransmitHandler_t handler)
{
  ASSERT_VALID_POINTER_PARAMETER(controller);
  ASSERT_VALID_HANDLER_PARAMETER(handler);
  CAPI_ENTER
  {
    ib::sim::fr::IFlexrayController* cppController = reinterpret_cast<ib::sim::fr::IFlexrayController*>(controller);
    cppController->AddFrameTransmitHandler(
        [context, handler](ib::sim::fr::IFlexrayController* ctrl, const ib::sim::fr::FlexrayFrameTransmitEvent& msg) {
            ib_Flexray_FrameTransmitEvent message;
            ib_Flexray_Frame frame;
            message.interfaceId = ib_InterfaceIdentifier_FlexrayFrameTransmitEvent;
            message.timestamp = msg.timestamp.count();
            message.txBufferIndex = msg.txBufferIndex;
            message.channel = (ib_Flexray_Channel)msg.channel;
            message.frame = &frame;
            frame.header.cycleCount = msg.frame.header.cycleCount;
            frame.header.frameId = msg.frame.header.frameId;
            frame.header.flags = msg.frame.header.flags;
            frame.header.headerCrc = msg.frame.header.headerCrc;
            frame.header.payloadLength = msg.frame.header.payloadLength;
            frame.payload.data = (uint8_t*)msg.frame.payload.data();
            frame.payload.size = (uint32_t)msg.frame.payload.size();
            handler(context, ctrl, &message);
        });
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}


ib_ReturnCode ib_Flexray_Controller_AddWakeupHandler(ib_Flexray_Controller* controller, void* context, ib_Flexray_WakeupHandler_t handler)
{
  ASSERT_VALID_POINTER_PARAMETER(controller);
  ASSERT_VALID_HANDLER_PARAMETER(handler);
  CAPI_ENTER
  {
    ib::sim::fr::IFlexrayController* cppController = reinterpret_cast<ib::sim::fr::IFlexrayController*>(controller);
    cppController->AddWakeupHandler(
        [context, handler](ib::sim::fr::IFlexrayController* ctrl, const ib::sim::fr::FlexrayWakeupEvent& msg) {
            ib_Flexray_SymbolEvent message;
            message.interfaceId = ib_InterfaceIdentifier_FlexrayWakeupEvent;
            message.timestamp = msg.timestamp.count();
            message.channel = (ib_Flexray_Channel)msg.channel;
            message.pattern = (ib_Flexray_SymbolPattern)msg.pattern;
            handler(context, ctrl, &message);
        });
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Flexray_Controller_AddPocStatusHandler(ib_Flexray_Controller* controller, void* context, ib_Flexray_PocStatusHandler_t handler)
{
  ASSERT_VALID_POINTER_PARAMETER(controller);
  ASSERT_VALID_HANDLER_PARAMETER(handler);
  CAPI_ENTER
  {
    ib::sim::fr::IFlexrayController* cppController = reinterpret_cast<ib::sim::fr::IFlexrayController*>(controller);
    cppController->AddPocStatusHandler(
        [context, handler](ib::sim::fr::IFlexrayController* ctrl, const ib::sim::fr::FlexrayPocStatusEvent& msg) {
            ib_Flexray_PocStatusEvent message;
            message.interfaceId = ib_InterfaceIdentifier_FlexrayPocStatusEvent;
            message.timestamp = msg.timestamp.count();
            message.state = (ib_Flexray_PocState)msg.state;
            message.chiHaltRequest = msg.chiHaltRequest;
            message.coldstartNoise = msg.coldstartNoise;
            message.freeze = msg.freeze;
            message.chiReadyRequest = msg.chiReadyRequest;
            message.errorMode = (ib_Flexray_ErrorModeType)msg.errorMode;
            message.slotMode = (ib_Flexray_SlotModeType)msg.slotMode;
            message.startupState = (ib_Flexray_StartupStateType)msg.startupState;
            message.wakeupStatus = (ib_Flexray_WakeupStatusType)msg.wakeupStatus;
            handler(context, ctrl, &message);
        });
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Flexray_Controller_AddSymbolHandler(ib_Flexray_Controller* controller, void* context, ib_Flexray_SymbolHandler_t handler)
{
  ASSERT_VALID_POINTER_PARAMETER(controller);
  ASSERT_VALID_HANDLER_PARAMETER(handler);
  CAPI_ENTER
  {
    ib::sim::fr::IFlexrayController* cppController = reinterpret_cast<ib::sim::fr::IFlexrayController*>(controller);
    cppController->AddSymbolHandler(
        [context, handler](ib::sim::fr::IFlexrayController* ctrl, const ib::sim::fr::FlexraySymbolEvent& msg) {
            ib_Flexray_SymbolEvent message;
            message.interfaceId = ib_InterfaceIdentifier_FlexraySymbolEvent;
            message.timestamp = msg.timestamp.count();
            message.channel = (ib_Flexray_Channel)msg.channel;
            message.pattern = (ib_Flexray_SymbolPattern)msg.pattern;
            handler(context, ctrl, &message);
        });
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Flexray_Controller_AddSymbolTransmitHandler(ib_Flexray_Controller* controller, void* context, ib_Flexray_SymbolTransmitHandler_t handler)
{
  ASSERT_VALID_POINTER_PARAMETER(controller);
  ASSERT_VALID_HANDLER_PARAMETER(handler);
  CAPI_ENTER
  {
    ib::sim::fr::IFlexrayController* cppController = reinterpret_cast<ib::sim::fr::IFlexrayController*>(controller);
    cppController->AddSymbolTransmitHandler(
        [context, handler](ib::sim::fr::IFlexrayController* ctrl, const ib::sim::fr::FlexraySymbolTransmitEvent& msg) {
            ib_Flexray_SymbolTransmitEvent message;
            message.interfaceId = ib_InterfaceIdentifier_FlexraySymbolTransmitEvent;
            message.timestamp = msg.timestamp.count();
            message.channel = (ib_Flexray_Channel)msg.channel;
            message.pattern = (ib_Flexray_SymbolPattern)msg.pattern;
            handler(context, ctrl, &message);
        });
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Flexray_Controller_AddCycleStartHandler(ib_Flexray_Controller* controller, void* context, ib_Flexray_CycleStartHandler_t handler)
{
  ASSERT_VALID_POINTER_PARAMETER(controller);
  ASSERT_VALID_HANDLER_PARAMETER(handler);
  CAPI_ENTER
  {
    ib::sim::fr::IFlexrayController* cppController = reinterpret_cast<ib::sim::fr::IFlexrayController*>(controller);
    cppController->AddCycleStartHandler(
        [context, handler](ib::sim::fr::IFlexrayController* ctrl, const ib::sim::fr::FlexrayCycleStartEvent& msg) {
            ib_Flexray_CycleStartEvent message;
            message.interfaceId = ib_InterfaceIdentifier_FlexrayCycleStartEvent;
            message.timestamp = msg.timestamp.count();
            message.cycleCounter = msg.cycleCounter;
            handler(context, ctrl, &message);
        });
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

}
