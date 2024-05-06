// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/experimental/netsim/NetworkSimulatorDatatypes.hpp"
#include "Configuration.hpp"

namespace SilKit {
namespace Experimental {
namespace NetworkSimulation {

inline auto ConvertNetworkTypeFromConfig(Config::v1::NetworkType configNetworkType) -> Experimental::NetworkSimulation::SimulatedNetworkType
{
    switch (configNetworkType)
    {
    case Config::v1::NetworkType::CAN:
        return SimulatedNetworkType::CAN;
    case Config::v1::NetworkType::LIN:
        return SimulatedNetworkType::LIN;
    case Config::v1::NetworkType::Ethernet:
        return SimulatedNetworkType::Ethernet;
    case Config::v1::NetworkType::FlexRay:
        return SimulatedNetworkType::FlexRay;
    default:
        return SimulatedNetworkType::Undefined;
    }
}

inline auto ConvertNetworkTypeToConfig(NetworkSimulation::SimulatedNetworkType networkType)
    -> Config::v1::NetworkType
{
    switch (networkType)
    {
    case SimulatedNetworkType::CAN:
        return Config::v1::NetworkType::CAN;
    case SimulatedNetworkType::LIN:
        return Config::v1::NetworkType::LIN;
    case SimulatedNetworkType::Ethernet:
        return Config::v1::NetworkType::Ethernet;
    case SimulatedNetworkType::FlexRay:
        return Config::v1::NetworkType::FlexRay;
    default:
        return Config::v1::NetworkType::Undefined;
    }
}

} // namespace NetworkSimulation
} // namespace Experimental
} // namespace SilKit
