// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "RpcSerdes.hpp"

#include <chrono>

#include "gtest/gtest.h"

using namespace std::chrono_literals;


const std::vector<uint8_t> referenceData(114'793, 'D');

TEST(MwVAsioSerdes, SimRpc_functionCall)
{
    using namespace ib::sim::rpc;
    using namespace ib::mw;

    ib::mw::MessageBuffer buffer;
    FunctionCall in, out;
    in.callUUID ={1234565, 0x789abcdf};
    in.data = referenceData;
    in.timestamp = 12345ns;

    Serialize(buffer, in);
    Deserialize(buffer, out);
    EXPECT_EQ(in, out);
}

TEST(MwVAsioSerdes, SimRpc_functioncall_response)
{
    using namespace ib::sim::rpc;
    using namespace ib::mw;

    ib::mw::MessageBuffer buffer;
    FunctionCallResponse in, out;
    in.callUUID ={1234565, 0x789abcdf};
    in.data = referenceData;
    in.timestamp = 12345ns;

    Serialize(buffer, in);
    Deserialize(buffer, out);
    EXPECT_EQ(in, out);
}
