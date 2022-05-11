/* Copyright (c) Vector Informatik GmbH. All rights reserved. */

#include <string>
#include <string.h>

#include "ib/capi/IntegrationBus.h"
#include "ib/IntegrationBus.hpp"
#include "ib/sim/fr/all.hpp"

#include "IParticipantInternal.hpp"
#include "CapiImpl.h"
#include "ParticipantConfiguration.hpp"

static void assign(ib::sim::fr::TxBufferConfig& cppConfig, const ib_FlexRay_TxBufferConfig* config)
{
  cppConfig.channels = (ib::sim::fr::Channel)config->channels;
  cppConfig.slotId = config->slotId;
  cppConfig.offset = config->offset;
  cppConfig.repetition = config->repetition;
  cppConfig.hasPayloadPreambleIndicator = config->hasPayloadPreambleIndicator == ib_True;
  cppConfig.headerCrc = config->headerCrc;
  cppConfig.transmissionMode = (ib::sim::fr::TransmissionMode)config->transmissionMode;
}

static void assign(ib::sim::fr::ClusterParameters& cppClusterParameters, const ib_FlexRay_ClusterParameters* clusterParameters)
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

static void assign(ib::sim::fr::NodeParameters& cppNodeParameters, const ib_FlexRay_NodeParameters* nodeParameters)
{
  cppNodeParameters.pAllowHaltDueToClock = nodeParameters->pAllowHaltDueToClock;
  cppNodeParameters.pAllowPassiveToActive = nodeParameters->pAllowPassiveToActive;
  cppNodeParameters.pChannels = (ib::sim::fr::Channel)nodeParameters->pChannels;
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
  cppNodeParameters.pWakeupChannel = (ib::sim::fr::Channel)nodeParameters->pWakeupChannel;
  cppNodeParameters.pWakeupPattern = nodeParameters->pWakeupPattern;
  cppNodeParameters.pdMicrotick = (ib::sim::fr::ClockPeriod)nodeParameters->pdMicrotick;
  cppNodeParameters.pSamplesPerMicrotick = nodeParameters->pSamplesPerMicrotick;
}

static void assign(ib::sim::fr::ControllerConfig& cppConfig, const ib_FlexRay_ControllerConfig* config)
{
  assign(cppConfig.clusterParams, &config->clusterParams);
  assign(cppConfig.nodeParams, &config->nodeParams);

  for (uint32_t i = 0;i < config->numBufferConfigs; i++)
  {
    ib::sim::fr::TxBufferConfig txBufferConfig;
    assign(txBufferConfig, &config->bufferConfigs[i]);
    cppConfig.bufferConfigs.push_back(std::move(txBufferConfig));
  }
}

