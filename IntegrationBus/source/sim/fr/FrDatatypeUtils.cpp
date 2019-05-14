// Copyright (c) Vector Informatik GmbH. All rights reserved.

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

bool operator==(const CycleStart& lhs, const CycleStart& rhs)
{
    return lhs.cycleCounter == rhs.cycleCounter
        && lhs.timestamp == rhs.timestamp;
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
