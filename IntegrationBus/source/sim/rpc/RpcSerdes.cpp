// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "RpcSerdes.hpp"

namespace ib {
namespace sim {
namespace rpc {

ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const CallUUID& msg)
{
    buffer << msg.ab << msg.cd;
    return buffer;
}
ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, CallUUID& msg)
{
    buffer >> msg.ab >> msg.cd;
    return buffer;
}
ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const FunctionCall& msg)
{
    buffer << msg.timestamp << msg.callUUID << msg.data;
    return buffer;
}
ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, FunctionCall& msg)
{
    buffer >> msg.timestamp >> msg.callUUID >> msg.data;
    return buffer;
}
ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const FunctionCallResponse& msg)
{
    buffer << msg.timestamp << msg.callUUID << msg.data;
    return buffer;
}
ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, FunctionCallResponse& msg)
{
    buffer >> msg.timestamp >> msg.callUUID >> msg.data;
    return buffer;
}
using ib::mw::MessageBuffer;

void Serialize(MessageBuffer& buffer, const FunctionCall& msg)
{
    buffer << msg;
    return;
}
void Serialize(MessageBuffer& buffer, const FunctionCallResponse& msg)
{
    buffer << msg;
    return;
}

void Deserialize(MessageBuffer& buffer, FunctionCall& out)
{
    buffer >> out;
}
void Deserialize(MessageBuffer& buffer, FunctionCallResponse& out)
{
    buffer >> out;
}
} // namespace rpc
} // namespace sim
} // namespace ib
