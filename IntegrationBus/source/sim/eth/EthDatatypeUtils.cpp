// Copyright (c)  Vector Informatik GmbH. All rights reserved.

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


std::ostream& operator<<(std::ostream& out, EthMode mode)
{
    switch (mode)
    {
    case EthMode::Active:
        out << "Active";
        break;
    case EthMode::Inactive:
        out << "Inactive";
        break;
    default:
        assert(false);
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, const EthSetMode& mode)
{
    out << "EthSetMode{" << mode.mode
        << "}";
    return out;
}

} // namespace eth
} // namespace sim
} // namespace ib
