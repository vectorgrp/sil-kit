/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include "RequestReplySerdes.hpp"

namespace SilKit {
namespace Core {
namespace RequestReply {

inline SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const RequestReplyCall& msg)
{
    buffer
        << msg.callUuid
        << msg.functionType
        << msg.callData
        ;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, RequestReplyCall& out)
{
    buffer
        >> out.callUuid
        >> out.functionType
        >> out.callData
        ;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const RequestReplyCallReturn& msg)
{
    buffer
        << msg.callUuid
        << msg.functionType
        << msg.callReturnData
        << msg.callReturnStatus
        ;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, RequestReplyCallReturn& out)
{
    buffer
        >> out.callUuid
        >> out.functionType
        >> out.callReturnData
        >> out.callReturnStatus
        ;
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
