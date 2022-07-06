// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <cstdint>

namespace SilKit {
namespace Services {

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

} // namespace Services
} // namespace SilKit
