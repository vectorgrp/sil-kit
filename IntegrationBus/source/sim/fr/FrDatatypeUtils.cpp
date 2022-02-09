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

bool operator==(const TxBufferConfigUpdate& lhs, const TxBufferConfigUpdate& rhs)
{
    return lhs.txBufferIndex == rhs.txBufferIndex
        && lhs.txBufferConfig == rhs.txBufferConfig;
}

bool operator==(const TxBufferUpdate& lhs, const TxBufferUpdate& rhs)
{
    return lhs.txBufferIndex == rhs.txBufferIndex
        && lhs.payloadDataValid == rhs.payloadDataValid
        && lhs.payload == rhs.payload;
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

bool operator==(const PocStatus& lhs, const PocStatus& rhs)
{
    return lhs.state == rhs.state
        && lhs.chiHaltRequest == rhs.chiHaltRequest
        && lhs.chiReadyRequest == rhs.chiReadyRequest
        && lhs.slotMode == rhs.slotMode
        && lhs.errorMode == rhs.errorMode
        && lhs.wakeupStatus == rhs.wakeupStatus
        && lhs.startupState == rhs.startupState
        && lhs.freeze == rhs.freeze
        && lhs.coldstartNoise == rhs.coldstartNoise
    ;
}

bool operator==(const CycleStart& lhs, const CycleStart& rhs)
{
    return lhs.cycleCounter == rhs.cycleCounter
        && lhs.timestamp == rhs.timestamp;
}

}
}
}
