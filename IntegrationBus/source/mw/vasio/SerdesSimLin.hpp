// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "MessageBuffer.hpp"

#include "ib/sim/lin/LinDatatypes.hpp"

namespace ib {
namespace sim {
namespace lin {

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const LinFrame& frame)
{
    buffer
        << frame.id
        << frame.checksumModel
        << frame.dataLength
        << frame.data;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, LinFrame& frame)
{
    buffer
        >> frame.id
        >> frame.checksumModel
        >> frame.dataLength
        >> frame.data;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const SendFrameRequest& frame)
{
    buffer
        << frame.frame
        << frame.responseType;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, SendFrameRequest& frame)
{
    buffer
        >> frame.frame
        >> frame.responseType;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const SendFrameHeaderRequest& header)
{
    buffer
        << header.id;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, SendFrameHeaderRequest& header)
{
    buffer
        >> header.id;
    return buffer;
}
inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const Transmission& transmission)
{
    buffer
        << transmission.timestamp
        << transmission.frame
        << transmission.status;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, Transmission& transmission)
{
    buffer
        >> transmission.timestamp
        >> transmission.frame
        >> transmission.status;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const WakeupPulse& pulse)
{
    buffer
        << pulse.timestamp
        << pulse.direction;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, WakeupPulse& pulse)
{
    buffer
        >> pulse.timestamp
        >> pulse.direction;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const FrameResponse& response)
{
    buffer
        << response.frame
        << response.responseMode;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, FrameResponse& response)
{
    buffer
        >> response.frame
        >> response.responseMode;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const ControllerConfig& config)
{
    buffer
        << config.controllerMode
        << config.baudRate
        << config.frameResponses;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, ControllerConfig& config)
{
    buffer
        >> config.controllerMode
        >> config.baudRate
        >> config.frameResponses;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const ControllerStatusUpdate& msg)
{
    buffer
        << msg.timestamp
        << msg.status;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, ControllerStatusUpdate& msg)
{
    buffer
        >> msg.timestamp
        >> msg.status;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const FrameResponseUpdate& update)
{
    buffer
        << update.frameResponses;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, FrameResponseUpdate& update)
{
    buffer
        >> update.frameResponses;
    return buffer;
}


} // namespace lin
} // namespace sim
} // namespace ib