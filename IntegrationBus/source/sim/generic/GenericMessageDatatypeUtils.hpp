// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <ostream>

#include "ib/sim/generic/GenericMessageDatatypes.hpp"

namespace ib {
namespace sim {
namespace generic {

bool operator==(const GenericMessage& lhs, const GenericMessage& rhs);

} // namespace generic
} // namespace sim
} // namespace ib
