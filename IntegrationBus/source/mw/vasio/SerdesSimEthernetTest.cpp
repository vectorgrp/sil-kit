// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "SerdesSimEthernet.hpp"

#include <chrono>
#include "gtest/gtest.h"
#include "gmock/gmock.h"

using namespace std::chrono_literals;

TEST(MwVAsioSerdes, SimEthernet_EthMessage)
{
    using namespace ib::sim::eth;
    ib::mw::MessageBuffer buffer;

    EthMessage in;
    EthMessage out;

    std::string payload{ "Hello from ethernet writer!  msgId = 1 -------------------------------------------------------" };
    EthMac sourceMac{1, 2, 3, 4, 5, 6};
    EthMac destinationMac{6, 5, 4, 3, 2, 1};

    EthTagControlInformation tci;
    tci.pcp = 3;
    tci.dei = 0;
    tci.vid = 1;

    in.transmitId = 5;
    in.timestamp = 13ns;
    in.ethFrame.SetSourceMac(sourceMac);
    in.ethFrame.SetDestinationMac(destinationMac);
    in.ethFrame.SetVlanTag(tci);
    in.ethFrame.SetPayload(std::vector<uint8_t>{payload.begin(), payload.end()});

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in.transmitId, out.transmitId);
    EXPECT_EQ(in.timestamp, out.timestamp);
    EXPECT_EQ(in.ethFrame.GetSourceMac(), out.ethFrame.GetSourceMac());
    EXPECT_EQ(in.ethFrame.GetDestinationMac(), out.ethFrame.GetDestinationMac());
    EXPECT_EQ(in.ethFrame.GetVlanTag().pcp, out.ethFrame.GetVlanTag().pcp);
    EXPECT_EQ(in.ethFrame.GetVlanTag().dei, out.ethFrame.GetVlanTag().dei);
    EXPECT_EQ(in.ethFrame.GetVlanTag().vid, out.ethFrame.GetVlanTag().vid);

    std::vector<uint8_t> payloadIn{ in.ethFrame.GetPayload().begin(), in.ethFrame.GetPayload().end() };
    std::vector<uint8_t> payloadOut{ out.ethFrame.GetPayload().begin(), out.ethFrame.GetPayload().end() };

    ASSERT_THAT(payloadIn, payloadOut);

    EXPECT_EQ(in.ethFrame.RawFrame(), out.ethFrame.RawFrame());
}



TEST(MwVAsioSerdes, SimEthernet_EthTransmitAcknowledge)
{
    using namespace ib::sim::eth;
    ib::mw::MessageBuffer buffer;

    EthTransmitAcknowledge in;
    EthTransmitAcknowledge out;

    in.transmitId = 5;
    in.timestamp = 13ns;
    in.status = EthTransmitStatus::Transmitted;

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in.transmitId, out.transmitId);
    EXPECT_EQ(in.timestamp, out.timestamp);
    EXPECT_EQ(in.status, out.status);
}

TEST(MwVAsioSerdes, SimEthernet_EthStatus)
{
    using namespace ib::sim::eth;
    ib::mw::MessageBuffer buffer;

    EthStatus in;
    EthStatus out;

    in.timestamp = 13ns;
    in.state = EthState::LinkUp;
    in.bitRate = 4294967295;

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in.timestamp, out.timestamp);
    EXPECT_EQ(in.state, out.state);
    EXPECT_EQ(in.bitRate, out.bitRate);
}

TEST(MwVAsioSerdes, SimEthernet_EthSetMode)
{
    using namespace ib::sim::eth;
    ib::mw::MessageBuffer buffer;

    EthSetMode in;
    EthSetMode out;

    in.mode = EthMode::Active;

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in.mode, out.mode);
}
