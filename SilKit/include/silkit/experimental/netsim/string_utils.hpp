// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <ostream>
#include <sstream>

#include "NetworkSimulatorDatatypes.hpp"

namespace SilKit {
namespace Experimental {
namespace NetworkSimulation {

inline std::string to_string(SimulatedNetworkType type);
inline std::ostream& operator<<(std::ostream& out, SimulatedNetworkType ftypelag);

// ================================================================================
//  Inline Implementations
// ================================================================================

std::string to_string(SimulatedNetworkType type)
{
    std::stringstream outStream;
    outStream << type;
    return outStream.str();
}

std::ostream& operator<<(std::ostream& out, SimulatedNetworkType type)
{
    switch (type)
    {
    case SimulatedNetworkType::Undefined:
        return out << "Undefined";
    case SimulatedNetworkType::CAN:
        return out << "CAN";
    case SimulatedNetworkType::LIN:
        return out << "LIN";
    case SimulatedNetworkType::FlexRay:
        return out << "FlexRay";
    case SimulatedNetworkType::Ethernet:
        return out << "Ethernet";
    }

    return out << "unknown(" << static_cast<uint32_t>(type) << ")";
}

} // namespace NetworkSimulation
} // namespace Experimental
} // namespace SilKit

