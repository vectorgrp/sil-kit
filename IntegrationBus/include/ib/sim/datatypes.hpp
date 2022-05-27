// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <cstdint>

namespace ib {
namespace sim {

/*! \brief Flag indicating the direction of a message
 */
enum class TransmitDirection : uint8_t
{
    // Undefined
    Undefined = 0,
    // Transmit
    TX = 1,
    // Receive
    RX = 2,
    // Send/Receive
    TXRX = 3,
};
using DirectionMask = uint8_t;

using HandlerId = uint64_t;

} // namespace sim
} // namespace ib
