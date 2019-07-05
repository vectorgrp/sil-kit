// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "MessageBuffer.hpp"

#include "ib/sim/lin/LinDatatypes.hpp"

namespace ib {
namespace sim {
namespace lin {

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const Payload& payload)
{
    buffer
        << payload.size
        << payload.data;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, Payload& payload)
{
    buffer
        >> payload.size
        >> payload.data;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const LinMessage& msg)
{
    buffer
        << msg.status
        << msg.timestamp
        << msg.linId
        << msg.payload
        << msg.checksumModel;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, LinMessage& msg)
{
    buffer
        >> msg.status
        >> msg.timestamp
        >> msg.linId
        >> msg.payload
        >> msg.checksumModel;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const RxRequest& request)
{
    buffer
        << request.linId
        << request.payloadLength
        << request.checksumModel;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, RxRequest& request)
{
    buffer
        >> request.linId
        >> request.payloadLength
        >> request.checksumModel;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const TxAcknowledge& ack)
{
    buffer
        << ack.timestamp
        << ack.linId
        << ack.status;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, TxAcknowledge& ack)
{
    buffer
        >> ack.timestamp
        >> ack.linId
        >> ack.status;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const WakeupRequest& request)
{
    buffer
        << request.timestamp;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, WakeupRequest& request)
{
    buffer
        >> request.timestamp;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const ControllerConfig& config)
{
    buffer
        << config.controllerMode
        << config.baudrate;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, ControllerConfig& config)
{
    buffer
        >> config.controllerMode
        >> config.baudrate;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const SlaveResponseConfig& config)
{
    buffer
        << config.linId
        << config.responseMode
        << config.checksumModel
        << config.payloadLength;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, SlaveResponseConfig& config)
{
    buffer
        >> config.linId
        >> config.responseMode
        >> config.checksumModel
        >> config.payloadLength;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const SlaveConfiguration& config)
{
    buffer
        << config.responseConfigs;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, SlaveConfiguration& config)
{
    buffer
        >> config.responseConfigs;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const SlaveResponse& response)
{
    buffer
        << response.linId
        << response.payload
        << response.checksumModel;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, SlaveResponse& response)
{
    buffer
        >> response.linId
        >> response.payload
        >> response.checksumModel;
    return buffer;
}

} // namespace lin
} // namespace sim
} // namespace ib