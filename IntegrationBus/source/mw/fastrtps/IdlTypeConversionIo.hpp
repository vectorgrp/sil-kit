// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/sim/io/IoDatatypes.hpp"

#include "idl/IoTopics.h"
#include "idl/IoTopicsPubSubTypes.h"


namespace ib {
namespace sim {
namespace io {

inline auto to_idl(const AnalogIoMessage& msg) -> idl::AnalogIoMessage;
inline auto to_idl(const DigitalIoMessage& msg) -> idl::DigitalIoMessage;
inline auto to_idl(const PatternIoMessage& msg) -> idl::PatternIoMessage;
inline auto to_idl(PatternIoMessage&& msg) -> idl::PatternIoMessage;
inline auto to_idl(const PwmValue& msg) -> idl::PwmValue;
inline auto to_idl(const PwmIoMessage& msg) -> idl::PwmIoMessage;

namespace idl {
inline auto from_idl(AnalogIoMessage&& idl) -> io::AnalogIoMessage;
inline auto from_idl(DigitalIoMessage&& idl) -> io::DigitalIoMessage;
inline auto from_idl(PatternIoMessage&& idl) -> io::PatternIoMessage;
inline auto from_idl(PwmValue&& idl) -> io::PwmValue;
inline auto from_idl(PwmIoMessage&& idl) -> io::PwmIoMessage;
} // namespace idl

// ================================================================================
//  Inline Implementations
// ================================================================================
auto to_idl(const AnalogIoMessage& msg) -> idl::AnalogIoMessage
{
    idl::AnalogIoMessage idl;

    idl.timestampNs(msg.timestamp.count());
    idl.value(msg.value);

    return idl;
}

auto idl::from_idl(AnalogIoMessage&& idl) -> io::AnalogIoMessage
{
    io::AnalogIoMessage msg;

    msg.timestamp = std::chrono::nanoseconds{idl.timestampNs()};
    msg.value = idl.value();

    return msg;
}

auto to_idl(const DigitalIoMessage& msg) -> idl::DigitalIoMessage
{
    idl::DigitalIoMessage idl;

    idl.timestampNs(msg.timestamp.count());
    idl.value(msg.value);

    return idl;
}

auto idl::from_idl(DigitalIoMessage&& idl) -> io::DigitalIoMessage
{
    io::DigitalIoMessage msg;

    msg.timestamp = std::chrono::nanoseconds{idl.timestampNs()};
    msg.value = idl.value();

    return msg;
}
    
auto to_idl(const PatternIoMessage& msg) -> idl::PatternIoMessage
{
    idl::PatternIoMessage idl;

    idl.timestampNs(msg.timestamp.count());
    idl.value(msg.value);

    return idl;
}

auto to_idl(PatternIoMessage&& msg) -> idl::PatternIoMessage
{
    idl::PatternIoMessage idl;

    idl.timestampNs(msg.timestamp.count());
    idl.value(std::move(msg.value));

    return idl;
}

auto idl::from_idl(PatternIoMessage&& idl) -> io::PatternIoMessage
{
    io::PatternIoMessage msg;

    msg.timestamp = std::chrono::nanoseconds{idl.timestampNs()};
    msg.value = std::move(idl.value());

    return msg;
}

auto to_idl(const PwmValue& msg) -> idl::PwmValue
{
    idl::PwmValue idl;
    idl.frequency(msg.frequency);
    idl.dutyCycle(msg.dutyCycle);
    return idl;
}

auto idl::from_idl(PwmValue&& idl) -> io::PwmValue
{
    io::PwmValue msg;
    msg.frequency = idl.frequency();
    msg.dutyCycle = idl.dutyCycle();
    return msg;
}

auto to_idl(const PwmIoMessage& msg) -> idl::PwmIoMessage
{
    idl::PwmIoMessage idl;

    idl.timestampNs(msg.timestamp.count());
    idl.value(to_idl(msg.value));

    return idl;
}

auto idl::from_idl(PwmIoMessage&& idl) -> io::PwmIoMessage
{
    io::PwmIoMessage msg;

    msg.timestamp = std::chrono::nanoseconds{idl.timestampNs()};
    msg.value = from_idl(std::move(idl.value()));

    return msg;
}
    

} // namespace io
} // namespace sim
} // namespace ib
