// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/sim/lin/LinDatatypes.hpp"

#include <ostream>

namespace ib {
namespace sim {
namespace lin { 

bool operator==(const Payload& lhs, const Payload& rhs);
bool operator==(const LinMessage& lhs, const LinMessage& rhs);
bool operator==(const RxRequest& lhs, const RxRequest& rhs);
bool operator==(const TxAcknowledge& lhs, const TxAcknowledge& rhs);
bool operator==(const ControllerConfig& lhs, const ControllerConfig& rhs);
bool operator==(const SlaveResponseConfig& lhs, const SlaveResponseConfig& rhs);
bool operator==(const SlaveResponse& lhs, const SlaveResponse& rhs);

std::ostream& operator<<(std::ostream& out, MessageStatus status);
std::ostream& operator<<(std::ostream& out, ChecksumModel model);
std::ostream& operator<<(std::ostream& out, const LinMessage& msg);

} // namespace lin
} // namespace sim
} // namespace ib
