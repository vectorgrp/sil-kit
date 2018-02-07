// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#include "FrDatatypeUtils.hpp"

namespace ib {
namespace sim {
namespace fr {

bool operator==(const Header& lhs, const Header& rhs)
{
    return lhs.flags == rhs.flags
        && lhs.frameId == rhs.frameId
        && lhs.headerCrc == rhs.headerCrc
        && lhs.cycleCount == rhs.cycleCount
        && lhs.payloadLength == rhs.payloadLength;
}

bool operator==(const Frame& lhs, const Frame& rhs)
{
    return lhs.header == rhs.header
        && lhs.payload == rhs.payload;
}

bool operator==(const FrMessage& lhs, const FrMessage& rhs)
{
    return lhs.channel == rhs.channel
        && lhs.frame == rhs.frame;
}

bool operator==(const FrMessageAck& lhs, const FrMessageAck& rhs)
{
    return lhs.txBufferIndex == rhs.txBufferIndex
        && lhs.channel == rhs.channel
        && lhs.frame == rhs.frame;
}

bool operator==(const FrSymbol& lhs, const FrSymbol& rhs)
{
    return lhs.channel == rhs.channel
        && lhs.pattern == rhs.pattern;
}

bool operator==(const TxBufferUpdate& lhs, const TxBufferUpdate& rhs)
{
    return lhs.txBufferIndex == rhs.txBufferIndex
        && lhs.payloadDataValid == rhs.payloadDataValid
        && lhs.payload == rhs.payload;
}

bool operator==(const ClusterParameters& lhs, const ClusterParameters& rhs)
{
    return lhs.gColdstartAttempts == rhs.gColdstartAttempts
        && lhs.gCycleCountMax == rhs.gCycleCountMax
        && lhs.gdActionPointOffset == rhs.gdActionPointOffset
        && lhs.gdDynamicSlotIdlePhase == rhs.gdDynamicSlotIdlePhase
        && lhs.gdMiniSlot == rhs.gdMiniSlot
        && lhs.gdMiniSlotActionPointOffset == rhs.gdMiniSlotActionPointOffset
        && lhs.gdStaticSlot == rhs.gdStaticSlot
        && lhs.gdSymbolWindow == rhs.gdSymbolWindow
        && lhs.gdSymbolWindowActionPointOffset == rhs.gdSymbolWindowActionPointOffset
        && lhs.gdTSSTransmitter == rhs.gdTSSTransmitter
        && lhs.gdWakeupTxActive == rhs.gdWakeupTxActive
        && lhs.gdWakeupTxIdle == rhs.gdWakeupTxIdle
        && lhs.gListenNoise == rhs.gListenNoise
        && lhs.gMacroPerCycle == rhs.gMacroPerCycle
        && lhs.gMaxWithoutClockCorrectionFatal == rhs.gMaxWithoutClockCorrectionFatal
        && lhs.gMaxWithoutClockCorrectionPassive == rhs.gMaxWithoutClockCorrectionPassive
        && lhs.gNumberOfMiniSlots == rhs.gNumberOfMiniSlots
        && lhs.gNumberOfStaticSlots == rhs.gNumberOfStaticSlots
        && lhs.gPayloadLengthStatic == rhs.gPayloadLengthStatic
        && lhs.gSyncFrameIDCountMax == rhs.gSyncFrameIDCountMax;
}

bool operator==(const NodeParameters& lhs, const NodeParameters& rhs)
{
    return lhs.pAllowHaltDueToClock == rhs.pAllowHaltDueToClock
        && lhs.pAllowPassiveToActive == rhs.pAllowPassiveToActive
        && lhs.pChannels == rhs.pChannels
        && lhs.pClusterDriftDamping == rhs.pClusterDriftDamping
        && lhs.pdAcceptedStartupRange == rhs.pdAcceptedStartupRange
        && lhs.pdListenTimeout == rhs.pdListenTimeout
        && lhs.pKeySlotId == rhs.pKeySlotId
        && lhs.pKeySlotOnlyEnabled == rhs.pKeySlotOnlyEnabled
        && lhs.pKeySlotUsedForStartup == rhs.pKeySlotUsedForStartup
        && lhs.pKeySlotUsedForSync == rhs.pKeySlotUsedForSync
        && lhs.pLatestTx == rhs.pLatestTx
        && lhs.pMacroInitialOffsetA == rhs.pMacroInitialOffsetA
        && lhs.pMacroInitialOffsetB == rhs.pMacroInitialOffsetB
        && lhs.pMicroInitialOffsetA == rhs.pMicroInitialOffsetA
        && lhs.pMicroInitialOffsetB == rhs.pMicroInitialOffsetB
        && lhs.pMicroPerCycle == rhs.pMicroPerCycle
        && lhs.pOffsetCorrectionOut == rhs.pOffsetCorrectionOut
        && lhs.pOffsetCorrectionStart == rhs.pOffsetCorrectionStart
        && lhs.pRateCorrectionOut == rhs.pRateCorrectionOut
        && lhs.pWakeupChannel == rhs.pWakeupChannel
        && lhs.pWakeupPattern == rhs.pWakeupPattern
        && lhs.pdMicrotick == rhs.pdMicrotick
        && lhs.pSamplesPerMicrotick == rhs.pSamplesPerMicrotick;
}

bool operator==(const TxBufferConfig& lhs, const TxBufferConfig& rhs)
{
    return lhs.channels == rhs.channels
        && lhs.slotId == rhs.slotId
        && lhs.offset == rhs.offset
        && lhs.repetition == rhs.repetition
        && lhs.hasPayloadPreambleIndicator == rhs.hasPayloadPreambleIndicator
        && lhs.headerCrc == rhs.headerCrc
        && lhs.transmissionMode == rhs.transmissionMode;
}

bool operator==(const ControllerConfig& lhs, const ControllerConfig& rhs)
{
    return lhs.clusterParams == rhs.clusterParams
        && lhs.nodeParams == rhs.nodeParams
        && lhs.bufferConfigs == rhs.bufferConfigs;
}

bool operator==(const HostCommand& lhs, const HostCommand& rhs)
{
    return lhs.command == rhs.command;
}

bool operator==(const ControllerStatus& lhs, const ControllerStatus& rhs)
{
    return lhs.pocState == rhs.pocState;
}

std::ostream& operator<<(std::ostream& out, Channel channel)
{
    switch (channel)
    {
    case Channel::A:
      return out << "A";
    case Channel::B:
      return out << "B";
    case Channel::AB:
      return out << "AB";
    case Channel::None:
      return out << "None";
    default:
      return out << "Channel=" << static_cast<uint32_t>(channel);
    }
}

std::ostream& operator<<(std::ostream& out, SymbolPattern pattern)
{
    switch (pattern)
    {
    case SymbolPattern::CasMts:
        return out << "CasMts";
    case SymbolPattern::Wus:
        return out << "Wus";
    case SymbolPattern::Wudop:
        return out << "Wudp";
    default:
        return out << "SymbolPattern=" << static_cast<uint32_t>(pattern);
    }
}

std::ostream& operator<<(std::ostream& out, const FrSymbol& symbol)
{
    return out << "FrSymbol{"
             << ", t=" << symbol.timestamp.count() << "ns"
             << ", Ch=" << symbol.channel
             << ", " << symbol.pattern
             << "}";
}

}
}
}
