// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "MessageBuffer.hpp"

#include "ib/sim/rpc/RpcDatatypes.hpp"

namespace ib {
namespace sim {
namespace rpc {

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const CallUUID& msg)
{
    buffer << msg.ab << msg.cd;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, CallUUID& msg)
{
    buffer >> msg.ab >> msg.cd;
    return buffer;
}
inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const FunctionCall& msg)
{
    buffer << msg.callUUID << msg.data;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, FunctionCall& msg)
{
    buffer >> msg.callUUID >> msg.data;
    return buffer;
}
inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const FunctionCallResponse& msg)
{
    buffer << msg.callUUID << msg.data;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, FunctionCallResponse& msg)
{
    buffer >> msg.callUUID >> msg.data;
    return buffer;
}

} // namespace rpc    
} // namespace sim
} // namespace ib
