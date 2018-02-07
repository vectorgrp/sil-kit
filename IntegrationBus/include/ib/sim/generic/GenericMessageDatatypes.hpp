// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include <cstdint>
#include <vector>

namespace ib {
namespace sim {
namespace generic {

/*! \brief A generic message
 *
 * Generic messages run over an abstract channel, without timing effects and/or data type constraints
 */
struct GenericMessage {
    std::vector<uint8_t> data;
};

} // namespace generic
} // namespace sim
} // namespace ib
