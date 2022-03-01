// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <cstdint>

namespace ib {
namespace sim {

/*! \brief TODO description
 */
enum class TransmitDirection : uint8_t
{
    // Undefined
    Undefined = 0,
    // Transmit
    TX = 1,
    // Receive
    RX = 2,
};
using DirectionMask = uint8_t;

} // namespace sim
} // namespace ib
