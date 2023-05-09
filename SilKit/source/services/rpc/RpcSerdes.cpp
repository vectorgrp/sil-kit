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
