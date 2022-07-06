// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "EthernetSerdes.hpp"


namespace SilKit {
namespace Services {
namespace Ethernet {

SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const EthernetFrame& msg)
{
    buffer << msg.raw;

    return buffer;
}
SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, EthernetFrame& msg)
{
    buffer >> msg.raw;

    return buffer;
}

SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const EthernetFrameEvent& msg)
{
    buffer << msg.transmitId
           << msg.timestamp
           << msg.frame;

    return buffer;
}

SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, EthernetFrameEvent& msg)
{
    buffer >> msg.transmitId
           >> msg.timestamp
           >> msg.frame;

    return buffer;
}

SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const EthernetFrameTransmitEvent& ack)
{
    buffer << ack.transmitId
           << ack.sourceMac
           << ack.timestamp
           << ack.status;
    return buffer;
}

SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, EthernetFrameTransmitEvent& ack)
{
    buffer >> ack.transmitId
           >> ack.sourceMac
           >> ack.timestamp
           >> ack.status;
    return buffer;
}

SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const EthernetStatus& msg)
{
    buffer << msg.timestamp
           << msg.state
           << msg.bitrate;
    return buffer;
}

SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, EthernetStatus& msg)
{
    buffer >> msg.timestamp
           >> msg.state
           >> msg.bitrate;
    return buffer;
}

SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const EthernetSetMode& msg)
{
    buffer << msg.mode;
    return buffer;
}

SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, EthernetSetMode& msg)
{
    buffer >> msg.mode;
    return buffer;
}

using SilKit::Core::MessageBuffer;

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

} // namespace Ethernet    
} // namespace Services
} // namespace SilKit
