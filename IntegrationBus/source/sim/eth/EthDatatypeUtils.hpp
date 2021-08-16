// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <iostream>

#include "ib/sim/eth/EthDatatypes.hpp"

namespace ib {
namespace sim {
namespace eth {

bool operator==(const EthTagControlInformation& lhs, const EthTagControlInformation& rhs);
bool operator==(const EthMessage& lhs, const EthMessage& rhs);
bool operator==(const EthFrame& lhs, const EthFrame& rhs);
bool operator==(const EthTransmitAcknowledge& lhs, const EthTransmitAcknowledge& rhs);
bool operator==(const EthSetMode& lhs, const EthSetMode& rhs);

} // namespace ib
} // namespace sim
} // namespace eth
