/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include "EthernetSerdes.hpp"

namespace SilKit {
namespace Services {
namespace Ethernet {

inline SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const WireEthernetFrame& msg)
{
    buffer << msg.raw;

    return buffer;
}

inline SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, WireEthernetFrame& msg)
{
    buffer >> msg.raw;

    return buffer;
}

inline SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const WireEthernetFrameEvent& msg)
{
    buffer
        << msg.timestamp
        << msg.frame
        << msg.direction
        << msg.userContext
        ;

    return buffer;
}

inline SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, WireEthernetFrameEvent& msg)
{
    buffer
        >> msg.timestamp
        >> msg.frame
        >> msg.direction
        >> msg.userContext
        ;

    return buffer;
}

inline SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const EthernetFrameTransmitEvent& ack)
{
    buffer
        << ack.timestamp
        << ack.status
        << ack.userContext
        ;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, EthernetFrameTransmitEvent& ack)
{
    buffer
        >> ack.timestamp
        >> ack.status
        >> ack.userContext
        ;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const EthernetStatus& msg)
{
    buffer << msg.timestamp
           << msg.state
           << msg.bitrate;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, EthernetStatus& msg)
{
    buffer >> msg.timestamp
           >> msg.state
           >> msg.bitrate;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const EthernetSetMode& msg)
{
    buffer << msg.mode;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, EthernetSetMode& msg)
{
    buffer >> msg.mode;
    return buffer;
}

using SilKit::Core::MessageBuffer;

void Serialize(MessageBuffer& buffer, const WireEthernetFrameEvent& msg)
{
    buffer << msg;
}

void Serialize(MessageBuffer& buffer, const EthernetFrameTransmitEvent& msg)
{
    buffer << msg;
}

void Serialize(MessageBuffer& buffer, const EthernetStatus& msg)
{
    buffer << msg;
}

void Serialize(MessageBuffer& buffer, const EthernetSetMode& msg)
{
    buffer << msg;
}

void Deserialize(MessageBuffer& buffer, WireEthernetFrameEvent& out)
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
