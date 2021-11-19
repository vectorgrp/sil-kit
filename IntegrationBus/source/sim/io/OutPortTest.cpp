// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "OutPort.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "ib/mw/string_utils.hpp"
#include "ib/util/functional.hpp"

#include "MockComAdapter.hpp"
#include "MockTraceSink.hpp"

#include "IoDatatypeUtils.hpp"

namespace {

using namespace std::chrono_literals;

using namespace testing;

using namespace ib::mw;
using namespace ib::sim;
using namespace ib::sim::io;

using ::ib::mw::test::DummyComAdapter;

class MockComAdapter : public DummyComAdapter
{
public:
    MOCK_METHOD2(SendIbMessage, void(const IServiceId*, const AnalogIoMessage&));
};

class OutPortTest : public ::testing::Test
{
protected:
    using MessageType = AnalogIoMessage;
    using ValueType = IOutPort<MessageType>::ValueType;
    using InterfaceType = IOutPort<MessageType>;

protected:
    OutPortTest()
        : port{&comAdapter, comAdapter.GetTimeProvider()}
    {
        port.SetEndpointAddress(portAddress);
    }

protected:
    const EndpointAddress portAddress{4, 5};
    const EndpointAddress otherPortAddress{5, 10};

    MockComAdapter comAdapter;
    ib::test::MockTraceSink traceSink;
    OutPort<MessageType> port;
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

    EXPECT_CALL(comAdapter, SendIbMessage(&port, msg))
        .Times(1);

    EXPECT_CALL(comAdapter.mockTimeProvider.mockTime, Now())
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

TEST_F(OutPortTest, uses_tracing)
{
    using namespace ib::extensions;
    port.AddSink(&traceSink);

    MessageType msg;
    msg.timestamp = 13ns;
    msg.value = 17.3;
    const auto now = 123456ns;

    ON_CALL(comAdapter.mockTimeProvider.mockTime, Now())
        .WillByDefault(testing::Return(now));

    EXPECT_CALL(comAdapter.mockTimeProvider.mockTime, Now())
        .Times(1);
    EXPECT_CALL(traceSink,
        Trace(Direction::Send, portAddress, now, msg))
        .Times(1);

    port.Write(msg.value, msg.timestamp);
}


} // anonymous namespace
