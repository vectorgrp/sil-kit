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

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const LinSendFrameRequest& frame)
{
    buffer
        << frame.frame
        << frame.responseType;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, LinSendFrameRequest& frame)
{
    buffer
        >> frame.frame
        >> frame.responseType;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const LinSendFrameHeaderRequest& header)
{
    buffer
        << header.id;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, LinSendFrameHeaderRequest& header)
{
    buffer
        >> header.id;
    return buffer;
}
inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const LinTransmission& transmission)
{
    buffer
        << transmission.timestamp
        << transmission.frame
        << transmission.status;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, LinTransmission& transmission)
{
    buffer
        >> transmission.timestamp
        >> transmission.frame
        >> transmission.status;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const LinWakeupPulse& pulse)
{
    buffer
        << pulse.timestamp
        << pulse.direction;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, LinWakeupPulse& pulse)
{
    buffer
        >> pulse.timestamp
        >> pulse.direction;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const LinFrameResponse& response)
{
    buffer
        << response.frame
        << response.responseMode;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, LinFrameResponse& response)
{
    buffer
        >> response.frame
        >> response.responseMode;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const LinControllerConfig& config)
{
    buffer
        << config.controllerMode
        << config.baudRate
        << config.frameResponses;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, LinControllerConfig& config)
{
    buffer
        >> config.controllerMode
        >> config.baudRate
        >> config.frameResponses;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const LinControllerStatusUpdate& msg)
{
    buffer
        << msg.timestamp
        << msg.status;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, LinControllerStatusUpdate& msg)
{
    buffer
        >> msg.timestamp
        >> msg.status;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const LinFrameResponseUpdate& update)
{
    buffer
        << update.frameResponses;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, LinFrameResponseUpdate& update)
{
    buffer
        >> update.frameResponses;
    return buffer;
}


} // namespace lin
} // namespace sim
} // namespace ib