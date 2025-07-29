// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "RequestReplySerdes.hpp"

namespace SilKit {
namespace Core {
namespace RequestReply {

inline SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const RequestReplyCall& msg)
{
    buffer << msg.callUuid << msg.functionType << msg.callData;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, RequestReplyCall& out)
{
    buffer >> out.callUuid >> out.functionType >> out.callData;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const RequestReplyCallReturn& msg)
{
    buffer << msg.callUuid << msg.functionType << msg.callReturnData << msg.callReturnStatus;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, RequestReplyCallReturn& out)
{
    buffer >> out.callUuid >> out.functionType >> out.callReturnData >> out.callReturnStatus;
    return buffer;
}

void Serialize(SilKit::Core::MessageBuffer& buffer, const RequestReplyCall& msg)
{
    buffer << msg;
}

void Deserialize(MessageBuffer& buffer, RequestReplyCall& out)
{
    buffer >> out;
}

void Serialize(SilKit::Core::MessageBuffer& buffer, const RequestReplyCallReturn& msg)
{
    buffer << msg;
}

void Deserialize(MessageBuffer& buffer, RequestReplyCallReturn& out)
{
    buffer >> out;
}

} // namespace RequestReply
} // namespace Core
} // namespace SilKit
