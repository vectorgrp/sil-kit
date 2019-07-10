// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "MessageBuffer.hpp"

#include "ib/sim/eth/EthDatatypes.hpp"

namespace ib {
namespace sim {
namespace eth {

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const EthMessage& msg)
{
    buffer << msg.transmitId
           << msg.timestamp
           << msg.ethFrame.RawFrame();

    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, EthMessage& msg)
{
    std::vector<uint8_t> rawFrame;

    buffer >> msg.transmitId
           >> msg.timestamp
           >> rawFrame;

    msg.ethFrame = EthFrame{std::move(rawFrame)};

    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const EthTransmitAcknowledge& ack)
{
    buffer << ack.transmitId
           << ack.timestamp
           << ack.status;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, EthTransmitAcknowledge& ack)
{
    buffer >> ack.transmitId
           >> ack.timestamp
           >> ack.status;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const EthStatus& msg)
{
    buffer << msg.timestamp
           << msg.state
           << msg.bitRate;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, EthStatus& msg)
{
    buffer >> msg.timestamp
           >> msg.state
           >> msg.bitRate;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const EthSetMode& msg)
{
    buffer << msg.mode;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, EthSetMode& msg)
{
    buffer >> msg.mode;
    return buffer;
}


} // namespace eth    
} // namespace sim
} // namespace ib
