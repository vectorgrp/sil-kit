/* Copyright (c) Vector Informatik GmbH. All rights reserved. */

#include "ib/capi/IntegrationBus.h"
#include "ib/IntegrationBus.hpp"
#include "IComAdapterInternal.hpp"
#include <string>
#include <string.h>
#include "CapiImpl.h"
#include "ib/sim/fr/all.hpp"

static void assign(ib::sim::fr::TxBufferConfig& cppConfig, const ib_FlexRay_TxBufferConfig* config)
{
  cppConfig.channels = (ib::sim::fr::Channel)config->channels;
  cppConfig.slotId = config->slotId;
  cppConfig.offset = config->offset;
  cppConfig.repetition = config->repetition;
  cppConfig.hasPayloadPreambleIndicator = config->hasPayloadPreambleIndicator;
  cppConfig.headerCrc = config->headerCrc;
  cppConfig.transmissionMode = (ib::sim::fr::TransmissionMode)config->transmissionMode;
}

static void assign(ib_FlexRay_TxBufferConfig* config, const ib::sim::fr::TxBufferConfig& cppConfig)
{
  config->channels = (ib_FlexRay_Channel)cppConfig.channels;
  config->slotId = cppConfig.slotId;
  config->offset = cppConfig.offset;
  config->repetition = cppConfig.repetition;
  config->hasPayloadPreambleIndicator = cppConfig.hasPayloadPreambleIndicator;
  config->headerCrc = cppConfig.headerCrc;
  config->transmissionMode = (ib_FlexRay_TransmissionMode)cppConfig.transmissionMode;
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

static void assign(ib_FlexRay_ClusterParameters* clusterParameters, const ib::sim::fr::ClusterParameters& cppClusterParameters)
{
  clusterParameters->gColdstartAttempts = cppClusterParameters.gColdstartAttempts;
  clusterParameters->gCycleCountMax = cppClusterParameters.gCycleCountMax;
  clusterParameters->gdActionPointOffset = cppClusterParameters.gdActionPointOffset;
  clusterParameters->gdDynamicSlotIdlePhase = cppClusterParameters.gdDynamicSlotIdlePhase;
  clusterParameters->gdMiniSlot = cppClusterParameters.gdMiniSlot;
  clusterParameters->gdMiniSlotActionPointOffset = cppClusterParameters.gdMiniSlotActionPointOffset;
  clusterParameters->gdStaticSlot = cppClusterParameters.gdStaticSlot;
  clusterParameters->gdSymbolWindow = cppClusterParameters.gdSymbolWindow;
  clusterParameters->gdSymbolWindowActionPointOffset = cppClusterParameters.gdSymbolWindowActionPointOffset;
  clusterParameters->gdTSSTransmitter = cppClusterParameters.gdTSSTransmitter;
  clusterParameters->gdWakeupTxActive = cppClusterParameters.gdWakeupTxActive;
  clusterParameters->gdWakeupTxIdle = cppClusterParameters.gdWakeupTxIdle;
  clusterParameters->gListenNoise = cppClusterParameters.gListenNoise;
  clusterParameters->gMacroPerCycle = cppClusterParameters.gMacroPerCycle;
  clusterParameters->gMaxWithoutClockCorrectionFatal = cppClusterParameters.gMaxWithoutClockCorrectionFatal;
  clusterParameters->gMaxWithoutClockCorrectionPassive = cppClusterParameters.gMaxWithoutClockCorrectionPassive;
  clusterParameters->gNumberOfMiniSlots = cppClusterParameters.gNumberOfMiniSlots;
  clusterParameters->gNumberOfStaticSlots = cppClusterParameters.gNumberOfStaticSlots;
  clusterParameters->gPayloadLengthStatic = cppClusterParameters.gPayloadLengthStatic;
  clusterParameters->gSyncFrameIDCountMax = cppClusterParameters.gSyncFrameIDCountMax;
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

static void assign(ib_FlexRay_NodeParameters* nodeParameters, const ib::sim::fr::NodeParameters& cppNodeParameters)
{
  nodeParameters->pAllowHaltDueToClock = cppNodeParameters.pAllowHaltDueToClock;
  nodeParameters->pAllowPassiveToActive = cppNodeParameters.pAllowPassiveToActive;
  nodeParameters->pChannels = (ib_FlexRay_Channel)cppNodeParameters.pChannels;
  nodeParameters->pClusterDriftDamping = cppNodeParameters.pClusterDriftDamping;
  nodeParameters->pdAcceptedStartupRange = cppNodeParameters.pdAcceptedStartupRange;
  nodeParameters->pdListenTimeout = cppNodeParameters.pdListenTimeout;
  nodeParameters->pKeySlotId = cppNodeParameters.pKeySlotId;
  nodeParameters->pKeySlotOnlyEnabled = cppNodeParameters.pKeySlotOnlyEnabled;
  nodeParameters->pKeySlotUsedForStartup = cppNodeParameters.pKeySlotUsedForStartup;
  nodeParameters->pKeySlotUsedForSync = cppNodeParameters.pKeySlotUsedForSync;
  nodeParameters->pLatestTx = cppNodeParameters.pLatestTx;
  nodeParameters->pMacroInitialOffsetA = cppNodeParameters.pMacroInitialOffsetA;
  nodeParameters->pMacroInitialOffsetB = cppNodeParameters.pMacroInitialOffsetB;
  nodeParameters->pMicroInitialOffsetA = cppNodeParameters.pMicroInitialOffsetA;
  nodeParameters->pMicroInitialOffsetB = cppNodeParameters.pMicroInitialOffsetB;
  nodeParameters->pMicroPerCycle = cppNodeParameters.pMicroPerCycle;
  nodeParameters->pOffsetCorrectionOut = cppNodeParameters.pOffsetCorrectionOut;
  nodeParameters->pOffsetCorrectionStart = cppNodeParameters.pOffsetCorrectionStart;
  nodeParameters->pRateCorrectionOut = cppNodeParameters.pRateCorrectionOut;
  nodeParameters->pWakeupChannel = (ib_FlexRay_Channel)cppNodeParameters.pWakeupChannel;
  nodeParameters->pWakeupPattern = cppNodeParameters.pWakeupPattern;
  nodeParameters->pdMicrotick = (ib_FlexRay_ClockPeriod)cppNodeParameters.pdMicrotick;
  nodeParameters->pSamplesPerMicrotick = cppNodeParameters.pSamplesPerMicrotick;
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

static void assign(ib_FlexRay_ControllerConfig** config, const ib::sim::fr::ControllerConfig& cppConfig)
{
  assign(&(*config)->clusterParams, cppConfig.clusterParams);
  assign(&(*config)->nodeParams, cppConfig.nodeParams);

  for (int i = 0;i < cppConfig.bufferConfigs.size(); i++)
  {
    ib_FlexRay_TxBufferConfig txBufferConfig;
    assign(&txBufferConfig, cppConfig.bufferConfigs[i]);
    ib_FlexRay_Append_TxBufferConfig(config, &txBufferConfig);
  }
}

static void assign(ib_FlexRay_ControllerConfig** config, const ib::cfg::FlexrayController& cppConfig)
{
  assign(&(*config)->clusterParams, cppConfig.clusterParameters);
  assign(&(*config)->nodeParams, cppConfig.nodeParameters);
  (*config)->numBufferConfigs = 0;
  for (int i = 0;i < cppConfig.txBufferConfigs.size(); i++)
  {
    ib_FlexRay_TxBufferConfig txBufferConfig;
    assign(&txBufferConfig, cppConfig.txBufferConfigs[i]);
    ib_FlexRay_Append_TxBufferConfig(config, &txBufferConfig);
  }
}

extern "C" {

IntegrationBusAPI ib_ReturnCode ib_FlexRay_Controller_Create(ib_FlexRay_Controller** outController, ib_SimulationParticipant* participant, const char* cName, const char* cNetwork)
{
  ASSERT_VALID_OUT_PARAMETER(outController);
  ASSERT_VALID_POINTER_PARAMETER(participant);
  ASSERT_VALID_POINTER_PARAMETER(cName);
  ASSERT_VALID_POINTER_PARAMETER(cNetwork);
  CAPI_ENTER
  {
    ib::mw::IComAdapter* comAdapter = reinterpret_cast<ib::mw::IComAdapter*>(participant);
    std::string name(cName);
    std::string network(cNetwork);
    auto controller = comAdapter->CreateFlexrayController(name, network);
    if (controller == nullptr)
    {
      return ib_ReturnCode_UNSPECIFIEDERROR;
    }
    *outController = reinterpret_cast<ib_FlexRay_Controller*>(controller);
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_FlexRay_ControllerConfig_Create(ib_FlexRay_ControllerConfig** outControllerConfig, ib_SimulationParticipant* participant, const char* cName)
{
  ASSERT_VALID_OUT_PARAMETER(outControllerConfig);
  ASSERT_VALID_POINTER_PARAMETER(participant);
  ASSERT_VALID_POINTER_PARAMETER(cName);
  CAPI_ENTER
  {
    auto* result = (ib_FlexRay_ControllerConfig*)malloc(sizeof(ib_FlexRay_ControllerConfig));
    memset(result, 0, sizeof(ib_FlexRay_ControllerConfig));
    ib::mw::IComAdapter* comAdapter = reinterpret_cast<ib::mw::IComAdapter*>(participant);
    ib::mw::IComAdapterInternal* comAdapterInternal = static_cast<ib::mw::IComAdapterInternal*>(comAdapter);
    std::string participantName = comAdapterInternal->GetParticipantName();
    auto& ibConfig = comAdapterInternal->GetConfig();
    auto& participantConfig = get_by_name(ibConfig.simulationSetup.participants, participantName);
    auto& flexrayControllerCfg = get_by_name(participantConfig.flexrayControllers, cName);
    assign(&result, flexrayControllerCfg);
    *outControllerConfig = result;
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_FlexRay_Append_TxBufferConfig(ib_FlexRay_ControllerConfig** inOutControllerConfig, const ib_FlexRay_TxBufferConfig* txBufferConfig)
{
  ASSERT_VALID_POINTER_TO_POINTER_PARAMETER(inOutControllerConfig);
  ASSERT_VALID_POINTER_PARAMETER(txBufferConfig);
  CAPI_ENTER
  {
    uint32_t numberOfTxBufferConfigs = (*inOutControllerConfig)->numBufferConfigs + 1;
    // NOTE: ib_FlexRay_ControllerConfig already contains one ib_FlexRay_TxBufferConfig,
    // so add numberOfTxBufferConfigs-1 times sizeof(ib_FlexRay_TxBufferConfig)
    size_t newSize = sizeof(ib_FlexRay_ControllerConfig) + ((numberOfTxBufferConfigs-1) * sizeof(ib_FlexRay_TxBufferConfig));
    ib_FlexRay_ControllerConfig* result = (ib_FlexRay_ControllerConfig*)realloc(*inOutControllerConfig, newSize);
    if (result == nullptr)
    {
      ib_error_string = std::string("could not realloc controller config to ") + std::to_string(newSize) + " bytes.";
      return ib_ReturnCode_UNSPECIFIEDERROR;
    }
    memcpy(&result->bufferConfigs[numberOfTxBufferConfigs-1], txBufferConfig, sizeof(ib_FlexRay_TxBufferConfig));
    result->numBufferConfigs = numberOfTxBufferConfigs;
    *inOutControllerConfig = result;
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

IntegrationBusAPI ib_ReturnCode ib_FlexRay_Controller_ReconfigureTxBuffer(ib_FlexRay_Controller* self, uint16_t txBufferIdx, const ib_FlexRay_TxBufferConfig* config)
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
    cppUpdate.payloadDataValid = update->payloadDataValid;
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


IntegrationBusAPI ib_ReturnCode ib_FlexRay_Controller_ExecuteCmd(ib_FlexRay_Controller* self, ib_FlexRay_ChiCommand cmd)
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
