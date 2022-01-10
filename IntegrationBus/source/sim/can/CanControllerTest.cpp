// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <functional>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ib/sim/can/string_utils.hpp"

#include "MockComAdapter.hpp"
#include "MockTraceSink.hpp"

#include "CanController.hpp"
#include "CanDatatypesUtils.hpp"

namespace {

using namespace std::chrono_literals;

using testing::Return;
using testing::A;
using testing::An;
using testing::_;
using testing::InSequence;
using testing::NiceMock;

using namespace ib::mw;
using namespace ib::sim::can;

using ib::mw::test::DummyComAdapter;

MATCHER_P(CanTransmitAckWithouthTransmitIdMatcher, truthAck, "matches CanTransmitAcks without checking the transmit id") {
    auto frame1 = truthAck;
    auto frame2 = arg;
    return frame1.canId == frame2.canId && frame1.status == frame2.status && frame1.timestamp == frame2.timestamp;
    return true;
}

class MockComAdapter : public DummyComAdapter
{
public:
    MOCK_METHOD2(SendIbMessage, void(const IIbServiceEndpoint*, const CanMessage&));
    MOCK_METHOD2(SendIbMessage, void(const IIbServiceEndpoint*, const CanTransmitAcknowledge&));
    MOCK_METHOD2(SendIbMessage, void(const IIbServiceEndpoint*, const CanConfigureBaudrate&));
    MOCK_METHOD2(SendIbMessage, void(const IIbServiceEndpoint*, const CanSetControllerMode&));
};

class CanControllerCallbacks
{
public:
    MOCK_METHOD2(ReceiveMessage, void(ICanController*, CanMessage));
    MOCK_METHOD2(StateChanged, void(ICanController*, CanControllerState));
    MOCK_METHOD2(ErrorStateChanged, void(ICanController*, CanErrorState));
    MOCK_METHOD2(ReceiveAck, void(ICanController*, CanTransmitAcknowledge));
};

class CanControllerTest : public testing::Test
{
public:
    CanControllerTest() {};
};

TEST(CanControllerTest, send_can_message)
{
    MockComAdapter mockComAdapter;

    const EndpointAddress controllerAddress = { 3, 8 };

    CanController canController(&mockComAdapter, mockComAdapter.GetTimeProvider());
    canController.SetServiceDescriptor(from_endpointAddress(controllerAddress));


    CanMessage msg;
    msg.transmitId = 1;

    EXPECT_CALL(mockComAdapter, SendIbMessage(&canController, msg))
        .Times(1);
    EXPECT_CALL(mockComAdapter.mockTimeProvider.mockTime, Now())
        .Times(1);

    canController.SendMessage(msg);
}

TEST(CanControllerTest, receive_can_message)
{
    using namespace std::placeholders;

    EndpointAddress senderAddress{ 17, 4 };

    MockComAdapter mockComAdapter;
    CanControllerCallbacks callbackProvider;

    CanController canController(&mockComAdapter, mockComAdapter.GetTimeProvider());
    canController.RegisterReceiveMessageHandler(std::bind(&CanControllerCallbacks::ReceiveMessage, &callbackProvider, _1, _2));

    CanMessage msg;
    msg.canId = 16;

    EXPECT_CALL(callbackProvider, ReceiveMessage(&canController, msg))
        .Times(1);
    EXPECT_CALL(mockComAdapter.mockTimeProvider.mockTime, Now()).Times(1);

    CanController canControllerProxy(&mockComAdapter, mockComAdapter.GetTimeProvider());
    canControllerProxy.SetServiceDescriptor(from_endpointAddress(senderAddress));
    auto& id = canControllerProxy.GetServiceDescriptor();
    canController.ReceiveIbMessage(&canControllerProxy, msg);
}


/*! \brief Ensure that start, stop, sleep, and reset have no effect
 *
 *  Rationale:
 *   ControllerModes are not supported by the CanController. This feature requires
 *   the usage of a proper CanBusSimulator and CanControllerProxies.
 */
TEST(CanControllerTest, start_stop_sleep_reset)
{
    MockComAdapter mockComAdapter;

    CanController canController(&mockComAdapter, mockComAdapter.GetTimeProvider());

    EXPECT_CALL(mockComAdapter, SendIbMessage(A<const IIbServiceEndpoint*>(), A<const CanSetControllerMode&>()))
        .Times(0);

    canController.Start();
    canController.Stop();
    canController.Sleep();
    canController.Reset();
}

/*! \brief Ensure that SetBaudRate has no effect
*
*  Rationale:
*   BaudRate is not supported by the CanController. This feature requires
*   the usage of a proper CanBusSimulator and CanControllerProxies.
*/
TEST(CanControllerTest, set_baudrate)
{
    MockComAdapter mockComAdapter;

    CanController canController(&mockComAdapter, mockComAdapter.GetTimeProvider());

    EXPECT_CALL(mockComAdapter, SendIbMessage(An<const IIbServiceEndpoint*>(), A<const CanConfigureBaudrate&>()))
        .Times(0);

    canController.SetBaudRate(3000, 500000);
}

TEST(CanControllerTest, receive_ack)
{
    using namespace std::placeholders;

    EndpointAddress controllerAddress = { 3, 8 };
    EndpointAddress senderAddress = { 4, 9 };

    MockComAdapter mockComAdapter;
    CanControllerCallbacks callbackProvider;

    CanController canController(&mockComAdapter, mockComAdapter.GetTimeProvider());
    canController.SetServiceDescriptor(from_endpointAddress(controllerAddress));
    canController.RegisterTransmitStatusHandler(std::bind(&CanControllerCallbacks::ReceiveAck, &callbackProvider, _1, _2));

    CanMessage msg{};
    CanTransmitAcknowledge ack1{ 0, msg.canId, 0ns, CanTransmitStatus::Transmitted };
    CanTransmitAcknowledge ack2{ 0, msg.canId, 0ns, CanTransmitStatus::Transmitted };

    EXPECT_CALL(callbackProvider, ReceiveAck(&canController, CanTransmitAckWithouthTransmitIdMatcher(ack1)))
        .Times(2);
    auto txId1 = canController.SendMessage(msg);
    auto txId2 = canController.SendMessage(msg);

    ASSERT_NE(txId1, txId2);
}


TEST(CanControllerTest, cancontroller_uses_tracing)
{
    using namespace ib::extensions;

    ib::test::MockTraceSink traceSink;
    test::DummyComAdapter comAdapter;
    const std::chrono::nanoseconds now = 1337ns;
    const EndpointAddress controllerAddress = {1,2};
    const EndpointAddress otherAddress = {2,2};

    ON_CALL(comAdapter.mockTimeProvider.mockTime, Now())
        .WillByDefault(testing::Return(now));


    auto controller = CanController(&comAdapter, comAdapter.GetTimeProvider());
    controller.SetServiceDescriptor(from_endpointAddress(controllerAddress));
    controller.AddSink(&traceSink);


    CanMessage msg{};

    //Send direction
    EXPECT_CALL(comAdapter.mockTimeProvider.mockTime, Now())
        .Times(1);
    EXPECT_CALL(traceSink,
        Trace(Direction::Send, controllerAddress, now, msg))
        .Times(1);
    controller.SendMessage(msg);

    // Receive direction
    EXPECT_CALL(comAdapter.mockTimeProvider.mockTime, Now())
        .Times(1);
    EXPECT_CALL(traceSink,
        Trace(Direction::Receive, controllerAddress, now, msg))
        .Times(1);

    CanController canControllerProxy(&comAdapter, comAdapter.GetTimeProvider());
    canControllerProxy.SetServiceDescriptor(from_endpointAddress(otherAddress));
    controller.ReceiveIbMessage(&canControllerProxy, msg);
}
}  // anonymous namespace
