// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "MessageBuffer.hpp"
#include "ib/sim/can/CanDatatypes.hpp"
#include "CanSerdes.hpp"

namespace ib {
namespace sim {
namespace can {

using namespace ib::sim::can;

ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const CanFrameEvent& msg)
{
    buffer << msg.transmitId
        << msg.timestamp
        << msg.frame.canId
        << *reinterpret_cast<const uint8_t*>(&msg.frame.flags)
        << msg.frame.dlc
        << msg.frame.dataField
        << msg.direction
        << msg.userContext
        ;
    return buffer;
}
ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, CanFrameEvent& msg)
{
    uint8_t flags;
    uint8_t dlc;
    buffer >> msg.transmitId
        >> msg.timestamp
        >> msg.frame.canId
        >> flags
        >> dlc
        >> msg.frame.dataField
        >> msg.direction
        >> msg.userContext
        ;
    *reinterpret_cast<uint8_t*>(&msg.frame.flags) = flags;
    msg.frame.dlc = dlc;
    return buffer;
}

ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const CanFrameTransmitEvent& ack)
{
    buffer << ack.transmitId
           << ack.canId
           << ack.timestamp
           << ack.status
           << ack.userContext;
    return buffer;
}
ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, CanFrameTransmitEvent& ack)
{
    buffer >> ack.transmitId
           >> ack.canId
           >> ack.timestamp
           >> ack.status
           >> ack.userContext;
    return buffer;
}

ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const CanControllerStatus& msg)
{
    buffer << msg.timestamp
           << msg.controllerState
           << msg.errorState;
    return buffer;
}
ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, CanControllerStatus& msg)
{
    buffer >> msg.timestamp
           >> msg.controllerState
           >> msg.errorState;
    return buffer;
}

ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const CanConfigureBaudrate& msg)
{
    buffer << msg.baudRate
           << msg.fdBaudRate;
    return buffer;
}
ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, CanConfigureBaudrate& msg)
{
    buffer >> msg.baudRate
           >> msg.fdBaudRate;
    return buffer;
}

ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const CanSetControllerMode& msg)
{
    buffer << *reinterpret_cast<const uint8_t*>(&msg.flags)
           << msg.mode;
    return buffer;
}
ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, CanSetControllerMode& msg)
{
    uint8_t flags;
    buffer >> flags
           >> msg.mode;
    *reinterpret_cast<uint8_t*>(&msg.flags) = flags;
    return buffer;
}

using namespace ib::mw;
// when changing any of the datatypes, add transparent compatibility code here,
// based on the buffer.GetFormatVersion()

//////////////////////////////////////////////////////////////////////
// Serialize
//////////////////////////////////////////////////////////////////////
void Serialize(MessageBuffer& buffer, const sim::can::CanFrameEvent& msg)
{
    buffer << msg;
    return;
}
void Serialize(MessageBuffer& buffer, const sim::can::CanFrameTransmitEvent& msg)
{
    buffer << msg;
    return;
}
void Serialize(MessageBuffer& buffer, const sim::can::CanControllerStatus& msg)
{
    buffer << msg;
    return;
}
void Serialize(MessageBuffer& buffer, const sim::can::CanConfigureBaudrate& msg)
{
    buffer << msg;
    return;
}
void Serialize(MessageBuffer& buffer, const sim::can::CanSetControllerMode& msg)
{
    buffer << msg;
    return;
}
//////////////////////////////////////////////////////////////////////
// Deserialize
//////////////////////////////////////////////////////////////////////
void Deserialize(MessageBuffer& buffer, sim::can::CanFrameEvent& out)
{
   buffer >> out;
}
void Deserialize(MessageBuffer& buffer, sim::can::CanFrameTransmitEvent& out)
{
   buffer >> out;
}
void Deserialize(MessageBuffer& buffer, sim::can::CanControllerStatus& out)
{
   buffer >> out;
}
void Deserialize(MessageBuffer& buffer, sim::can::CanConfigureBaudrate& out)
{
   buffer >> out;
}
void Deserialize(MessageBuffer& buffer, sim::can::CanSetControllerMode& out)
{
   buffer >> out;
}

} // namespace can    
} // namespace sim
} // namespace ib
