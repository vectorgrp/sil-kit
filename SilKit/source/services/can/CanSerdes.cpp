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

#include "silkit/services/can/CanDatatypes.hpp"

#include "MessageBuffer.hpp"
#include "CanSerdes.hpp"

namespace SilKit {
namespace Services {
namespace Can {

SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const WireCanFrameEvent& msg)
{
    buffer
        << msg.timestamp
        << msg.frame.canId
        << msg.frame.flags
        << msg.frame.dlc
        << msg.frame.sdt
        << msg.frame.vcid
        << msg.frame.af
        << msg.frame.dataField
        << msg.direction
        << msg.userContext
        ;
    return buffer;
}

SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, WireCanFrameEvent& msg)
{
    buffer
        >> msg.timestamp
        >> msg.frame.canId
        >> msg.frame.flags
        >> msg.frame.dlc
        >> msg.frame.sdt
        >> msg.frame.vcid
        >> msg.frame.af
        >> msg.frame.dataField
        >> msg.direction
        >> msg.userContext
        ;
    return buffer;
}

SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const CanFrameTransmitEvent& ack)
{
    buffer << ack.canId
           << ack.timestamp
           << ack.status
           << ack.userContext;
    return buffer;
}

SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, CanFrameTransmitEvent& ack)
{
    buffer >> ack.canId
           >> ack.timestamp
           >> ack.status
           >> ack.userContext;
    return buffer;
}

SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const CanControllerStatus& msg)
{
    buffer << msg.timestamp
           << msg.controllerState
           << msg.errorState;
    return buffer;
}

SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, CanControllerStatus& msg)
{
    buffer >> msg.timestamp
           >> msg.controllerState
           >> msg.errorState;
    return buffer;
}

SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const CanConfigureBaudrate& msg)
{
    buffer << msg.baudRate
           << msg.fdBaudRate
           << msg.xlBaudRate;
    return buffer;
}

SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, CanConfigureBaudrate& msg)
{
    buffer >> msg.baudRate
           >> msg.fdBaudRate
           >> msg.xlBaudRate;
    return buffer;
}

SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const CanSetControllerMode& msg)
{
    buffer << *reinterpret_cast<const uint8_t*>(&msg.flags)
           << msg.mode;
    return buffer;
}

SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, CanSetControllerMode& msg)
{
    uint8_t flags;
    buffer >> flags
           >> msg.mode;
    *reinterpret_cast<uint8_t*>(&msg.flags) = flags;
    return buffer;
}

using namespace SilKit::Core;
// when changing any of the datatypes, add transparent compatibility code here,
// based on the buffer.GetProtocolVersion()

//////////////////////////////////////////////////////////////////////
// Serialize
//////////////////////////////////////////////////////////////////////

void Serialize(MessageBuffer& buffer, const Services::Can::WireCanFrameEvent& msg)
{
    buffer << msg;
}

void Serialize(MessageBuffer& buffer, const Services::Can::CanFrameTransmitEvent& msg)
{
    buffer << msg;
}

void Serialize(MessageBuffer& buffer, const Services::Can::CanControllerStatus& msg)
{
    buffer << msg;
}

void Serialize(MessageBuffer& buffer, const Services::Can::CanConfigureBaudrate& msg)
{
    buffer << msg;
}

void Serialize(MessageBuffer& buffer, const Services::Can::CanSetControllerMode& msg)
{
    buffer << msg;
}

//////////////////////////////////////////////////////////////////////
// Deserialize
//////////////////////////////////////////////////////////////////////

void Deserialize(MessageBuffer& buffer, Services::Can::WireCanFrameEvent& out)
{
   buffer >> out;
}

void Deserialize(MessageBuffer& buffer, Services::Can::CanFrameTransmitEvent& out)
{
   buffer >> out;
}

void Deserialize(MessageBuffer& buffer, Services::Can::CanControllerStatus& out)
{
   buffer >> out;
}

void Deserialize(MessageBuffer& buffer, Services::Can::CanConfigureBaudrate& out)
{
   buffer >> out;
}

void Deserialize(MessageBuffer& buffer, Services::Can::CanSetControllerMode& out)
{
   buffer >> out;
}

} // namespace Can
} // namespace Services
} // namespace SilKit
