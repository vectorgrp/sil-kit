// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "InPort.hpp"

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
using namespace ib::sim::io;

using ::ib::mw::test::DummyComAdapter;

class MockComAdapter : public DummyComAdapter
{
public:
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const AnalogIoMessage&));
};

class InPortTest : public ::testing::Test
{
protected:
    using MessageType = AnalogIoMessage;
    using ValueType = IInPort<MessageType>::ValueType;
    using InterfaceType = IInPort<MessageType>;

protected:
    struct Callbacks
    {
        MOCK_METHOD2(ReceiveMessage, void(InterfaceType* port, const InterfaceType::MessageType& msg));
        MOCK_METHOD2(ReceiveValue, void(InterfaceType* port, const InterfaceType::ValueType& value));
    };

protected:
    InPortTest()
        : port{&comAdapter, comAdapter.GetTimeProvider()}
    {
        port.SetEndpointAddress(portAddress);
    }

    void RegisterMessageCallback()
    {
        port.RegisterHandler(ib::util::bind_method(&callbacks, &Callbacks::ReceiveMessage));
    }
    void RegisterValueCallback()
    {
        port.RegisterHandler(ib::util::bind_method(&callbacks, &Callbacks::ReceiveValue));
    }

protected:
    const EndpointAddress portAddress{4, 5};
    const EndpointAddress otherPortAddress{5, 10};

    MockComAdapter comAdapter;
    InPort<MessageType> port;
    Callbacks callbacks;
};


TEST_F(InPortTest, trigger_callbacks_on_receive)
{
    RegisterMessageCallback();
    RegisterValueCallback();

    MessageType msg;
    msg.timestamp = 13ns;
    msg.value = 17.3;

    EXPECT_CALL(callbacks, ReceiveMessage(&port, msg))
        .Times(1);
    EXPECT_CALL(callbacks, ReceiveValue(&port, msg.value))
        .Times(1);
    EXPECT_CALL(comAdapter.mockTimeProvider.mockTime, Now())
        .Times(1);

    port.ReceiveIbMessage(otherPortAddress, msg);
}

TEST_F(InPortTest, ignore_own_messages)
{
    RegisterMessageCallback();
    RegisterValueCallback();

    MessageType msg;

    EXPECT_CALL(callbacks, ReceiveMessage(&port, A<const MessageType&>()))
        .Times(0);
    EXPECT_CALL(callbacks, ReceiveValue(&port, A<const ValueType&>()))
        .Times(0);
    EXPECT_CALL(comAdapter.mockTimeProvider.mockTime, Now())
        .Times(0);

    port.ReceiveIbMessage(portAddress, msg);
}

TEST_F(InPortTest, can_read_last_value)
{
    MessageType msg;
    msg.timestamp = 13ns;
    msg.value = 17.3;

    port.ReceiveIbMessage(otherPortAddress, msg);

    EXPECT_EQ(port.Read(), msg.value);
}

} // anonymous namespace
