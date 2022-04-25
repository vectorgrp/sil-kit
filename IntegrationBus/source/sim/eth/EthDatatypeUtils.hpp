// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <iostream>

#include "ib/sim/eth/EthernetDatatypes.hpp"

namespace ib {
namespace sim {
namespace eth {

bool operator==(const EthernetTagControlInformation& lhs, const EthernetTagControlInformation& rhs);
bool operator==(const EthernetFrameEvent& lhs, const EthernetFrameEvent& rhs);
bool operator==(const EthernetFrame& lhs, const EthernetFrame& rhs);
bool operator==(const EthernetFrameTransmitEvent& lhs, const EthernetFrameTransmitEvent& rhs);
bool operator==(const EthernetSetMode& lhs, const EthernetSetMode& rhs);
bool operator==(const EthernetStateChangeEvent& lhs, const EthernetStateChangeEvent& rhs);
bool operator==(const EthernetBitrateChangeEvent& lhs, const EthernetBitrateChangeEvent& rhs);
bool operator!=(const EthernetBitrateChangeEvent& lhs, const EthernetBitrateChangeEvent& rhs); // For NeMatcher in EthControllerProxyTest

} // namespace ib
} // namespace sim
} // namespace eth
