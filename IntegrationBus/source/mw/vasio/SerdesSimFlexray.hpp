// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "MessageBuffer.hpp"

#include "ib/sim/fr/FrDatatypes.hpp"

namespace ib {
namespace sim {
namespace fr {

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const Header& header)
{
    buffer
        << header.flags
        << header.frameId
        << header.payloadLength
        << header.headerCrc
        << header.cycleCount;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, Header& header)
{
    buffer
        >> header.flags
        >> header.frameId
        >> header.payloadLength
        >> header.headerCrc
        >> header.cycleCount;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const Frame& frame)
{
    buffer
        << frame.header
        << frame.payload;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, Frame& frame)
{
    buffer
        >> frame.header
        >> frame.payload;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const FrMessage& msg)
{
    buffer
        << msg.timestamp
        << msg.channel
        << msg.frame;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, FrMessage& msg)
{
    buffer
        >> msg.timestamp
        >> msg.channel
        >> msg.frame;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const FrMessageAck& msg)
{
    buffer
        << msg.timestamp
        << msg.txBufferIndex
        << msg.channel
        << msg.frame;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, FrMessageAck& msg)
{
    buffer
        >> msg.timestamp
        >> msg.txBufferIndex
        >> msg.channel
        >> msg.frame;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const FrSymbol& symbol)
{
    buffer
        << symbol.timestamp
        << symbol.channel
        << symbol.pattern;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, FrSymbol& symbol)
{
    buffer
        >> symbol.timestamp
        >> symbol.channel
        >> symbol.pattern;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const FrSymbolAck& ack)
{
    auto&& symbol = static_cast<const FrSymbol&>(ack);
    buffer
        << symbol;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, FrSymbolAck& ack)
{
    auto&& symbol = static_cast<FrSymbol&>(ack);
    buffer
        >> symbol;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const CycleStart& cycleStart)
{
    buffer
        << cycleStart.timestamp
        << cycleStart.cycleCounter;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, CycleStart& cycleStart)
{
    buffer
        >> cycleStart.timestamp
        >> cycleStart.cycleCounter;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const HostCommand& cmd)
{
    buffer
        << cmd.command;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, HostCommand& cmd)
{
    buffer
        >> cmd.command;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const ClusterParameters& clusterParam)
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
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, ClusterParameters& clusterParam)
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

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const NodeParameters& nodeParams)
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
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, NodeParameters& nodeParams)
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

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const TxBufferConfig& config)
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
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, TxBufferConfig& config)
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

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const ControllerConfig& config)
{
    buffer
        << config.clusterParams
        << config.nodeParams
        << config.bufferConfigs;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, ControllerConfig& config)
{
    buffer
        >> config.clusterParams
        >> config.nodeParams
        >> config.bufferConfigs;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const TxBufferConfigUpdate& update)
{
    buffer
        << update.txBufferIndex
        << update.txBufferConfig;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, TxBufferConfigUpdate& update)
{
    buffer
        >> update.txBufferIndex
        >> update.txBufferConfig;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const TxBufferUpdate& update)
{
    buffer
        << update.txBufferIndex
        << update.payloadDataValid
        << update.payload;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, TxBufferUpdate& update)
{
    buffer
        >> update.txBufferIndex
        >> update.payloadDataValid
        >> update.payload;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const ControllerStatus& status)
{
    buffer
        << status.timestamp
        << status.pocState;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, ControllerStatus& status)
{
    buffer
        >> status.timestamp
        >> status.pocState;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const PocStatus& status)
{
    buffer
        << status.timestamp
        << status.chiHaltRequest
        << status.coldstartNoise
        << status.errorMode
        << status.freeze
        << status.slotMode
        << status.startupState
        << status.state
        << status.wakeupStatus
        << status.chiReadyRequest;
    return buffer;
}

inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, PocStatus& status)
{
    buffer
      >> status.timestamp
      >> status.chiHaltRequest
      >> status.coldstartNoise
      >> status.errorMode
      >> status.freeze
      >> status.slotMode
      >> status.startupState
      >> status.state
      >> status.wakeupStatus
      >> status.chiReadyRequest;
    return buffer;
}
} // namespace fr
} // namespace sim
} // namespace ib
