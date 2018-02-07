// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include <chrono>
#include <vector>

#include "ib/sim/datatypes.hpp"

// ================================================================================
//  I/O specific data types
// ================================================================================
namespace ib {
namespace sim {
namespace io {

struct PwmValue
{
    double frequency;
    double dutyCycle;
};

/*!
 * \brief An I/O message
 * 
 * \tparam ValueT The I/O value type, depending on the kind of I/O
 */
template <typename ValueT>
struct IoMessage
{
    using ValueType = ValueT;

    std::chrono::nanoseconds timestamp;
    ValueType value;
};

using AnalogIoMessage = IoMessage<double>;
using DigitalIoMessage = IoMessage<bool>;
using PatternIoMessage = IoMessage<std::vector<uint8_t>>;
using PwmIoMessage = IoMessage<PwmValue>;

} // namespace io
} // namespace sim
} // namespace ib
