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

#include <chrono>

#include "gtest/gtest.h"

using namespace std::chrono_literals;


const std::vector<uint8_t> referenceData(114'793, 'D');

TEST(Test_RpcSerdes, SimRpc_functionCall)
{
    using namespace SilKit::Services::Rpc;
    using namespace SilKit::Core;

    SilKit::Core::MessageBuffer buffer;
    FunctionCall in, out;
    in.callUuid ={1234565, 0x789abcdf};
    in.data = referenceData;
    in.timestamp = 12345ns;

    Serialize(buffer, in);
    Deserialize(buffer, out);
    EXPECT_EQ(in, out);
}

TEST(Test_RpcSerdes, SimRpc_functioncall_response)
{
    using namespace SilKit::Services::Rpc;
    using namespace SilKit::Core;

    SilKit::Core::MessageBuffer buffer;
    FunctionCallResponse in, out;
    in.callUuid ={1234565, 0x789abcdf};
    in.data = referenceData;
    in.timestamp = 12345ns;

    Serialize(buffer, in);
    Deserialize(buffer, out);
    EXPECT_EQ(in, out);
}
