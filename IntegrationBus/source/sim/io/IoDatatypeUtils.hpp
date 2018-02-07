// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/sim/io/IoDatatypes.hpp"

namespace ib {
namespace sim {
namespace io { 

bool operator==(const AnalogIoMessage& lhs, const AnalogIoMessage& rhs);
bool operator==(const DigitalIoMessage& lhs, const DigitalIoMessage& rhs);
bool operator==(const PatternIoMessage& lhs, const PatternIoMessage& rhs);
bool operator==(const PwmValue& lhs, const PwmValue& rhs);
bool operator==(const PwmIoMessage& lhs, const PwmIoMessage& rhs);

} // namespace io
} // namespace sim
} // namespace ib
