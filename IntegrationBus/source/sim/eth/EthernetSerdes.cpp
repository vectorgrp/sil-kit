// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "EthernetSerdes.hpp"


namespace ib {
namespace sim {
namespace eth {
using namespace ib::sim::eth;

ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const EthernetFrame& msg)
{
    buffer << msg.raw;

    return buffer;
}
ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, EthernetFrame& msg)
{
    buffer >> msg.raw;

    return buffer;
}

ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const EthernetFrameEvent& msg)
{
    buffer << msg.transmitId
           << msg.timestamp
           << msg.frame;

    return buffer;
}
ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, EthernetFrameEvent& msg)
{
    buffer >> msg.transmitId
           >> msg.timestamp
           >> msg.frame;

    return buffer;
}

ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const EthernetFrameTransmitEvent& ack)
{
    buffer << ack.transmitId
           << ack.sourceMac
           << ack.timestamp
           << ack.status;
    return buffer;
}
ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, EthernetFrameTransmitEvent& ack)
{
    buffer >> ack.transmitId
           >> ack.sourceMac
           >> ack.timestamp
           >> ack.status;
    return buffer;
}

ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const EthernetStatus& msg)
{
    buffer << msg.timestamp
           << msg.state
           << msg.bitrate;
    return buffer;
}
ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, EthernetStatus& msg)
{
    buffer >> msg.timestamp
           >> msg.state
           >> msg.bitrate;
    return buffer;
}

ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const EthernetSetMode& msg)
{
    buffer << msg.mode;
    return buffer;
}
ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, EthernetSetMode& msg)
{
    buffer >> msg.mode;
    return buffer;
}
using ib::mw::MessageBuffer;

void Serialize(MessageBuffer& buffer, const EthernetFrameEvent& msg)
{
    buffer << msg;
    return;
}
void Serialize(MessageBuffer& buffer, const EthernetFrameTransmitEvent& msg)
{
    buffer << msg;
    return;
}
void Serialize(MessageBuffer& buffer, const EthernetStatus& msg)
{
    buffer << msg;
    return;
}
void Serialize(MessageBuffer& buffer, const EthernetSetMode& msg)
{
    buffer << msg;
    return;
}

void Deserialize(MessageBuffer& buffer, EthernetFrameEvent& out)
{
    buffer >> out;
}
void Deserialize(MessageBuffer& buffer, EthernetFrameTransmitEvent& out)
{
    buffer >> out;
}
void Deserialize(MessageBuffer& buffer, EthernetStatus& out)
{
    buffer >> out;
}
void Deserialize(MessageBuffer& buffer, EthernetSetMode& out)
{
    buffer >> out;
}

} // namespace eth    
} // namespace sim
} // namespace ib
