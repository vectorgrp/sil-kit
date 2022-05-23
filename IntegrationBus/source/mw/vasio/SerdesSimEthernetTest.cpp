// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "SerdesSimEthernet.hpp"

#include <chrono>
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "EthDatatypeUtils.hpp"

using namespace std::chrono_literals;

TEST(MwVAsioSerdes, SimEthernet_EthMessage)
{
    using namespace ib::sim::eth;
    ib::mw::MessageBuffer buffer;

    EthernetFrameEvent in;
    EthernetFrameEvent out;

    EthernetMac destinationMac{ 0x12, 0x23, 0x45, 0x67, 0x89, 0x9a };
    EthernetMac sourceMac{ 0x9a, 0x89, 0x67, 0x45, 0x23, 0x12 };
    EthernetEtherType etherType{ 0x0800 };

    in.transmitId = 5;
    in.timestamp = 13ns;
    in.frame = CreateEthernetFrame(destinationMac, sourceMac, etherType,
        "Hello from ethernet writer!  msgId = 1 -------------------------------------------------------");

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in.transmitId, out.transmitId);
    EXPECT_EQ(in.timestamp, out.timestamp);
    ASSERT_THAT(in.frame.raw, testing::ContainerEq(out.frame.raw));
}

TEST(MwVAsioSerdes, SimEthernet_EthTransmitAcknowledge)
{
    using namespace ib::sim::eth;
    ib::mw::MessageBuffer buffer;

    EthernetFrameTransmitEvent in;
    EthernetFrameTransmitEvent out;

    in.transmitId = 5;
    in.timestamp = 13ns;
    in.status = EthernetTransmitStatus::Transmitted;

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

    EthernetStatus in;
    EthernetStatus out;

    in.timestamp = 13ns;
    in.state = EthernetState::LinkUp;
    in.bitrate = 4294967295;

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in.timestamp, out.timestamp);
    EXPECT_EQ(in.state, out.state);
    EXPECT_EQ(in.bitrate, out.bitrate);
}

TEST(MwVAsioSerdes, SimEthernet_EthSetMode)
{
    using namespace ib::sim::eth;
    ib::mw::MessageBuffer buffer;

    EthernetSetMode in;
    EthernetSetMode out;

    in.mode = EthernetMode::Active;

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in.mode, out.mode);
}
