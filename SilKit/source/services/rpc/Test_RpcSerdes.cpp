// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

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
    in.callUuid = {1234565, 0x789abcdf};
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
    in.callUuid = {1234565, 0x789abcdf};
    in.data = referenceData;
    in.timestamp = 12345ns;

    Serialize(buffer, in);
    Deserialize(buffer, out);
    EXPECT_EQ(in, out);
}
