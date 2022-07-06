// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "RpcSerdes.hpp"

namespace SilKit {
namespace Services {
namespace Rpc {

SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const CallUUID& msg)
{
    buffer << msg.ab << msg.cd;
    return buffer;
}
SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, CallUUID& msg)
{
    buffer >> msg.ab >> msg.cd;
    return buffer;
}
SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const FunctionCall& msg)
{
    buffer << msg.timestamp << msg.callUUID << msg.data;
    return buffer;
}
SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, FunctionCall& msg)
{
    buffer >> msg.timestamp >> msg.callUUID >> msg.data;
    return buffer;
}
SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const FunctionCallResponse& msg)
{
    buffer << msg.timestamp << msg.callUUID << msg.data;
    return buffer;
}
SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, FunctionCallResponse& msg)
{
    buffer >> msg.timestamp >> msg.callUUID >> msg.data;
    return buffer;
}
using SilKit::Core::MessageBuffer;

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
} // namespace Rpc
} // namespace Services
} // namespace SilKit
