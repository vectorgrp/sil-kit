// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "MessageBuffer.hpp"

#include "ib/sim/can/CanDatatypes.hpp"

namespace ib {
namespace sim {
namespace can {


inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const CanFrameEvent& msg)
{
    buffer << msg.transmitId
        << msg.timestamp
        << msg.frame.canId
        << *reinterpret_cast<const uint8_t*>(&msg.frame.flags)
        << msg.frame.dlc
        << msg.frame.dataField
        << msg.frame.direction
        << msg.frame.userContext
        ;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, CanFrameEvent& msg)
{
    uint8_t flags;
    uint8_t dlc;
    buffer >> msg.transmitId
        >> msg.timestamp
        >> msg.frame.canId
        >> flags
        >> dlc
        >> msg.frame.dataField
        >> msg.frame.direction
        >> msg.frame.userContext
        ;
    *reinterpret_cast<uint8_t*>(&msg.frame.flags) = flags;
    msg.frame.dlc = dlc;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const CanFrameTransmitEvent& ack)
{
    buffer << ack.transmitId
           << ack.canId
           << ack.timestamp
           << ack.status;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, CanFrameTransmitEvent& ack)
{
    buffer >> ack.transmitId
           >> ack.canId
           >> ack.timestamp
           >> ack.status;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const CanControllerStatus& msg)
{
    buffer << msg.timestamp
           << msg.controllerState
           << msg.errorState;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, CanControllerStatus& msg)
{
    buffer >> msg.timestamp
           >> msg.controllerState
           >> msg.errorState;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const CanConfigureBaudrate& msg)
{
    buffer << msg.baudRate
           << msg.fdBaudRate;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, CanConfigureBaudrate& msg)
{
    buffer >> msg.baudRate
           >> msg.fdBaudRate;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const CanSetControllerMode& msg)
{
    buffer << *reinterpret_cast<const uint8_t*>(&msg.flags)
           << msg.mode;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, CanSetControllerMode& msg)
{
    uint8_t flags;
    buffer >> flags
           >> msg.mode;
    *reinterpret_cast<uint8_t*>(&msg.flags) = flags;
    return buffer;
}


} // namespace can    
} // namespace sim
} // namespace ib
