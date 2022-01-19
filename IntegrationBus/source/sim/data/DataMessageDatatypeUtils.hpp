// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <ostream>

#include "ib/sim/data/DataMessageDatatypes.hpp"

namespace ib {
namespace sim {
namespace data {

bool operator==(const DataMessage& lhs, const DataMessage& rhs);

bool               operator==(const DataExchangeFormat& lhs, const DataExchangeFormat& rhs);
bool               Match(const DataExchangeFormat& lhs, const DataExchangeFormat& rhs);
DataExchangeFormat Join(const DataExchangeFormat& defA, const DataExchangeFormat& defB);

} // namespace data
} // namespace sim
} // namespace ib
