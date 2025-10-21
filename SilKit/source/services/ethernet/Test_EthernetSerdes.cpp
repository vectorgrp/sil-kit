// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "EthernetSerdes.hpp"

#include <chrono>
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "EthDatatypeUtils.hpp"

using namespace std::chrono_literals;

TEST(Test_EthernetSerdes, SimEthernet_EthMessage)
{
    using namespace SilKit::Services::Ethernet;
    SilKit::Core::MessageBuffer buffer;

    WireEthernetFrameEvent in;
    WireEthernetFrameEvent out;

    EthernetMac destinationMac{0x12, 0x23, 0x45, 0x67, 0x89, 0x9a};
    EthernetMac sourceMac{0x9a, 0x89, 0x67, 0x45, 0x23, 0x12};
    EthernetEtherType etherType{0x0800};

    in.timestamp = 13ns;
    in.frame = CreateEthernetFrame(
        destinationMac, sourceMac, etherType,
        "Hello from ethernet writer!  msgId = 1 -------------------------------------------------------");
    in.direction = SilKit::Services::TransmitDirection::TX;
    in.userContext = reinterpret_cast<void*>(0x1234);

    Serialize(buffer, in);
    Deserialize(buffer, out);

    EXPECT_EQ(in.timestamp, out.timestamp);
    EXPECT_TRUE(SilKit::Util::ItemsAreEqual(in.frame.raw.AsSpan(), out.frame.raw.AsSpan()));
    EXPECT_EQ(in.direction, out.direction);
    EXPECT_EQ(in.userContext, out.userContext);
}

TEST(Test_EthernetSerdes, SimEthernet_EthTransmitAcknowledge)
{
    using namespace SilKit::Services::Ethernet;
    SilKit::Core::MessageBuffer buffer;

    EthernetFrameTransmitEvent in;
    EthernetFrameTransmitEvent out;

    in.timestamp = 13ns;
    in.status = EthernetTransmitStatus::Transmitted;
    in.userContext = reinterpret_cast<void*>(0x1234);

    Serialize(buffer, in);
    Deserialize(buffer, out);

    EXPECT_EQ(in.timestamp, out.timestamp);
    EXPECT_EQ(in.status, out.status);
    EXPECT_EQ(in.userContext, out.userContext);
}

TEST(Test_EthernetSerdes, SimEthernet_EthStatus)
{
    using namespace SilKit::Services::Ethernet;
    SilKit::Core::MessageBuffer buffer;

    EthernetStatus in;
    EthernetStatus out;

    in.timestamp = 13ns;
    in.state = EthernetState::LinkUp;
    in.bitrate = 4294967295;

    Serialize(buffer, in);
    Deserialize(buffer, out);

    EXPECT_EQ(in.timestamp, out.timestamp);
    EXPECT_EQ(in.state, out.state);
    EXPECT_EQ(in.bitrate, out.bitrate);
}

TEST(Test_EthernetSerdes, SimEthernet_EthSetMode)
{
    using namespace SilKit::Services::Ethernet;
    SilKit::Core::MessageBuffer buffer;

    EthernetSetMode in;
    EthernetSetMode out;

    in.mode = EthernetMode::Active;

    Serialize(buffer, in);
    Deserialize(buffer, out);

    EXPECT_EQ(in.mode, out.mode);
}
