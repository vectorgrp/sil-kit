// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "OutPort.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "ib/mw/string_utils.hpp"
#include "ib/util/functional.hpp"

#include "MockComAdapter.hpp"

#include "IoDatatypeUtils.hpp"


namespace {

using namespace std::chrono_literals;

using namespace testing;

using namespace ib::mw;
using namespace ib::sim;

class OutPortTest : public ::testing::Test
{
protected:
    using MessageType = io::AnalogIoMessage;
    using ValueType = io::IOutPort<MessageType>::ValueType;
    using InterfaceType = io::IOutPort<MessageType>;

protected:
    OutPortTest()
        : port{&comAdapter}
    {
        port.SetEndpointAddress(portAddress);
    }

protected:
    const EndpointAddress portAddress{4, 5};
    const EndpointAddress otherPortAddress{5, 10};

    test::MockComAdapter comAdapter;
    io::OutPort<MessageType> port;
};

TEST_F(OutPortTest, check_endpoint_address)
{
    port.SetEndpointAddress(portAddress);
    EXPECT_EQ(port.EndpointAddress(), portAddress);
}

TEST_F(OutPortTest, send_ibmessage_on_write)
{
    MessageType msg;
    msg.timestamp = 13ns;
    msg.value = 17.3;

    EXPECT_CALL(comAdapter, SendIbMessage(portAddress, msg))
        .Times(1);

    port.Write(msg.value, msg.timestamp);
}

TEST_F(OutPortTest, can_read_last_value)
{
    ValueType value = 17.3;
    std::chrono::nanoseconds timestamp = 13ns;

    port.Write(value, timestamp);

    EXPECT_EQ(port.Read(), value);
}

} // anonymous namespace
