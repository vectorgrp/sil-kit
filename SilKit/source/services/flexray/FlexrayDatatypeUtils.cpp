// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "FlexrayDatatypeUtils.hpp"

namespace SilKit {
namespace Services {
namespace Flexray {

bool operator==(const FlexrayHeader& lhs, const FlexrayHeader& rhs)
{
    return lhs.flags == rhs.flags && lhs.frameId == rhs.frameId && lhs.headerCrc == rhs.headerCrc
           && lhs.cycleCount == rhs.cycleCount && lhs.payloadLength == rhs.payloadLength;
}

bool operator==(const FlexrayFrame& lhs, const FlexrayFrame& rhs)
{
    return lhs.header == rhs.header && Util::ItemsAreEqual(lhs.payload, rhs.payload);
}

bool operator==(const FlexrayFrameEvent& lhs, const FlexrayFrameEvent& rhs)
{
    return lhs.channel == rhs.channel && lhs.frame == rhs.frame;
}

bool operator==(const FlexrayFrameTransmitEvent& lhs, const FlexrayFrameTransmitEvent& rhs)
{
    return lhs.txBufferIndex == rhs.txBufferIndex && lhs.channel == rhs.channel && lhs.frame == rhs.frame;
}

bool operator==(const FlexraySymbolEvent& lhs, const FlexraySymbolEvent& rhs)
{
    return lhs.channel == rhs.channel && lhs.pattern == rhs.pattern;
}

bool operator==(const FlexrayWakeupEvent& lhs, const FlexrayWakeupEvent& rhs)
{
    return lhs.channel == rhs.channel && lhs.pattern == rhs.pattern;
}

bool operator==(const FlexrayTxBufferConfigUpdate& lhs, const FlexrayTxBufferConfigUpdate& rhs)
{
    return lhs.txBufferIndex == rhs.txBufferIndex && lhs.txBufferConfig == rhs.txBufferConfig;
}

bool operator==(const WireFlexrayTxBufferUpdate& lhs, const WireFlexrayTxBufferUpdate& rhs)
{
    return lhs.txBufferIndex == rhs.txBufferIndex && lhs.payloadDataValid == rhs.payloadDataValid
           && Util::ItemsAreEqual(lhs.payload, rhs.payload);
}

bool operator==(const FlexrayControllerConfig& lhs, const FlexrayControllerConfig& rhs)
{
    return lhs.clusterParams == rhs.clusterParams && lhs.nodeParams == rhs.nodeParams
           && lhs.bufferConfigs == rhs.bufferConfigs;
}

bool operator==(const FlexrayHostCommand& lhs, const FlexrayHostCommand& rhs)
{
    return lhs.command == rhs.command;
}

bool operator==(const FlexrayPocStatusEvent& lhs, const FlexrayPocStatusEvent& rhs)
{
    return lhs.state == rhs.state && lhs.chiHaltRequest == rhs.chiHaltRequest
           && lhs.chiReadyRequest == rhs.chiReadyRequest && lhs.slotMode == rhs.slotMode
           && lhs.errorMode == rhs.errorMode && lhs.wakeupStatus == rhs.wakeupStatus
           && lhs.startupState == rhs.startupState && lhs.freeze == rhs.freeze
           && lhs.coldstartNoise == rhs.coldstartNoise;
}

bool operator==(const FlexrayCycleStartEvent& lhs, const FlexrayCycleStartEvent& rhs)
{
    return lhs.cycleCounter == rhs.cycleCounter && lhs.timestamp == rhs.timestamp;
}

} // namespace Flexray
} // namespace Services
} // namespace SilKit
