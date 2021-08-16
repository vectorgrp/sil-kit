// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "EthDatatypeUtils.hpp"

#include <cassert>

namespace ib {
namespace sim {
namespace eth {

bool operator==(const EthTagControlInformation& lhs, const EthTagControlInformation& rhs)
{
    return lhs.pcp == rhs.pcp
        && lhs.dei == rhs.dei
        && lhs.vid == rhs.vid;
}

bool operator==(const EthMessage& lhs, const EthMessage& rhs)
{
    return lhs.transmitId == rhs.transmitId
        && lhs.timestamp == rhs.timestamp
        && lhs.ethFrame.RawFrame() == rhs.ethFrame.RawFrame();
}

bool operator==(const EthFrame& lhs, const EthFrame& rhs)
{
    return lhs.RawFrame() == rhs.RawFrame();
}

bool operator==(const EthTransmitAcknowledge& lhs, const EthTransmitAcknowledge& rhs)
{
    return lhs.transmitId == rhs.transmitId
        && lhs.timestamp == rhs.timestamp
        && lhs.status == rhs.status;
}

bool operator==(const EthSetMode& lhs, const EthSetMode& rhs)
{
    return lhs.mode == rhs.mode;
}

} // namespace eth
} // namespace sim
} // namespace ib
