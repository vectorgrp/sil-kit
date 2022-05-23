// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "MessageBuffer.hpp"

#include "ib/sim/eth/EthernetDatatypes.hpp"

namespace ib {
namespace sim {
namespace eth {

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const EthernetFrame& msg)
{
    buffer << msg.raw;

    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, EthernetFrame& msg)
{
    buffer >> msg.raw;

    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const EthernetFrameEvent& msg)
{
    buffer << msg.transmitId
           << msg.timestamp
           << msg.frame;

    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, EthernetFrameEvent& msg)
{
    buffer >> msg.transmitId
           >> msg.timestamp
           >> msg.frame;

    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const EthernetFrameTransmitEvent& ack)
{
    buffer << ack.transmitId
           << ack.sourceMac
           << ack.timestamp
           << ack.status;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, EthernetFrameTransmitEvent& ack)
{
    buffer >> ack.transmitId
           >> ack.sourceMac
           >> ack.timestamp
           >> ack.status;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const EthernetStatus& msg)
{
    buffer << msg.timestamp
           << msg.state
           << msg.bitrate;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, EthernetStatus& msg)
{
    buffer >> msg.timestamp
           >> msg.state
           >> msg.bitrate;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const EthernetSetMode& msg)
{
    buffer << msg.mode;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, EthernetSetMode& msg)
{
    buffer >> msg.mode;
    return buffer;
}


} // namespace eth    
} // namespace sim
} // namespace ib
