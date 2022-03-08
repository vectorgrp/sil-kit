// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "SerdesSimRpc.hpp"
#include "SerdesMw.hpp"
#include "VAsioMsgKind.hpp"

#include <chrono>

#include "gtest/gtest.h"

using namespace std::chrono_literals;

struct PackedMsg
{
    uint32_t msgSizePlaceholder;
    ib::mw::VAsioMsgKind msgKind{ib::mw::VAsioMsgKind::IbSimMsg};
    ib::mw::EndpointId receiverIdx;
    ib::mw::EndpointAddress from;
    std::vector<uint8_t> data;
};

bool operator==(const PackedMsg& lhs, const PackedMsg& rhs)
{
    return lhs.msgSizePlaceholder == rhs.msgSizePlaceholder
        && lhs.msgKind == rhs.msgKind
        && lhs.receiverIdx == rhs.receiverIdx
        && lhs.from == rhs.from
        && lhs.data == rhs.data;
}

TEST(MwVAsioSerdes, SimRpc_LargeDataMessage)
{
    using namespace ib::sim::rpc;
    using namespace ib::mw;

    const std::vector<uint8_t> referenceData(114'793, 'D');
    ib::mw::MessageBuffer buffer;

    PackedMsg in1;
    in1.msgSizePlaceholder = 1u;
    in1.receiverIdx = 1u;
    in1.from = {1,3};
    in1.data = referenceData;

    PackedMsg in2;
    in2.msgSizePlaceholder = 2u;
    in2.receiverIdx = 2u;
    in2.from = {2,3};
    in2.data = referenceData;

    buffer
        << in1.msgSizePlaceholder
        << in1.msgKind
        << in1.receiverIdx
        << in1.from
        << in1.data
        << in2.msgSizePlaceholder
        << in2.msgKind
        << in2.receiverIdx
        << in2.from
        << in2.data;


    PackedMsg out1;
    PackedMsg out2;

    buffer
        >> out1.msgSizePlaceholder
        >> out1.msgKind
        >> out1.receiverIdx
        >> out1.from
        >> out1.data
        >> out2.msgSizePlaceholder
        >> out2.msgKind
        >> out2.receiverIdx
        >> out2.from
        >> out2.data;

    EXPECT_EQ(in1, out1);
    EXPECT_EQ(in2, out2);
}

