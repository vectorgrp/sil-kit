// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <cstdint>

namespace ib {
namespace sim {

using LinkId = int16_t;

/*! \brief TODO description
 */
enum class TransmitDirection : uint8_t
{
    /*!
    */
    TX = 1,

    /*!
    */
    RX = 2,
};
using DirectionMask = uint8_t;

} // namespace sim
} // namespace ib
