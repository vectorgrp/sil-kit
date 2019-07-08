// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <chrono>
#include <vector>

#include "ib/sim/datatypes.hpp"

// ================================================================================
//  I/O specific data types
// ================================================================================
namespace ib {
namespace sim {
//! The IO namespace
namespace io {

//! \brief A pwm (pulse-width modulation) value
struct PwmValue
{
    double frequency; //!< The frequency of the pwm value.
    double dutyCycle; //!< The duty cycle of the pwm value.
};

/*!
 * \brief An I/O message
 * 
 * \tparam ValueT The I/O value type, depending on the kind of I/O.
 */
template <typename ValueT>
struct IoMessage
{
    using ValueType = ValueT;

    std::chrono::nanoseconds timestamp; //!< End of message time stamp.
    ValueType value; //!< I/O message value, depending on the message type.
};

//! \brief Analog I/O value type, an IoMessage with a double value.
using AnalogIoMessage = IoMessage<double>;
//! \brief Digital I/O value type, an IoMessage with a bool.
using DigitalIoMessage = IoMessage<bool>;
//! \brief Pattern I/O value type, an IoMessage with a vector of uint8_t.
using PatternIoMessage = IoMessage<std::vector<uint8_t>>;
//! \brief Pwm I/O value type, an IoMessage with a PwmValue.
using PwmIoMessage = IoMessage<PwmValue>;

} // namespace io
} // namespace sim
} // namespace ib
