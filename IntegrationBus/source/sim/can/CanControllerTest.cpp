// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <functional>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ib/mw/string_utils.hpp"
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

class MockComAdapter : public DummyComAdapter
{
public:
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const CanMessage&));
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const CanTransmitAcknowledge&));
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const CanConfigureBaudrate&));
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const CanSetControllerMode&));
};

class CanControllerCallbacks
{
public:
    MOCK_METHOD2(ReceiveMessage, void(ICanController*, CanMessage));
    MOCK_METHOD2(StateChanged, void(ICanController*, CanControllerState));
    MOCK_METHOD2(ErrorStateChanged, void(ICanController*, CanErrorState));
    MOCK_METHOD2(ReceiveAck, void(ICanController*, CanTransmitAcknowledge));
};

TEST(CanControllerTest, send_can_message)
{
    MockComAdapter mockComAdapter;

    const EndpointAddress controllerAddress = { 3, 8 };

    CanController canController(&mockComAdapter, mockComAdapter.GetTimeProvider());
    canController.SetEndpointAddress(controllerAddress);


    CanMessage msg;
    msg.transmitId = 1;

    EXPECT_CALL(mockComAdapter, SendIbMessage(controllerAddress, msg))
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

    canController.ReceiveIbMessage(senderAddress, msg);
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

    EXPECT_CALL(mockComAdapter, SendIbMessage(A<EndpointAddress>(), A<const CanSetControllerMode&>()))
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

    EXPECT_CALL(mockComAdapter, SendIbMessage(An<EndpointAddress>(), A<const CanConfigureBaudrate&>()))
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
    canController.SetEndpointAddress(controllerAddress);
    canController.RegisterTransmitStatusHandler(std::bind(&CanControllerCallbacks::ReceiveAck, &callbackProvider, _1, _2));

    CanMessage msg;
    auto txId1 = canController.SendMessage(msg);
    auto txId2 = canController.SendMessage(msg);

    ASSERT_NE(txId1, txId2);

    CanTransmitAcknowledge ack1{txId1, msg.canId, 0ns, CanTransmitStatus::Transmitted};
    CanTransmitAcknowledge ack2{txId2, msg.canId, 0ns, CanTransmitStatus::Transmitted};

    EXPECT_CALL(callbackProvider, ReceiveAck(&canController, ack1))
        .Times(1);
    EXPECT_CALL(callbackProvider, ReceiveAck(&canController, ack2))
        .Times(1);

    canController.ReceiveIbMessage(senderAddress, ack1);
    canController.ReceiveIbMessage(senderAddress, ack2);
}

TEST(CanControllerTest, ignore_unknown_acks)
{
    using namespace std::placeholders;

    EndpointAddress controllerAddress = {3, 8};
    EndpointAddress senderAddress = {4, 9};

    MockComAdapter mockComAdapter;
    CanControllerCallbacks callbackProvider;

    CanController canController(&mockComAdapter, mockComAdapter.GetTimeProvider());
    canController.SetEndpointAddress(controllerAddress);
    canController.RegisterTransmitStatusHandler(std::bind(&CanControllerCallbacks::ReceiveAck, &callbackProvider, _1, _2));

    CanMessage msg;
    msg.canId = 3;
    auto txId1 = canController.SendMessage(msg);
    auto txId2 = canController.SendMessage(msg);

    ASSERT_NE(txId1, txId2);

    CanTransmitAcknowledge ack1{txId1, 4, 0ns, CanTransmitStatus::Transmitted};
    CanTransmitAcknowledge ack2{txId2, 4, 0ns, CanTransmitStatus::Transmitted};

    EXPECT_CALL(callbackProvider, ReceiveAck(&canController, ack1))
        .Times(0);
    EXPECT_CALL(callbackProvider, ReceiveAck(&canController, ack2))
        .Times(0);

    canController.ReceiveIbMessage(senderAddress, ack1);
    canController.ReceiveIbMessage(senderAddress, ack2);
}

/*! \brief Ensure that the reception of a message generates a matching Ack
*
*  Rationale:
*   CanControllers must acknowledge the reception of messages since there
*   is no Network Simulator that, otherwise, would take care of Ack generation.
*/
TEST(CanControllerTest, generate_ack)
{
    using namespace std::placeholders;

    EndpointAddress controllerAddress = { 3, 8 };
    EndpointAddress senderAddress = { 4, 10 };

    MockComAdapter mockComAdapter;
    CanControllerCallbacks callbackProvider;

    CanController canController(&mockComAdapter, mockComAdapter.GetTimeProvider());
    canController.SetEndpointAddress(controllerAddress);

    CanMessage msg{};
    CanTransmitAcknowledge ack{msg.transmitId, msg.canId, msg.timestamp, CanTransmitStatus::Transmitted};

    EXPECT_CALL(mockComAdapter, SendIbMessage(controllerAddress, ack))
        .Times(1);

    canController.ReceiveIbMessage(senderAddress, msg);
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
    controller.SetEndpointAddress(controllerAddress);
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

    controller.ReceiveIbMessage(otherAddress, msg);
}
}  // anonymous namespace
