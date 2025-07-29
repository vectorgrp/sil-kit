// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "RpcSerdes.hpp"

namespace SilKit {
namespace Services {
namespace Rpc {

inline SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const FunctionCall& msg)
{
    buffer << msg.timestamp << msg.callUuid << msg.data;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, FunctionCall& msg)
{
    buffer >> msg.timestamp >> msg.callUuid >> msg.data;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const FunctionCallResponse& msg)
{
    buffer << msg.timestamp << msg.callUuid << msg.data << msg.status;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, FunctionCallResponse& msg)
{
    buffer >> msg.timestamp >> msg.callUuid >> msg.data >> msg.status;
    return buffer;
}

using SilKit::Core::MessageBuffer;

void Serialize(MessageBuffer& buffer, const FunctionCall& msg)
{
    buffer << msg;
}
void Serialize(MessageBuffer& buffer, const FunctionCallResponse& msg)
{
    buffer << msg;
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
