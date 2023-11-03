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

#include "silkit/capi/Can.h"
#include "silkit/services/can/CanDatatypes.hpp"

#include "silkit/capi/Flexray.h"
#include "silkit/services/flexray/FlexrayDatatypes.hpp"

#include "silkit/capi/Ethernet.h"
#include "silkit/services/ethernet/EthernetDatatypes.hpp"

#include "silkit/capi/Lin.h"
#include "silkit/services/lin/LinDatatypes.hpp"

#include "silkit/capi/NetworkSimulator.h"
#include "silkit/experimental/netsim/NetworkSimulatorDatatypes.hpp"

#include "silkit/capi/InterfaceIdentifiers.h"

namespace {

// ================================================================
// Cxx to C
// ================================================================

// --------------------------------
// Cxx Services::Can to C
// --------------------------------

inline void assignCxxToC(const SilKit::Services::Can::CanFrame& cxxIn, SilKit_CanFrame& cOut)
{
    cOut.id = cxxIn.canId;
    cOut.flags = cxxIn.flags;
    cOut.dlc = cxxIn.dlc;
    cOut.sdt = cxxIn.sdt;
    cOut.vcid = cxxIn.vcid;
    cOut.af = cxxIn.af;
    cOut.data = ToSilKitByteVector(cxxIn.dataField);
}

inline void assignCxxToC(const SilKit::Services::Can::CanFrameEvent& cxxIn, SilKit_CanFrameEvent& cOut)
{
    assignCxxToC(cxxIn.frame, *cOut.frame);
    cOut.direction = static_cast<SilKit_Direction>(cxxIn.direction);
    cOut.timestamp = cxxIn.timestamp.count();
    cOut.userContext = cxxIn.userContext;
}

inline void assignCxxToC(const SilKit::Services::Can::CanFrameTransmitEvent& cxxIn, SilKit_CanFrameTransmitEvent& cOut)
{
    cOut.userContext = cxxIn.userContext;
    cOut.timestamp = cxxIn.timestamp.count();
    cOut.status = (SilKit_CanTransmitStatus)cxxIn.status;
    cOut.canId = cxxIn.canId;
}

inline void assignCxxToC(const SilKit::Services::Can::CanStateChangeEvent& cxxIn, SilKit_CanStateChangeEvent& cOut)
{
    cOut.timestamp = cxxIn.timestamp.count();
    cOut.state = (SilKit_CanControllerState)cxxIn.state;
}

inline void assignCxxToC(const SilKit::Services::Can::CanErrorStateChangeEvent& cxxIn, SilKit_CanErrorStateChangeEvent& cOut)
{
    cOut.timestamp = cxxIn.timestamp.count();
    cOut.errorState = (SilKit_CanErrorState)cxxIn.errorState;
}

// --------------------------------
// Cxx Experimental::NetworkSimulation::Can to C
// --------------------------------

inline void assignCxxToC(const SilKit::Experimental::NetworkSimulation::Can::CanControllerMode& cxxIn,
                         SilKit_Experimental_NetSim_CanControllerMode& cOut)
{
    cOut.state = (SilKit_CanControllerState)cxxIn.state;
    cOut.canControllerModeFlags = cxxIn.canControllerModeFlags;
}

inline void assignCxxToC(const SilKit::Experimental::NetworkSimulation::Can::CanConfigureBaudrate& cxxIn,
                  SilKit_Experimental_NetSim_CanConfigureBaudrate& cOut)
{
    cOut.rate = cxxIn.baudRate;
    cOut.fdRate = cxxIn.fdBaudRate;
    cOut.xlRate = cxxIn.xlBaudRate;
}

inline void assignCxxToC(const SilKit::Experimental::NetworkSimulation::Can::CanFrameRequest& cxxIn,
                         SilKit_Experimental_NetSim_CanFrameRequest& cOut)
{
    cOut.userContext = cxxIn.userContext;
    assignCxxToC(cxxIn.frame, *cOut.frame);
}

// --------------------------------
// Cxx Services::Flexray to C
// --------------------------------

inline void assignCxxToC(const SilKit::Services::Flexray::FlexrayHeader& cxxIn, SilKit_FlexrayHeader& cOut)
{
    cOut.cycleCount = cxxIn.cycleCount;
    cOut.frameId = cxxIn.frameId;
    cOut.flags = cxxIn.flags;
    cOut.headerCrc = cxxIn.headerCrc;
    cOut.payloadLength = cxxIn.payloadLength;
}

inline void assignCxxToC(const SilKit::Services::Flexray::FlexrayFrame& cxxIn, SilKit_FlexrayFrame& cOut)
{
    assignCxxToC(cxxIn.header, *cOut.header);
    cOut.payload.data = (uint8_t*)cxxIn.payload.data();
    cOut.payload.size = (uint32_t)cxxIn.payload.size();
}

inline void assignCxxToC(const SilKit::Services::Flexray::FlexrayFrameEvent& cxxIn, SilKit_FlexrayFrameEvent& cOut)
{
    assignCxxToC(cxxIn.frame, *cOut.frame);
    cOut.timestamp = cxxIn.timestamp.count();
    cOut.channel = (SilKit_FlexrayChannel)cxxIn.channel;
}

inline void assignCxxToC(const SilKit::Services::Flexray::FlexrayFrameTransmitEvent& cxxIn,
                   SilKit_FlexrayFrameTransmitEvent& cOut)
{
    assignCxxToC(cxxIn.frame, *cOut.frame);
    cOut.timestamp = cxxIn.timestamp.count();
    cOut.txBufferIndex = cxxIn.txBufferIndex;
    cOut.channel = (SilKit_FlexrayChannel)cxxIn.channel;
}

inline void assignCxxToC(const SilKit::Services::Flexray::FlexraySymbolEvent& cxxIn, SilKit_FlexraySymbolEvent& cOut)
{
    cOut.timestamp = cxxIn.timestamp.count();
    cOut.channel = (SilKit_FlexrayChannel)cxxIn.channel;
    cOut.pattern = (SilKit_FlexraySymbolPattern)cxxIn.pattern;
}

inline void assignCxxToC(const SilKit::Services::Flexray::FlexraySymbolTransmitEvent& cxxIn,
                   SilKit_FlexraySymbolTransmitEvent& cOut)
{
    cOut.timestamp = cxxIn.timestamp.count();
    cOut.channel = (SilKit_FlexrayChannel)cxxIn.channel;
    cOut.pattern = (SilKit_FlexraySymbolPattern)cxxIn.pattern;
}

inline void assignCxxToC(const SilKit::Services::Flexray::FlexrayCycleStartEvent& cxxIn, SilKit_FlexrayCycleStartEvent& cOut)
{
    cOut.timestamp = cxxIn.timestamp.count();
    cOut.cycleCounter = cxxIn.cycleCounter;
}

inline void assignCxxToC(const SilKit::Services::Flexray::FlexrayPocStatusEvent& cxxIn, SilKit_FlexrayPocStatusEvent& cOut)
{
    cOut.timestamp = cxxIn.timestamp.count();
    cOut.state = (SilKit_FlexrayPocState)cxxIn.state;
    cOut.chiHaltRequest = cxxIn.chiHaltRequest;
    cOut.coldstartNoise = cxxIn.coldstartNoise;
    cOut.freeze = cxxIn.freeze;
    cOut.chiReadyRequest = cxxIn.chiReadyRequest;
    cOut.errorMode = (SilKit_FlexrayErrorModeType)cxxIn.errorMode;
    cOut.slotMode = (SilKit_FlexraySlotModeType)cxxIn.slotMode;
    cOut.startupState = (SilKit_FlexrayStartupStateType)cxxIn.startupState;
    cOut.wakeupStatus = (SilKit_FlexrayWakeupStatusType)cxxIn.wakeupStatus;
}

inline void assignCxxToC(const SilKit::Services::Flexray::FlexrayClusterParameters& cxxIn,
                  SilKit_FlexrayClusterParameters& cOut)
{
    cOut.gColdstartAttempts = cxxIn.gColdstartAttempts;
    cOut.gCycleCountMax = cxxIn.gCycleCountMax;
    cOut.gdActionPointOffset = cxxIn.gdActionPointOffset;
    cOut.gdDynamicSlotIdlePhase = cxxIn.gdDynamicSlotIdlePhase;
    cOut.gdMiniSlot = cxxIn.gdMiniSlot;
    cOut.gdMiniSlotActionPointOffset = cxxIn.gdMiniSlotActionPointOffset;
    cOut.gdStaticSlot = cxxIn.gdStaticSlot;
    cOut.gdSymbolWindow = cxxIn.gdSymbolWindow;
    cOut.gdSymbolWindowActionPointOffset = cxxIn.gdSymbolWindowActionPointOffset;
    cOut.gdTSSTransmitter = cxxIn.gdTSSTransmitter;
    cOut.gdWakeupTxActive = cxxIn.gdWakeupTxActive;
    cOut.gdWakeupTxIdle = cxxIn.gdWakeupTxIdle;
    cOut.gListenNoise = cxxIn.gListenNoise;
    cOut.gMacroPerCycle = cxxIn.gMacroPerCycle;
    cOut.gMaxWithoutClockCorrectionFatal = cxxIn.gMaxWithoutClockCorrectionFatal;
    cOut.gMaxWithoutClockCorrectionPassive = cxxIn.gMaxWithoutClockCorrectionPassive;
    cOut.gNumberOfMiniSlots = cxxIn.gNumberOfMiniSlots;
    cOut.gNumberOfStaticSlots = cxxIn.gNumberOfStaticSlots;
    cOut.gPayloadLengthStatic = cxxIn.gPayloadLengthStatic;
    cOut.gSyncFrameIDCountMax = cxxIn.gSyncFrameIDCountMax;
}

inline void assignCxxToC(const SilKit::Services::Flexray::FlexrayNodeParameters& cxxIn, SilKit_FlexrayNodeParameters& cOut)
{
    cOut.pAllowHaltDueToClock = cxxIn.pAllowHaltDueToClock;
    cOut.pAllowPassiveToActive = cxxIn.pAllowPassiveToActive;
    cOut.pChannels = static_cast<SilKit_FlexrayChannel>(cxxIn.pChannels);
    cOut.pClusterDriftDamping = cxxIn.pClusterDriftDamping;
    cOut.pdAcceptedStartupRange = cxxIn.pdAcceptedStartupRange;
    cOut.pdListenTimeout = cxxIn.pdListenTimeout;
    cOut.pKeySlotId = cxxIn.pKeySlotId;
    cOut.pKeySlotOnlyEnabled = cxxIn.pKeySlotOnlyEnabled;
    cOut.pKeySlotUsedForStartup = cxxIn.pKeySlotUsedForStartup;
    cOut.pKeySlotUsedForSync = cxxIn.pKeySlotUsedForSync;
    cOut.pLatestTx = cxxIn.pLatestTx;
    cOut.pMacroInitialOffsetA = cxxIn.pMacroInitialOffsetA;
    cOut.pMacroInitialOffsetB = cxxIn.pMacroInitialOffsetB;
    cOut.pMicroInitialOffsetA = cxxIn.pMicroInitialOffsetA;
    cOut.pMicroInitialOffsetB = cxxIn.pMicroInitialOffsetB;
    cOut.pMicroPerCycle = cxxIn.pMicroPerCycle;
    cOut.pOffsetCorrectionOut = cxxIn.pOffsetCorrectionOut;
    cOut.pOffsetCorrectionStart = cxxIn.pOffsetCorrectionStart;
    cOut.pRateCorrectionOut = cxxIn.pRateCorrectionOut;
    cOut.pWakeupChannel = static_cast<SilKit_FlexrayChannel>(cxxIn.pWakeupChannel);
    cOut.pWakeupPattern = cxxIn.pWakeupPattern;
    cOut.pdMicrotick = static_cast<SilKit_FlexrayClockPeriod>(cxxIn.pdMicrotick);
    cOut.pSamplesPerMicrotick = cxxIn.pSamplesPerMicrotick;
}

inline void assignCxxToC(const SilKit::Services::Flexray::FlexrayTxBufferConfig& cxxIn, SilKit_FlexrayTxBufferConfig& cOut)
{
    cOut.channels = static_cast<SilKit_FlexrayChannel>(cxxIn.channels);
    cOut.slotId = cxxIn.slotId;
    cOut.offset = cxxIn.offset;
    cOut.repetition = cxxIn.repetition;
    cOut.hasPayloadPreambleIndicator = cxxIn.hasPayloadPreambleIndicator;
    cOut.headerCrc = cxxIn.headerCrc;
    cOut.transmissionMode = static_cast<SilKit_FlexrayTransmissionMode>(cxxIn.transmissionMode);
}

// -----------------------------------
// Cxx Experimental::NetworkSimulation::Flexray to C
// -----------------------------------

inline void assignCxxToC(const SilKit::Experimental::NetworkSimulation::Flexray::FlexrayHostCommand& cxxIn,
                         SilKit_Experimental_NetSim_FlexrayHostCommand& cOut)
{
    cOut.chiCommand = (SilKit_FlexrayChiCommand)cxxIn.command;
}

inline void assignCxxToC(const SilKit::Experimental::NetworkSimulation::Flexray::FlexrayControllerConfig& cxxIn,
                  SilKit_Experimental_NetSim_FlexrayControllerConfig& cOut)
{
    assignCxxToC(cxxIn.clusterParams, *cOut.clusterParams);
    assignCxxToC(cxxIn.nodeParams, *cOut.nodeParams);

    cOut.numBufferConfigs = static_cast<uint32_t>(cxxIn.bufferConfigs.size());
    size_t i = 0;
    for (const auto& cxxTxBufferConfig : cxxIn.bufferConfigs)
    {
        assignCxxToC(cxxTxBufferConfig, cOut.bufferConfigs[i++]);
    }
}

inline void assignCxxToC(const SilKit::Experimental::NetworkSimulation::Flexray::FlexrayTxBufferConfigUpdate& cxxIn,
                  SilKit_Experimental_NetSim_FlexrayTxBufferConfigUpdate& cOut)
{
    cOut.txBufferIdx = cxxIn.txBufferIndex;
    assignCxxToC(cxxIn.txBufferConfig, *cOut.txBufferConfig);
}

inline void assignCxxToC(const SilKit::Experimental::NetworkSimulation::Flexray::FlexrayTxBufferUpdate& cxxIn,
                  SilKit_Experimental_NetSim_FlexrayTxBufferUpdate& cOut)
{
    cOut.txBufferIndex = cxxIn.txBufferIndex;
    cOut.payloadDataValid = cxxIn.payloadDataValid;
    cOut.payload = ToSilKitByteVector(cxxIn.payload);
}

// -----------------------------------
// Cxx Services::Ethernet to C
// -----------------------------------

inline void assignCxxToC(const SilKit::Services::Ethernet::EthernetFrame& cxxIn, SilKit_EthernetFrame& cOut)
{
    auto* dataPointer = !cxxIn.raw.empty() ? cxxIn.raw.data() : nullptr;
    cOut.raw = {dataPointer, cxxIn.raw.size()};
}

inline void assignCxxToC(const SilKit::Services::Ethernet::EthernetFrameEvent& cxxIn, SilKit_EthernetFrameEvent& cOut)
{
    assignCxxToC(cxxIn.frame, *cOut.ethernetFrame);
    cOut.timestamp = cxxIn.timestamp.count();
    cOut.direction = static_cast<SilKit_Direction>(cxxIn.direction);
    cOut.userContext = cxxIn.userContext;
}


inline void assignCxxToC(const SilKit::Services::Ethernet::EthernetFrameTransmitEvent& cxxIn,
                         SilKit_EthernetFrameTransmitEvent& cOut)
{
    cOut.status = (SilKit_EthernetTransmitStatus)cxxIn.status;
    cOut.timestamp = cxxIn.timestamp.count();
    cOut.userContext = cxxIn.userContext;
}


inline void assignCxxToC(const SilKit::Services::Ethernet::EthernetStateChangeEvent& cxxIn,
                         SilKit_EthernetStateChangeEvent& cOut)
{
    cOut.state = (SilKit_EthernetState)cxxIn.state;
    cOut.timestamp = cxxIn.timestamp.count();
}

inline void assignCxxToC(const SilKit::Services::Ethernet::EthernetBitrateChangeEvent& cxxIn,
                         SilKit_EthernetBitrateChangeEvent& cOut)
{
    cOut.bitrate = cxxIn.bitrate;
    cOut.timestamp = cxxIn.timestamp.count();
}


// -----------------------------------
// Cxx Experimental::NetworkSimulation::Ethernet to C
// -----------------------------------

inline void assignCxxToC(const SilKit::Experimental::NetworkSimulation::Ethernet::EthernetFrameRequest& cxxIn,
                         SilKit_Experimental_NetSim_EthernetFrameRequest& cOut)
{
    assignCxxToC(cxxIn.ethernetFrame, *cOut.ethernetFrame);
    cOut.userContext = cxxIn.userContext;
}

inline void assignCxxToC(const SilKit::Experimental::NetworkSimulation::Ethernet::EthernetControllerMode& cxxIn,
                         SilKit_Experimental_NetSim_EthernetControllerMode& cOut)
{
    cOut.mode = (SilKit_EthernetControllerMode)cxxIn.mode;
}

// -----------------------------------
// Cxx Services::Lin to C
// -----------------------------------

inline void assignCxxToC(const SilKit::Services::Lin::LinFrame& cxxIn, SilKit_LinFrame& cOut)
{
    cOut.id = cxxIn.id;
    cOut.checksumModel = static_cast<SilKit_LinChecksumModel>(cxxIn.checksumModel);
    cOut.dataLength = cxxIn.dataLength;
    std::copy_n(cxxIn.data.data(), cxxIn.data.size(), cOut.data);
}

inline void assignCxxToC(const SilKit::Services::Lin::LinFrameStatusEvent& cxxIn, SilKit_LinFrameStatusEvent& cOut)
{
    assignCxxToC(cxxIn.frame, *cOut.frame);
    cOut.timestamp = (SilKit_NanosecondsTime)cxxIn.timestamp.count();
    cOut.status = (SilKit_LinFrameStatus)cxxIn.status;
}

inline void assignCxxToC(const SilKit::Services::Lin::LinWakeupEvent& cxxIn, SilKit_LinWakeupEvent& cOut)
{
    cOut.timestamp = (SilKit_NanosecondsTime)cxxIn.timestamp.count();
    cOut.direction = (SilKit_Direction)cxxIn.direction;
}

inline void assignCxxToC(const SilKit::Services::Lin::LinGoToSleepEvent& cxxIn, SilKit_LinGoToSleepEvent& cOut)
{
    cOut.timestamp = (SilKit_NanosecondsTime)cxxIn.timestamp.count();
}

inline void assignCxxToC(const SilKit::Services::Lin::LinFrameResponse& cxxIn, SilKit_LinFrameResponse& cOut)
{
    assignCxxToC(cxxIn.frame, *cOut.frame);
    cOut.responseMode = (SilKit_LinFrameResponseMode)cxxIn.responseMode;
}

inline void assignCxxToC(const SilKit::Services::Lin::LinSendFrameHeaderRequest& cxxIn,
                         SilKit_LinSendFrameHeaderRequest& cOut)
{
    cOut.id = cxxIn.id;
    cOut.timestamp = (SilKit_NanosecondsTime)cxxIn.timestamp.count();
}

// -----------------------------------
// Cxx Experimental::NetworkSimulation::Lin to C
// -----------------------------------

inline void assignCxxToC(const SilKit::Experimental::NetworkSimulation::Lin::LinFrameRequest& cxxIn, SilKit_Experimental_NetSim_LinFrameRequest& cOut)
{
    assignCxxToC(cxxIn.frame, *cOut.frame);
    cOut.responseType = (SilKit_LinFrameResponseType)cxxIn.responseType;
}

inline void assignCxxToC(const SilKit::Experimental::NetworkSimulation::Lin::LinFrameHeaderRequest& cxxIn,
                         SilKit_Experimental_NetSim_LinFrameHeaderRequest& cOut)
{
    cOut.id = cxxIn.id;
}

inline void assignCxxToC(const SilKit::Experimental::NetworkSimulation::Lin::LinWakeupPulse& cxxIn,
                         SilKit_Experimental_NetSim_LinWakeupPulse& cOut)
{
    cOut.timestamp = (SilKit_NanosecondsTime)cxxIn.timestamp.count();
}

inline void assignCxxToC(const SilKit::Experimental::NetworkSimulation::Lin::LinControllerConfig& cxxIn,
                         SilKit_Experimental_NetSim_LinControllerConfig& cOut)
{
    cOut.baudRate = cxxIn.baudRate;
    cOut.controllerMode = static_cast<SilKit_LinControllerMode>(cxxIn.controllerMode);
    size_t i = 0;
    for (const auto& cxxFrameResponse : cxxIn.frameResponses)
    {
        assignCxxToC(cxxFrameResponse, cOut.frameResponses[i++]);
    }
    cOut.numFrameResponses = cxxIn.frameResponses.size();
    cOut.simulationMode = (SilKit_Experimental_NetSim_LinSimulationMode)cxxIn.simulationMode;
}

inline void assignCxxToC(const SilKit::Experimental::NetworkSimulation::Lin::LinFrameResponseUpdate& cxxIn,
                         SilKit_Experimental_NetSim_LinFrameResponseUpdate& cOut)
{
    size_t i = 0;
    for (const auto& cxxFrameResponse : cxxIn.frameResponses)
    {
        assignCxxToC(cxxFrameResponse, cOut.frameResponses[i++]);
    }
    cOut.numFrameResponses = cxxIn.frameResponses.size();
}

inline void assignCxxToC(const SilKit::Experimental::NetworkSimulation::Lin::LinControllerStatusUpdate& cxxIn,
                         SilKit_Experimental_NetSim_LinControllerStatusUpdate& cOut)
{
    cOut.status = (SilKit_LinControllerStatus)cxxIn.status;
}

// ================================================================
// C to Cxx
// ================================================================

// --------------------------------
// C to Cxx Services::Can
// --------------------------------

inline void assignCToCxx(const SilKit_CanFrame* cIn, SilKit::Services::Can::CanFrame& cxxOut)
{
    cxxOut.canId = cIn->id;
    cxxOut.flags = cIn->flags;
    cxxOut.dlc = cIn->dlc;
    cxxOut.sdt = cIn->sdt;
    cxxOut.vcid = cIn->vcid;
    cxxOut.af = cIn->af;
    cxxOut.dataField = SilKit::Util::ToSpan(cIn->data);
}


inline void assignCToCxx(const SilKit_CanFrameEvent* cIn, SilKit::Services::Can::CanFrameEvent& cxxOut)
{
    cxxOut.timestamp = std::chrono::nanoseconds{cIn->timestamp};
    cxxOut.frame.canId = cIn->frame->id;
    cxxOut.frame.flags = cIn->frame->flags;
    cxxOut.frame.dlc = cIn->frame->dlc;
    cxxOut.frame.sdt = cIn->frame->sdt;
    cxxOut.frame.vcid = cIn->frame->vcid;
    cxxOut.frame.af = cIn->frame->af;
    cxxOut.frame.dataField = SilKit::Util::ToSpan(cIn->frame->data);
    cxxOut.direction = static_cast<SilKit::Services::TransmitDirection>(cIn->direction);
    cxxOut.userContext = cIn->userContext;
}

inline void assignCToCxx(const SilKit_CanFrameTransmitEvent* cIn, SilKit::Services::Can::CanFrameTransmitEvent& cxxOut) {
    cxxOut.userContext = cIn->userContext;
    cxxOut.timestamp = std::chrono::nanoseconds{cIn->timestamp};
    cxxOut.status = static_cast<SilKit::Services::Can::CanTransmitStatus>(cIn->status);
    if (SK_ID_GET_VERSION(SilKit_Struct_GetId((*cIn))) >= 2)
    {
        cxxOut.canId = cIn->canId;
    }
}


inline void assignCToCxx(const SilKit_CanStateChangeEvent* cIn, SilKit::Services::Can::CanStateChangeEvent& cxxOut) 
{
    cxxOut.timestamp = std::chrono::nanoseconds{cIn->timestamp};
    cxxOut.state = static_cast<SilKit::Services::Can::CanControllerState>(cIn->state);
}

inline void assignCToCxx(const SilKit_CanErrorStateChangeEvent* cIn, SilKit::Services::Can::CanErrorStateChangeEvent& cxxOut)
{
    cxxOut.timestamp = std::chrono::nanoseconds{cIn->timestamp};
    cxxOut.errorState = static_cast<SilKit::Services::Can::CanErrorState>(cIn->errorState);
}

// --------------------------------
// C to Cxx Experimental::NetworkSimulation::Can
// --------------------------------

inline void assignCToCxx(const SilKit_Experimental_NetSim_CanFrameRequest* cIn, SilKit::Experimental::NetworkSimulation::Can::CanFrameRequest& cxxOut)
{
    cxxOut.userContext = cIn->userContext;
    assignCToCxx(cIn->frame, cxxOut.frame);
}

inline void assignCToCxx(const SilKit_Experimental_NetSim_CanConfigureBaudrate* cIn,
                         SilKit::Experimental::NetworkSimulation::Can::CanConfigureBaudrate& cxxOut)
{
    cxxOut.baudRate = cIn->rate;
    cxxOut.fdBaudRate = cIn->fdRate;
    cxxOut.xlBaudRate = cIn->xlRate;
}

inline void assignCToCxx(const SilKit_Experimental_NetSim_CanControllerMode* cIn,
                         SilKit::Experimental::NetworkSimulation::Can::CanControllerMode& cxxOut)
{
    cxxOut.state = static_cast<SilKit::Services::Can::CanControllerState>(cIn->state);
    cxxOut.canControllerModeFlags = cIn->canControllerModeFlags;
}

// --------------------------------
// C to Cxx Services::Flexray
// --------------------------------

inline void assignCToCxx(const SilKit_FlexrayHeader* cIn, SilKit::Services::Flexray::FlexrayHeader& cxxOut) 
{
    cxxOut.cycleCount = cIn->cycleCount;
    cxxOut.flags = cIn->flags;
    cxxOut.frameId = cIn->frameId;
    cxxOut.headerCrc = cIn->headerCrc;
    cxxOut.payloadLength = cIn->payloadLength;
}

inline void assignCToCxx(const SilKit_FlexrayFrame* cIn, SilKit::Services::Flexray::FlexrayFrame& cxxOut)
{
    cxxOut.payload = SilKit::Util::ToSpan(cIn->payload);
    assignCToCxx(cIn->header, cxxOut.header);
}

inline void assignCToCxx(const SilKit_FlexrayFrameEvent* cIn, SilKit::Services::Flexray::FlexrayFrameEvent& cxxOut)
{
    cxxOut.timestamp = std::chrono::nanoseconds{cIn->timestamp};
    cxxOut.channel = static_cast<SilKit::Services::Flexray::FlexrayChannel>(cIn->channel);
    assignCToCxx(cIn->frame, cxxOut.frame);
}

inline void assignCToCxx(const SilKit_FlexrayFrameTransmitEvent* cIn,
                  SilKit::Services::Flexray::FlexrayFrameTransmitEvent& cxxOut)
{
    cxxOut.timestamp = std::chrono::nanoseconds{cIn->timestamp};
    cxxOut.channel = static_cast<SilKit::Services::Flexray::FlexrayChannel>(cIn->channel);
    cxxOut.txBufferIndex = cIn->txBufferIndex;
    assignCToCxx(cIn->frame, cxxOut.frame);    
}

inline void assignCToCxx(const SilKit_FlexraySymbolEvent* cIn, SilKit::Services::Flexray::FlexraySymbolEvent& cxxOut)
{
    cxxOut.timestamp = std::chrono::nanoseconds{cIn->timestamp};
    cxxOut.channel = static_cast<SilKit::Services::Flexray::FlexrayChannel>(cIn->channel);
    cxxOut.pattern = static_cast<SilKit::Services::Flexray::FlexraySymbolPattern>(cIn->pattern);
}

inline void assignCToCxx(const SilKit_FlexraySymbolTransmitEvent* cIn,
                  SilKit::Services::Flexray::FlexraySymbolTransmitEvent& cxxOut)
{
    cxxOut.timestamp = std::chrono::nanoseconds{cIn->timestamp};
    cxxOut.channel = static_cast<SilKit::Services::Flexray::FlexrayChannel>(cIn->channel);
    cxxOut.pattern = static_cast<SilKit::Services::Flexray::FlexraySymbolPattern>(cIn->pattern);
}

inline void assignCToCxx(const SilKit_FlexrayCycleStartEvent* cIn, SilKit::Services::Flexray::FlexrayCycleStartEvent& cxxOut)
{
    cxxOut.timestamp = std::chrono::nanoseconds{cIn->timestamp};
    cxxOut.cycleCounter = cIn->cycleCounter;
}

inline void assignCToCxx(const SilKit_FlexrayPocStatusEvent* cIn, SilKit::Services::Flexray::FlexrayPocStatusEvent& cxxOut)
{
    cxxOut.timestamp = std::chrono::nanoseconds{cIn->timestamp};
    cxxOut.state = static_cast<SilKit::Services::Flexray::FlexrayPocState>(cIn->state);
    cxxOut.chiHaltRequest = cIn->chiHaltRequest;
    cxxOut.chiReadyRequest = cIn->chiReadyRequest;
    cxxOut.coldstartNoise = cIn->coldstartNoise;
    cxxOut.errorMode = static_cast<SilKit::Services::Flexray::FlexrayErrorModeType>(cIn->errorMode);
    cxxOut.freeze = cIn->freeze;
    cxxOut.slotMode = static_cast<SilKit::Services::Flexray::FlexraySlotModeType>(cIn->slotMode);
    cxxOut.startupState = static_cast<SilKit::Services::Flexray::FlexrayStartupStateType>(cIn->startupState);
    cxxOut.wakeupStatus = static_cast<SilKit::Services::Flexray::FlexrayWakeupStatusType>(cIn->wakeupStatus);
}

inline void assignCToCxx(const SilKit_FlexrayTxBufferConfig* cIn, SilKit::Services::Flexray::FlexrayTxBufferConfig& cxxOut)
{
    cxxOut.channels = (SilKit::Services::Flexray::FlexrayChannel)cIn->channels;
    cxxOut.slotId = cIn->slotId;
    cxxOut.offset = cIn->offset;
    cxxOut.repetition = cIn->repetition;
    cxxOut.hasPayloadPreambleIndicator = cIn->hasPayloadPreambleIndicator == SilKit_True;
    cxxOut.headerCrc = cIn->headerCrc;
    cxxOut.transmissionMode = (SilKit::Services::Flexray::FlexrayTransmissionMode)cIn->transmissionMode;
}

inline void assignCToCxx(const SilKit_FlexrayClusterParameters* cIn,
                  SilKit::Services::Flexray::FlexrayClusterParameters& cxxOut)
{
    cxxOut.gColdstartAttempts = cIn->gColdstartAttempts;
    cxxOut.gCycleCountMax = cIn->gCycleCountMax;
    cxxOut.gdActionPointOffset = cIn->gdActionPointOffset;
    cxxOut.gdDynamicSlotIdlePhase = cIn->gdDynamicSlotIdlePhase;
    cxxOut.gdMiniSlot = cIn->gdMiniSlot;
    cxxOut.gdMiniSlotActionPointOffset = cIn->gdMiniSlotActionPointOffset;
    cxxOut.gdStaticSlot = cIn->gdStaticSlot;
    cxxOut.gdSymbolWindow = cIn->gdSymbolWindow;
    cxxOut.gdSymbolWindowActionPointOffset = cIn->gdSymbolWindowActionPointOffset;
    cxxOut.gdTSSTransmitter = cIn->gdTSSTransmitter;
    cxxOut.gdWakeupTxActive = cIn->gdWakeupTxActive;
    cxxOut.gdWakeupTxIdle = cIn->gdWakeupTxIdle;
    cxxOut.gListenNoise = cIn->gListenNoise;
    cxxOut.gMacroPerCycle = cIn->gMacroPerCycle;
    cxxOut.gMaxWithoutClockCorrectionFatal = cIn->gMaxWithoutClockCorrectionFatal;
    cxxOut.gMaxWithoutClockCorrectionPassive = cIn->gMaxWithoutClockCorrectionPassive;
    cxxOut.gNumberOfMiniSlots = cIn->gNumberOfMiniSlots;
    cxxOut.gNumberOfStaticSlots = cIn->gNumberOfStaticSlots;
    cxxOut.gPayloadLengthStatic = cIn->gPayloadLengthStatic;
    cxxOut.gSyncFrameIDCountMax = cIn->gSyncFrameIDCountMax;
}

inline void assignCToCxx(const SilKit_FlexrayNodeParameters* cIn, SilKit::Services::Flexray::FlexrayNodeParameters& cxxOut)
{
    cxxOut.pAllowHaltDueToClock = cIn->pAllowHaltDueToClock;
    cxxOut.pAllowPassiveToActive = cIn->pAllowPassiveToActive;
    cxxOut.pChannels = (SilKit::Services::Flexray::FlexrayChannel)cIn->pChannels;
    cxxOut.pClusterDriftDamping = cIn->pClusterDriftDamping;
    cxxOut.pdAcceptedStartupRange = cIn->pdAcceptedStartupRange;
    cxxOut.pdListenTimeout = cIn->pdListenTimeout;
    cxxOut.pKeySlotId = cIn->pKeySlotId;
    cxxOut.pKeySlotOnlyEnabled = cIn->pKeySlotOnlyEnabled;
    cxxOut.pKeySlotUsedForStartup = cIn->pKeySlotUsedForStartup;
    cxxOut.pKeySlotUsedForSync = cIn->pKeySlotUsedForSync;
    cxxOut.pLatestTx = cIn->pLatestTx;
    cxxOut.pMacroInitialOffsetA = cIn->pMacroInitialOffsetA;
    cxxOut.pMacroInitialOffsetB = cIn->pMacroInitialOffsetB;
    cxxOut.pMicroInitialOffsetA = cIn->pMicroInitialOffsetA;
    cxxOut.pMicroInitialOffsetB = cIn->pMicroInitialOffsetB;
    cxxOut.pMicroPerCycle = cIn->pMicroPerCycle;
    cxxOut.pOffsetCorrectionOut = cIn->pOffsetCorrectionOut;
    cxxOut.pOffsetCorrectionStart = cIn->pOffsetCorrectionStart;
    cxxOut.pRateCorrectionOut = cIn->pRateCorrectionOut;
    cxxOut.pWakeupChannel = (SilKit::Services::Flexray::FlexrayChannel)cIn->pWakeupChannel;
    cxxOut.pWakeupPattern = cIn->pWakeupPattern;
    cxxOut.pdMicrotick = (SilKit::Services::Flexray::FlexrayClockPeriod)cIn->pdMicrotick;
    cxxOut.pSamplesPerMicrotick = cIn->pSamplesPerMicrotick;
}

// -----------------------------------
// C to Cxx Experimental::NetworkSimulation::Flexray
// -----------------------------------

inline void assignCToCxx(const SilKit_Experimental_NetSim_FlexrayHostCommand* cIn,
                         SilKit::Experimental::NetworkSimulation::Flexray::FlexrayHostCommand& cxxOut)
{
    cxxOut.command = static_cast<SilKit::Experimental::NetworkSimulation::Flexray::FlexrayChiCommand>(cIn->chiCommand);
}

inline void assignCToCxx(const SilKit_Experimental_NetSim_FlexrayControllerConfig* cIn,
                         SilKit::Experimental::NetworkSimulation::Flexray::FlexrayControllerConfig& cxxOut)
{
    assignCToCxx(cIn->clusterParams, cxxOut.clusterParams);
    assignCToCxx(cIn->nodeParams, cxxOut.nodeParams);

    for (uint32_t i = 0; i < cIn->numBufferConfigs; i++)
    {
        SilKit::Services::Flexray::FlexrayTxBufferConfig txBufferConfig{};
        assignCToCxx(&cIn->bufferConfigs[i], txBufferConfig);
        cxxOut.bufferConfigs.push_back(std::move(txBufferConfig));
    }
}

inline void assignCToCxx(const SilKit_Experimental_NetSim_FlexrayTxBufferConfigUpdate* cIn,
                         SilKit::Experimental::NetworkSimulation::Flexray::FlexrayTxBufferConfigUpdate& cxxOut)
{
    cxxOut.txBufferIndex = cIn->txBufferIdx;
    assignCToCxx(cIn->txBufferConfig, cxxOut.txBufferConfig);
}

inline void assignCToCxx(const SilKit_Experimental_NetSim_FlexrayTxBufferUpdate* cIn,
                         SilKit::Experimental::NetworkSimulation::Flexray::FlexrayTxBufferUpdate& cxxOut)
{
    cxxOut.txBufferIndex = cIn->txBufferIndex;
    cxxOut.payloadDataValid = cIn->payloadDataValid == SilKit_True;
    if (cIn->payloadDataValid)
    {
        cxxOut.payload = SilKit::Util::ToSpan(cIn->payload);
    }
}

// --------------------------------
// C to Cxx Services::Ethernet
// --------------------------------

inline void assignCToCxx(const SilKit_EthernetFrame* cIn,
                         SilKit::Services::Ethernet::EthernetFrame& cxxOut)
{
    cxxOut.raw = SilKit::Util::ToSpan(cIn->raw);
}

inline void assignCToCxx(const SilKit_EthernetFrameEvent* cIn,
                         SilKit::Services::Ethernet::EthernetFrameEvent& cxxOut)
{
    assignCToCxx(cIn->ethernetFrame, cxxOut.frame);
    cxxOut.timestamp = std::chrono::nanoseconds{cIn->timestamp};
    cxxOut.direction = static_cast<SilKit::Services::TransmitDirection>(cIn->direction);
    cxxOut.userContext = cIn->userContext;
}

inline void assignCToCxx(const SilKit_EthernetFrameTransmitEvent* cIn,
                         SilKit::Services::Ethernet::EthernetFrameTransmitEvent& cxxOut)
{
    cxxOut.timestamp = std::chrono::nanoseconds{cIn->timestamp};
    cxxOut.status = static_cast<SilKit::Services::Ethernet::EthernetTransmitStatus>(cIn->status);
    cxxOut.userContext = cIn->userContext;
}

inline void assignCToCxx(const SilKit_EthernetBitrateChangeEvent* cIn,
                         SilKit::Services::Ethernet::EthernetBitrateChangeEvent& cxxOut)
{
    cxxOut.timestamp = std::chrono::nanoseconds{cIn->timestamp};
    cxxOut.bitrate = static_cast<SilKit::Services::Ethernet::EthernetBitrate>(cIn->bitrate);
}

inline void assignCToCxx(const SilKit_EthernetStateChangeEvent* cIn,
                         SilKit::Services::Ethernet::EthernetStateChangeEvent& cxxOut)
{
    cxxOut.timestamp = std::chrono::nanoseconds{cIn->timestamp};
    cxxOut.state = static_cast<SilKit::Services::Ethernet::EthernetState>(cIn->state);
}

// -----------------------------------
// C to Cxx Experimental::NetworkSimulation::Ethernet
// -----------------------------------

inline void assignCToCxx(const SilKit_Experimental_NetSim_EthernetControllerMode* cIn,
                         SilKit::Experimental::NetworkSimulation::Ethernet::EthernetControllerMode& cxxOut)
{
    cxxOut.mode = static_cast<SilKit::Experimental::NetworkSimulation::Ethernet::EthernetMode>(cIn->mode);
}

inline void assignCToCxx(const SilKit_Experimental_NetSim_EthernetFrameRequest* cIn,
                         SilKit::Experimental::NetworkSimulation::Ethernet::EthernetFrameRequest& cxxOut)
{
    assignCToCxx(cIn->ethernetFrame, cxxOut.ethernetFrame);
    cxxOut.userContext = cIn->userContext;
}

// --------------------------------
// C to Cxx Services::Lin
// --------------------------------

inline void assignCToCxx(const SilKit_LinFrame* cIn, SilKit::Services::Lin::LinFrame& cxxOut)
{
    cxxOut.id = static_cast<SilKit::Services::Lin::LinId>(cIn->id);
    cxxOut.checksumModel = static_cast<SilKit::Services::Lin::LinChecksumModel>(cIn->checksumModel);
    cxxOut.dataLength = static_cast<SilKit::Services::Lin::LinDataLength>(cIn->dataLength);
    memcpy(cxxOut.data.data(), cIn->data, 8);
}

inline void assignCToCxx(const SilKit_LinFrameResponse* cIn, SilKit::Services::Lin::LinFrameResponse& cxxOut)
{
    assignCToCxx(cIn->frame, cxxOut.frame);
    cxxOut.responseMode = static_cast<SilKit::Services::Lin::LinFrameResponseMode>(cIn->responseMode);
}

inline void assignCToCxx(const SilKit_LinFrameStatusEvent* cIn, SilKit::Services::Lin::LinFrameStatusEvent& cxxOut)
{
    assignCToCxx(cIn->frame, cxxOut.frame);
    cxxOut.status = static_cast<SilKit::Services::Lin::LinFrameStatus>(cIn->status);
    cxxOut.timestamp = std::chrono::nanoseconds{cIn->timestamp};
}

inline void assignCToCxx(const SilKit_LinSendFrameHeaderRequest* cIn,
                         SilKit::Services::Lin::LinSendFrameHeaderRequest& cxxOut)
{
    cxxOut.id = cIn->id;
    cxxOut.timestamp = std::chrono::nanoseconds{cIn->timestamp};
}

inline void assignCToCxx(const SilKit_LinWakeupEvent* cIn, SilKit::Services::Lin::LinWakeupEvent& cxxOut)
{
    cxxOut.timestamp = std::chrono::nanoseconds{cIn->timestamp};
    cxxOut.direction = static_cast<SilKit::Services::TransmitDirection>(cIn->direction);
}


// -----------------------------------
// C to Cxx Experimental::NetworkSimulation::Lin
// -----------------------------------

inline void assignCToCxx(const SilKit_Experimental_NetSim_LinFrameRequest* cIn,
                         SilKit::Experimental::NetworkSimulation::Lin::LinFrameRequest& cxxOut)
{
    assignCToCxx(cIn->frame, cxxOut.frame);
    cxxOut.responseType = static_cast<SilKit::Services::Lin::LinFrameResponseType>(cIn->responseType);
}

inline void assignCToCxx(const SilKit_Experimental_NetSim_LinFrameHeaderRequest* cIn,
                         SilKit::Experimental::NetworkSimulation::Lin::LinFrameHeaderRequest& cxxOut)
{
    cxxOut.id = cIn->id;
}

inline void assignCToCxx(const SilKit_Experimental_NetSim_LinWakeupPulse* cIn,
                         SilKit::Experimental::NetworkSimulation::Lin::LinWakeupPulse& cxxOut)
{
    cxxOut.timestamp = std::chrono::nanoseconds{cIn->timestamp};
}

inline void assignCToCxx(const SilKit_Experimental_NetSim_LinControllerConfig* cIn,
                         SilKit::Experimental::NetworkSimulation::Lin::LinControllerConfig& cxxOut)
{
    cxxOut.baudRate = cIn->baudRate;
    cxxOut.controllerMode = static_cast<SilKit::Services::Lin::LinControllerMode>(cIn->controllerMode);
    for (uint32_t i = 0; i < cIn->numFrameResponses; i++)
    {
        SilKit::Services::Lin::LinFrameResponse frameResponse{};
        assignCToCxx(&cIn->frameResponses[i], frameResponse);
        cxxOut.frameResponses.push_back(std::move(frameResponse));
    }
    cxxOut.simulationMode = static_cast<SilKit::Experimental::NetworkSimulation::Lin::LinControllerConfig::SimulationMode>(cIn->simulationMode);
}

inline void assignCToCxx(const SilKit_Experimental_NetSim_LinFrameResponseUpdate* cIn,
                         SilKit::Experimental::NetworkSimulation::Lin::LinFrameResponseUpdate& cxxOut)
{
    for (uint32_t i = 0; i < cIn->numFrameResponses; i++)
    {
        SilKit::Services::Lin::LinFrameResponse frameResponse{};
        assignCToCxx(&cIn->frameResponses[i], frameResponse);
        cxxOut.frameResponses.push_back(std::move(frameResponse));
    }
}

inline void assignCToCxx(const SilKit_Experimental_NetSim_LinControllerStatusUpdate* cIn,
                         SilKit::Experimental::NetworkSimulation::Lin::LinControllerStatusUpdate& cxxOut)
{
    cxxOut.status = static_cast<SilKit::Services::Lin::LinControllerStatus>(cIn->status);
    cxxOut.timestamp = std::chrono::nanoseconds{cIn->timestamp};
}

} //namespace