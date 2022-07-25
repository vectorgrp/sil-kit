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

#include "EthernetSerdes.hpp"

#include <chrono>
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "EthDatatypeUtils.hpp"

using namespace std::chrono_literals;

TEST(MwVAsioSerdes, SimEthernet_EthMessage)
{
    using namespace SilKit::Services::Ethernet;
    SilKit::Core::MessageBuffer buffer;

    WireEthernetFrameEvent in;
    WireEthernetFrameEvent out;

    EthernetMac destinationMac{ 0x12, 0x23, 0x45, 0x67, 0x89, 0x9a };
    EthernetMac sourceMac{ 0x9a, 0x89, 0x67, 0x45, 0x23, 0x12 };
    EthernetEtherType etherType{ 0x0800 };

    in.timestamp = 13ns;
    in.frame = CreateEthernetFrame(destinationMac, sourceMac, etherType,
        "Hello from ethernet writer!  msgId = 1 -------------------------------------------------------");
    in.direction = SilKit::Services::TransmitDirection::TX;
    in.userContext = reinterpret_cast<void *>(0x1234);

    Serialize(buffer, in);
    Deserialize(buffer, out);

    EXPECT_EQ(in.timestamp, out.timestamp);
    EXPECT_TRUE(SilKit::Util::ItemsAreEqual(in.frame.raw.AsSpan(), out.frame.raw.AsSpan()));
    EXPECT_EQ(in.direction, out.direction);
    EXPECT_EQ(in.userContext, out.userContext);
}

TEST(MwVAsioSerdes, SimEthernet_EthTransmitAcknowledge)
{
    using namespace SilKit::Services::Ethernet;
    SilKit::Core::MessageBuffer buffer;

    EthernetFrameTransmitEvent in;
    EthernetFrameTransmitEvent out;

    in.timestamp = 13ns;
    in.status = EthernetTransmitStatus::Transmitted;
    in.userContext = reinterpret_cast<void *>(0x1234);

    Serialize(buffer, in);
    Deserialize(buffer, out);

    EXPECT_EQ(in.timestamp, out.timestamp);
    EXPECT_EQ(in.status, out.status);
    EXPECT_EQ(in.userContext, out.userContext);
}

TEST(MwVAsioSerdes, SimEthernet_EthStatus)
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

TEST(MwVAsioSerdes, SimEthernet_EthSetMode)
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