extern "C" {

ib_ReturnCode ib_FlexRay_Controller_Create(ib_FlexRay_Controller** outController, ib_Participant* participant, const char* cName, const char* cNetwork)
{
  ASSERT_VALID_OUT_PARAMETER(outController);
  ASSERT_VALID_POINTER_PARAMETER(participant);
  ASSERT_VALID_POINTER_PARAMETER(cName);
  ASSERT_VALID_POINTER_PARAMETER(cNetwork);
  CAPI_ENTER
  {
    auto cppParticipant = reinterpret_cast<ib::mw::IParticipant*>(participant);
    std::string name(cName);
    std::string network(cNetwork);
    auto controller = cppParticipant->CreateFlexrayController(name, network);
    if (controller == nullptr)
    {
      return ib_ReturnCode_UNSPECIFIEDERROR;
    }
    *outController = reinterpret_cast<ib_FlexRay_Controller*>(controller);
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_FlexRay_Controller_Configure(ib_FlexRay_Controller* controller, const ib_FlexRay_ControllerConfig* config)
{
  ASSERT_VALID_POINTER_PARAMETER(controller);
  ASSERT_VALID_POINTER_PARAMETER(config);
  CAPI_ENTER
  {
    ib::sim::fr::IFrController* cppController = reinterpret_cast<ib::sim::fr::IFrController*>(controller);
    ib::sim::fr::ControllerConfig cppConfig;
    assign(cppConfig, config);

    cppController->Configure(cppConfig);
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_FlexRay_Controller_ReconfigureTxBuffer(ib_FlexRay_Controller* self, uint16_t txBufferIdx, const ib_FlexRay_TxBufferConfig* config)
{
  ASSERT_VALID_POINTER_PARAMETER(self);
  ASSERT_VALID_POINTER_PARAMETER(config);
  CAPI_ENTER
  {
    ib::sim::fr::IFrController* cppController = reinterpret_cast<ib::sim::fr::IFrController*>(self);
    ib::sim::fr::TxBufferConfig cppConfig;
    assign(cppConfig, config);

    cppController->ReconfigureTxBuffer(txBufferIdx, cppConfig);
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_FlexRay_Controller_UpdateTxBuffer(ib_FlexRay_Controller* self, const ib_FlexRay_TxBufferUpdate* update)
{
  ASSERT_VALID_POINTER_PARAMETER(self);
  ASSERT_VALID_POINTER_PARAMETER(update);
  CAPI_ENTER
  {
    ib::sim::fr::IFrController* cppController = reinterpret_cast<ib::sim::fr::IFrController*>(self);
    ib::sim::fr::TxBufferUpdate cppUpdate;
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


ib_ReturnCode ib_FlexRay_Controller_ExecuteCmd(ib_FlexRay_Controller* self, ib_FlexRay_ChiCommand cmd)
{
  ASSERT_VALID_POINTER_PARAMETER(self);
  CAPI_ENTER
  {
    ib::sim::fr::IFrController* cppController = reinterpret_cast<ib::sim::fr::IFrController*>(self);
    switch (cmd)
    {
    case ib_FlexRay_ChiCommand_RUN:
      cppController->Run();
      break;
    case ib_FlexRay_ChiCommand_DEFERRED_HALT:
      cppController->DeferredHalt();
      break;
    case ib_FlexRay_ChiCommand_FREEZE:
      cppController->Freeze();
      break;
    case ib_FlexRay_ChiCommand_ALLOW_COLDSTART:
      cppController->AllowColdstart();
      break;
    case ib_FlexRay_ChiCommand_ALL_SLOTS:
      cppController->AllSlots();
      break;
    case ib_FlexRay_ChiCommand_WAKEUP:
      cppController->Wakeup();
      break;
    default:
      return ib_ReturnCode_BADPARAMETER;
    }
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_FlexRay_Controller_RegisterMessageHandler(ib_FlexRay_Controller* self, void* context, ib_FlexRay_MessageHandler_t handler)
{
  ASSERT_VALID_POINTER_PARAMETER(self);
  ASSERT_VALID_HANDLER_PARAMETER(handler);
  CAPI_ENTER
  {
    ib::sim::fr::IFrController* cppController = reinterpret_cast<ib::sim::fr::IFrController*>(self);
    cppController->RegisterMessageHandler(
      [context, handler](ib::sim::fr::IFrController* ctrl, const ib::sim::fr::FrMessage& msg)
      {
        ib_FlexRay_Message message;
        ib_FlexRay_Frame frame;
        message.interfaceId = ib_InterfaceIdentifier_FlexRayMessage;
        message.timestamp = msg.timestamp.count();
        message.channel = (ib_FlexRay_Channel)msg.channel;
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


ib_ReturnCode ib_FlexRay_Controller_RegisterMessageAckHandler(ib_FlexRay_Controller* controller, void* context, ib_FlexRay_MessageAckHandler_t handler)
{
  ASSERT_VALID_POINTER_PARAMETER(controller);
  ASSERT_VALID_HANDLER_PARAMETER(handler);
  CAPI_ENTER
  {
    ib::sim::fr::IFrController* cppController = reinterpret_cast<ib::sim::fr::IFrController*>(controller);
    cppController->RegisterMessageAckHandler(
      [context, handler](ib::sim::fr::IFrController* ctrl, const ib::sim::fr::FrMessageAck& msg)
      {
        ib_FlexRay_MessageAck message;
        ib_FlexRay_Frame frame;
        message.interfaceId = ib_InterfaceIdentifier_FlexRayMessageAck;
        message.timestamp = msg.timestamp.count();
        message.txBufferIndex = msg.txBufferIndex;
        message.channel = (ib_FlexRay_Channel)msg.channel;
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


ib_ReturnCode ib_FlexRay_Controller_RegisterWakeupHandler(ib_FlexRay_Controller* controller, void* context, ib_FlexRay_WakeupHandler_t handler)
{
  ASSERT_VALID_POINTER_PARAMETER(controller);
  ASSERT_VALID_HANDLER_PARAMETER(handler);
  CAPI_ENTER
  {
    ib::sim::fr::IFrController* cppController = reinterpret_cast<ib::sim::fr::IFrController*>(controller);
    cppController->RegisterWakeupHandler(
      [context, handler](ib::sim::fr::IFrController* ctrl, const ib::sim::fr::FrSymbol& msg)
      {
        ib_FlexRay_Symbol message;
        message.interfaceId = ib_InterfaceIdentifier_FlexRaySymbol;
        message.timestamp = msg.timestamp.count();
        message.channel = (ib_FlexRay_Channel)msg.channel;
        message.pattern = (ib_FlexRay_SymbolPattern)msg.pattern;
        handler(context, ctrl, &message);
      });
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_FlexRay_Controller_RegisterPocStatusHandler(ib_FlexRay_Controller* controller, void* context, ib_FlexRay_PocStatusHandler_t handler)
{
  ASSERT_VALID_POINTER_PARAMETER(controller);
  ASSERT_VALID_HANDLER_PARAMETER(handler);
  CAPI_ENTER
  {
    ib::sim::fr::IFrController* cppController = reinterpret_cast<ib::sim::fr::IFrController*>(controller);
    cppController->RegisterPocStatusHandler(
      [context, handler](ib::sim::fr::IFrController* ctrl, const ib::sim::fr::PocStatus& msg)
      {
        ib_FlexRay_PocStatus message;
        message.interfaceId = ib_InterfaceIdentifier_FlexRayPocStatus;
        message.timestamp = msg.timestamp.count();
        message.state = (ib_FlexRay_PocState)msg.state;
        message.chiHaltRequest = msg.chiHaltRequest;
        message.coldstartNoise = msg.coldstartNoise;
        message.freeze = msg.freeze;
        message.chiReadyRequest = msg.chiReadyRequest;
        message.errorMode = (ib_FlexRay_ErrorModeType)msg.errorMode;
        message.slotMode = (ib_FlexRay_SlotModeType)msg.slotMode;
        message.startupState = (ib_FlexRay_StartupStateType) msg.startupState;
        message.wakeupStatus = (ib_FlexRay_WakeupStatusType)msg.wakeupStatus;
        handler(context, ctrl, &message);
      });
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_FlexRay_Controller_RegisterSymbolHandler(ib_FlexRay_Controller* controller, void* context, ib_FlexRay_SymbolHandler_t handler)
{
  ASSERT_VALID_POINTER_PARAMETER(controller);
  ASSERT_VALID_HANDLER_PARAMETER(handler);
  CAPI_ENTER
  {
    ib::sim::fr::IFrController* cppController = reinterpret_cast<ib::sim::fr::IFrController*>(controller);
    cppController->RegisterSymbolHandler(
      [context, handler](ib::sim::fr::IFrController* ctrl, const ib::sim::fr::FrSymbol& msg)
      {
        ib_FlexRay_Symbol message;
        message.interfaceId = ib_InterfaceIdentifier_FlexRaySymbol;
        message.timestamp = msg.timestamp.count();
        message.channel = (ib_FlexRay_Channel)msg.channel;
        message.pattern = (ib_FlexRay_SymbolPattern)msg.pattern;
        handler(context, ctrl, &message);
      });
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_FlexRay_Controller_RegisterSymbolAckHandler(ib_FlexRay_Controller* controller, void* context, ib_FlexRay_SymbolAckHandler_t handler)
{
  ASSERT_VALID_POINTER_PARAMETER(controller);
  ASSERT_VALID_HANDLER_PARAMETER(handler);
  CAPI_ENTER
  {
    ib::sim::fr::IFrController* cppController = reinterpret_cast<ib::sim::fr::IFrController*>(controller);
    cppController->RegisterSymbolAckHandler(
      [context, handler](ib::sim::fr::IFrController* ctrl, const ib::sim::fr::FrSymbolAck& msg)
      {
        ib_FlexRay_SymbolAck message;
        message.interfaceId = ib_InterfaceIdentifier_FlexRaySymbolAck;
        message.timestamp = msg.timestamp.count();
        message.channel = (ib_FlexRay_Channel)msg.channel;
        message.pattern = (ib_FlexRay_SymbolPattern)msg.pattern;
        handler(context, ctrl, &message);
      });
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_FlexRay_Controller_RegisterCycleStartHandler(ib_FlexRay_Controller* controller, void* context, ib_FlexRay_CycleStartHandler_t handler)
{
  ASSERT_VALID_POINTER_PARAMETER(controller);
  ASSERT_VALID_HANDLER_PARAMETER(handler);
  CAPI_ENTER
  {
    ib::sim::fr::IFrController* cppController = reinterpret_cast<ib::sim::fr::IFrController*>(controller);
    cppController->RegisterCycleStartHandler(
      [context, handler](ib::sim::fr::IFrController* ctrl, const ib::sim::fr::CycleStart& msg)
      {
        ib_FlexRay_CycleStart message;
        message.interfaceId = ib_InterfaceIdentifier_FlexRayCycleStart;
        message.timestamp = msg.timestamp.count();
        message.cycleCounter = msg.cycleCounter;
        handler(context, ctrl, &message);
      });
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

}
