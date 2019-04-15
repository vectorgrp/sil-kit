// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <ostream>

#include "ib/sim/can/CanDatatypes.hpp"

namespace ib {
namespace sim {
namespace can {

bool operator==(const CanMessage::CanReceiveFlags& lhs, const CanMessage::CanReceiveFlags& rhs);
bool operator==(const CanMessage& lhs, const CanMessage& rhs);
bool operator==(const CanTransmitAcknowledge& lhs, const CanTransmitAcknowledge& rhs);
bool operator==(const CanSetControllerMode& lhs, const CanSetControllerMode& rhs);
bool operator==(const CanConfigureBaudrate& lhs, const CanConfigureBaudrate& rhs);


}
}
}
