// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "MessageBuffer.hpp"

#include "ib/sim/fr/FlexrayDatatypes.hpp"

namespace ib {
namespace sim {
namespace fr {

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const FlexrayHeader& header)
{
    buffer
        << header.flags
        << header.frameId
        << header.payloadLength
        << header.headerCrc
        << header.cycleCount;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, FlexrayHeader& header)
{
    buffer
        >> header.flags
        >> header.frameId
        >> header.payloadLength
        >> header.headerCrc
        >> header.cycleCount;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const FlexrayFrame& frame)
{
    buffer
        << frame.header
        << frame.payload;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, FlexrayFrame& frame)
{
    buffer
        >> frame.header
        >> frame.payload;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const FlexrayFrameEvent& msg)
{
    buffer
        << msg.timestamp
        << msg.channel
        << msg.frame;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, FlexrayFrameEvent& msg)
{
    buffer
        >> msg.timestamp
        >> msg.channel
        >> msg.frame;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const FlexrayFrameTransmitEvent& msg)
{
    buffer
        << msg.timestamp
        << msg.txBufferIndex
        << msg.channel
        << msg.frame;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, FlexrayFrameTransmitEvent& msg)
{
    buffer
        >> msg.timestamp
        >> msg.txBufferIndex
        >> msg.channel
        >> msg.frame;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const FlexraySymbolEvent& symbol)
{
    buffer
        << symbol.timestamp
        << symbol.channel
        << symbol.pattern;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, FlexraySymbolEvent& symbol)
{
    buffer
        >> symbol.timestamp
        >> symbol.channel
        >> symbol.pattern;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const FlexraySymbolTransmitEvent& ack)
{
    auto&& symbol = static_cast<const FlexraySymbolEvent&>(ack);
    buffer
        << symbol;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, FlexraySymbolTransmitEvent& ack)
{
    auto&& symbol = static_cast<FlexraySymbolEvent&>(ack);
    buffer
        >> symbol;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer,
                                         const FlexrayCycleStartEvent& flexrayCycleStartEvent)
{
    buffer
        << flexrayCycleStartEvent.timestamp
        << flexrayCycleStartEvent.cycleCounter;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, FlexrayCycleStartEvent& flexrayCycleStartEvent)
{
    buffer
        >> flexrayCycleStartEvent.timestamp
        >> flexrayCycleStartEvent.cycleCounter;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const FlexrayHostCommand& cmd)
{
    buffer
        << cmd.command;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, FlexrayHostCommand& cmd)
{
    buffer
        >> cmd.command;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const FlexrayClusterParameters& clusterParam)
{
    buffer
        << clusterParam.gColdstartAttempts
        << clusterParam.gCycleCountMax
        << clusterParam.gdActionPointOffset
        << clusterParam.gdDynamicSlotIdlePhase
        << clusterParam.gdMiniSlot
        << clusterParam.gdMiniSlotActionPointOffset
        << clusterParam.gdStaticSlot
        << clusterParam.gdSymbolWindow
        << clusterParam.gdSymbolWindowActionPointOffset
        << clusterParam.gdTSSTransmitter
        << clusterParam.gdWakeupTxActive
        << clusterParam.gdWakeupTxIdle
        << clusterParam.gListenNoise
        << clusterParam.gMacroPerCycle
        << clusterParam.gMaxWithoutClockCorrectionFatal
        << clusterParam.gMaxWithoutClockCorrectionPassive
        << clusterParam.gNumberOfMiniSlots
        << clusterParam.gNumberOfStaticSlots
        << clusterParam.gPayloadLengthStatic
        << clusterParam.gSyncFrameIDCountMax;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, FlexrayClusterParameters& clusterParam)
{
    buffer
        >> clusterParam.gColdstartAttempts
        >> clusterParam.gCycleCountMax
        >> clusterParam.gdActionPointOffset
        >> clusterParam.gdDynamicSlotIdlePhase
        >> clusterParam.gdMiniSlot
        >> clusterParam.gdMiniSlotActionPointOffset
        >> clusterParam.gdStaticSlot
        >> clusterParam.gdSymbolWindow
        >> clusterParam.gdSymbolWindowActionPointOffset
        >> clusterParam.gdTSSTransmitter
        >> clusterParam.gdWakeupTxActive
        >> clusterParam.gdWakeupTxIdle
        >> clusterParam.gListenNoise
        >> clusterParam.gMacroPerCycle
        >> clusterParam.gMaxWithoutClockCorrectionFatal
        >> clusterParam.gMaxWithoutClockCorrectionPassive
        >> clusterParam.gNumberOfMiniSlots
        >> clusterParam.gNumberOfStaticSlots
        >> clusterParam.gPayloadLengthStatic
        >> clusterParam.gSyncFrameIDCountMax;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const FlexrayNodeParameters& nodeParams)
{
    buffer
        << nodeParams.pAllowHaltDueToClock
        << nodeParams.pAllowPassiveToActive
        << nodeParams.pChannels
        << nodeParams.pClusterDriftDamping
        << nodeParams.pdAcceptedStartupRange
        << nodeParams.pdListenTimeout
        << nodeParams.pKeySlotId
        << nodeParams.pKeySlotOnlyEnabled
        << nodeParams.pKeySlotUsedForStartup
        << nodeParams.pKeySlotUsedForSync
        << nodeParams.pLatestTx
        << nodeParams.pMacroInitialOffsetA
        << nodeParams.pMacroInitialOffsetB
        << nodeParams.pMicroInitialOffsetA
        << nodeParams.pMicroInitialOffsetB
        << nodeParams.pMicroPerCycle
        << nodeParams.pOffsetCorrectionOut
        << nodeParams.pOffsetCorrectionStart
        << nodeParams.pRateCorrectionOut
        << nodeParams.pWakeupChannel
        << nodeParams.pWakeupPattern
        << nodeParams.pdMicrotick
        << nodeParams.pSamplesPerMicrotick;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, FlexrayNodeParameters& nodeParams)
{
    buffer
        >> nodeParams.pAllowHaltDueToClock
        >> nodeParams.pAllowPassiveToActive
        >> nodeParams.pChannels
        >> nodeParams.pClusterDriftDamping
        >> nodeParams.pdAcceptedStartupRange
        >> nodeParams.pdListenTimeout
        >> nodeParams.pKeySlotId
        >> nodeParams.pKeySlotOnlyEnabled
        >> nodeParams.pKeySlotUsedForStartup
        >> nodeParams.pKeySlotUsedForSync
        >> nodeParams.pLatestTx
        >> nodeParams.pMacroInitialOffsetA
        >> nodeParams.pMacroInitialOffsetB
        >> nodeParams.pMicroInitialOffsetA
        >> nodeParams.pMicroInitialOffsetB
        >> nodeParams.pMicroPerCycle
        >> nodeParams.pOffsetCorrectionOut
        >> nodeParams.pOffsetCorrectionStart
        >> nodeParams.pRateCorrectionOut
        >> nodeParams.pWakeupChannel
        >> nodeParams.pWakeupPattern
        >> nodeParams.pdMicrotick
        >> nodeParams.pSamplesPerMicrotick;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const FlexrayTxBufferConfig& config)
{
    buffer
        << config.channels
        << config.slotId
        << config.offset
        << config.repetition
        << config.hasPayloadPreambleIndicator
        << config.headerCrc
        << config.transmissionMode;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, FlexrayTxBufferConfig& config)
{
    buffer
        >> config.channels
        >> config.slotId
        >> config.offset
        >> config.repetition
        >> config.hasPayloadPreambleIndicator
        >> config.headerCrc
        >> config.transmissionMode;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const FlexrayControllerConfig& config)
{
    buffer
        << config.clusterParams
        << config.nodeParams
        << config.bufferConfigs;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, FlexrayControllerConfig& config)
{
    buffer
        >> config.clusterParams
        >> config.nodeParams
        >> config.bufferConfigs;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const FlexrayTxBufferConfigUpdate& update)
{
    buffer
        << update.txBufferIndex
        << update.txBufferConfig;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, FlexrayTxBufferConfigUpdate& update)
{
    buffer
        >> update.txBufferIndex
        >> update.txBufferConfig;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const FlexrayTxBufferUpdate& update)
{
    buffer
        << update.txBufferIndex
        << update.payloadDataValid
        << update.payload;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, FlexrayTxBufferUpdate& update)
{
    buffer
        >> update.txBufferIndex
        >> update.payloadDataValid
        >> update.payload;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer,
                                         const FlexrayPocStatusEvent& flexrayPocStatusEvent)
{
    buffer
        << flexrayPocStatusEvent.timestamp
        << flexrayPocStatusEvent.chiHaltRequest
        << flexrayPocStatusEvent.coldstartNoise
        << flexrayPocStatusEvent.errorMode
        << flexrayPocStatusEvent.freeze
        << flexrayPocStatusEvent.slotMode
        << flexrayPocStatusEvent.startupState
        << flexrayPocStatusEvent.state
        << flexrayPocStatusEvent.wakeupStatus
        << flexrayPocStatusEvent.chiReadyRequest;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, FlexrayPocStatusEvent& flexrayPocStatusEvent)
{
    buffer
      >> flexrayPocStatusEvent.timestamp
      >> flexrayPocStatusEvent.chiHaltRequest
      >> flexrayPocStatusEvent.coldstartNoise
      >> flexrayPocStatusEvent.errorMode
      >> flexrayPocStatusEvent.freeze
      >> flexrayPocStatusEvent.slotMode
      >> flexrayPocStatusEvent.startupState
      >> flexrayPocStatusEvent.state
      >> flexrayPocStatusEvent.wakeupStatus
      >> flexrayPocStatusEvent.chiReadyRequest;
    return buffer;
}
} // namespace fr
} // namespace sim
} // namespace ib
