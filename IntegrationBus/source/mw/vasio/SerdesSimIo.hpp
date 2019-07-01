// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "MessageBuffer.hpp"

#include "ib/sim/io/IoDatatypes.hpp"

namespace ib {
namespace sim {
namespace io {

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const AnalogIoMessage& msg)
{
    buffer << msg.timestamp
           << msg.value;

    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, AnalogIoMessage& msg)
{
    buffer >> msg.timestamp
           >> msg.value;

    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const DigitalIoMessage& msg)
{
    buffer << msg.timestamp
           << msg.value;

    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, DigitalIoMessage& msg)
{
    buffer >> msg.timestamp
           >> msg.value;

    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const PatternIoMessage& msg)
{
    buffer << msg.timestamp
           << msg.value;

    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, PatternIoMessage& msg)
{
    buffer >> msg.timestamp
           >> msg.value;

    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const PwmIoMessage& msg)
{
    buffer << msg.timestamp
           << msg.value.frequency
           << msg.value.dutyCycle;

    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, PwmIoMessage& msg)
{
    buffer >> msg.timestamp
           >> msg.value.frequency
           >> msg.value.dutyCycle;

    return buffer;
}

} // namespace io    
} // namespace sim
} // namespace ib
