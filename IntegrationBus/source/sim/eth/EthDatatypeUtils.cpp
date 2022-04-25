// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "EthDatatypeUtils.hpp"

#include <cassert>

namespace ib {
namespace sim {
namespace eth {

bool operator==(const EthernetTagControlInformation& lhs, const EthernetTagControlInformation& rhs)
{
    return lhs.pcp == rhs.pcp
        && lhs.dei == rhs.dei
        && lhs.vid == rhs.vid;
}

bool operator==(const EthernetFrameEvent& lhs, const EthernetFrameEvent& rhs)
{
    return lhs.transmitId == rhs.transmitId
        && lhs.timestamp == rhs.timestamp
        && lhs.ethFrame.RawFrame() == rhs.ethFrame.RawFrame();
}

bool operator==(const EthernetFrame& lhs, const EthernetFrame& rhs)
{
    return lhs.RawFrame() == rhs.RawFrame();
}

bool operator==(const EthernetFrameTransmitEvent& lhs, const EthernetFrameTransmitEvent& rhs)
{
    return lhs.transmitId == rhs.transmitId
        && lhs.timestamp == rhs.timestamp
        && lhs.status == rhs.status;
}

bool operator==(const EthernetSetMode& lhs, const EthernetSetMode& rhs)
{
    return lhs.mode == rhs.mode;
}

bool operator==(const EthernetStateChangeEvent& lhs, const EthernetStateChangeEvent& rhs)
{
    return lhs.state == rhs.state && lhs.timestamp == rhs.timestamp;
}

bool operator==(const EthernetBitrateChangeEvent& lhs, const EthernetBitrateChangeEvent& rhs)
{
    return lhs.bitrate == rhs.bitrate && lhs.timestamp == rhs.timestamp;
}

bool operator!=(const EthernetBitrateChangeEvent& lhs, const EthernetBitrateChangeEvent& rhs)
{
    return lhs.bitrate != rhs.bitrate || lhs.timestamp != rhs.timestamp;
}

} // namespace eth
} // namespace sim
} // namespace ib
