// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <functional>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ib/sim/can/string_utils.hpp"

#include "MockParticipant.hpp"
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

using ib::mw::test::DummyParticipant;

MATCHER_P(CanTransmitAckWithouthTransmitIdMatcher, truthAck, "") {
    *result_listener << "matches CanTransmitAcks without checking the transmit id";
    auto frame1 = truthAck;
    auto frame2 = arg;
    return frame1.canId == frame2.canId && frame1.status == frame2.status && frame1.timestamp == frame2.timestamp;
}

class MockParticipant : public DummyParticipant
{
public:
    MOCK_METHOD2(SendIbMessage, void(const IIbServiceEndpoint*, const CanFrameEvent&));
    MOCK_METHOD2(SendIbMessage, void(const IIbServiceEndpoint*, const CanFrameTransmitEvent&));
    MOCK_METHOD2(SendIbMessage, void(const IIbServiceEndpoint*, const CanConfigureBaudrate&));
    MOCK_METHOD2(SendIbMessage, void(const IIbServiceEndpoint*, const CanSetControllerMode&));
};

class CanControllerCallbacks
{
public:
    MOCK_METHOD2(FrameHandler, void(ICanController*, CanFrameEvent));
    MOCK_METHOD2(StateChangeHandler, void(ICanController*, CanStateChangeEvent));
    MOCK_METHOD2(ErrorStateChangeHandler, void(ICanController*, CanErrorStateChangeEvent));
    MOCK_METHOD2(FrameTransmitHandler, void(ICanController*, CanFrameTransmitEvent));
};

class CanControllerTest : public testing::Test
{
public:
    CanControllerTest() {};
};

TEST(CanControllerTest, send_can_message)
{
    MockParticipant mockParticipant;

    ServiceDescriptor senderDescriptor{};
    senderDescriptor.SetParticipantName("canControllerPlaceholder");
    senderDescriptor.SetServiceId(17);
    ib::cfg::CanController cfg;

    CanController canController(&mockParticipant, cfg, mockParticipant.GetTimeProvider());
    canController.SetServiceDescriptor(senderDescriptor);

    CanFrameEvent testFrameEvent{};
    testFrameEvent.transmitId = 1;
    testFrameEvent.timestamp = 0ns;
    testFrameEvent.userContext = 0;

    EXPECT_CALL(mockParticipant, SendIbMessage(&canController, testFrameEvent))
        .Times(1);
    EXPECT_CALL(mockParticipant.mockTimeProvider.mockTime, Now())
        .Times(1);

    canController.SendFrame(testFrameEvent.frame);
}

TEST(CanControllerTest, receive_can_message)
{
    using namespace std::placeholders;

    ServiceDescriptor senderDescriptor{};
    senderDescriptor.SetParticipantName("canControllerPlaceholder");
    senderDescriptor.SetServiceId(17);

    MockParticipant mockParticipant;
    CanControllerCallbacks callbackProvider;
    ib::cfg::CanController cfg;

    CanController canController(&mockParticipant, cfg, mockParticipant.GetTimeProvider());
    canController.AddFrameHandler(std::bind(&CanControllerCallbacks::FrameHandler, &callbackProvider, _1, _2));

    CanFrameEvent testFrameEvent{};
    testFrameEvent.frame.canId = 16;
    testFrameEvent.transmitId = 321;
    testFrameEvent.direction = ib::sim::TransmitDirection::RX;
    testFrameEvent.userContext = (void*)1234;

    EXPECT_CALL(callbackProvider, FrameHandler(&canController, testFrameEvent))
        .Times(1);
    EXPECT_CALL(mockParticipant.mockTimeProvider.mockTime, Now()).Times(1);

    CanController canControllerPlaceholder(&mockParticipant, cfg, mockParticipant.GetTimeProvider());
    canControllerPlaceholder.SetServiceDescriptor(senderDescriptor);
    canController.ReceiveIbMessage(&canControllerPlaceholder, testFrameEvent);
}

TEST(CanControllerTest, receive_can_message_rx_filter1)
{
    using namespace std::placeholders;

    ServiceDescriptor senderDescriptor{};
    senderDescriptor.SetParticipantName("canControllerPlaceholder");
    senderDescriptor.SetServiceId(17);

    MockParticipant mockParticipant;
    CanControllerCallbacks callbackProvider;
    ib::cfg::CanController cfg;
    CanController canController(&mockParticipant, cfg, mockParticipant.GetTimeProvider());
    canController.AddFrameHandler(std::bind(&CanControllerCallbacks::FrameHandler, &callbackProvider, _1, _2), (ib::sim::DirectionMask)ib::sim::TransmitDirection::RX);

    CanFrameEvent testFrameEvent{};
    testFrameEvent.frame.canId = 16;
    testFrameEvent.direction = ib::sim::TransmitDirection::RX;
    testFrameEvent.userContext = (void*)1234;

    EXPECT_CALL(callbackProvider, FrameHandler(&canController, testFrameEvent))
        .Times(1);
    EXPECT_CALL(mockParticipant.mockTimeProvider.mockTime, Now()).Times(1);

    CanController canControllerPlaceholder(&mockParticipant, cfg, mockParticipant.GetTimeProvider());
    canControllerPlaceholder.SetServiceDescriptor(senderDescriptor);
    canController.ReceiveIbMessage(&canControllerPlaceholder, testFrameEvent);
}


TEST(CanControllerTest, receive_can_message_rx_filter2)
{
    using namespace std::placeholders;

    ServiceDescriptor senderDescriptor{};
    senderDescriptor.SetParticipantName("canControllerPlaceholder");
    senderDescriptor.SetServiceId(17);

    MockParticipant mockParticipant;
    CanControllerCallbacks callbackProvider;

    ib::cfg::CanController cfg;
    CanController canController(&mockParticipant, cfg, mockParticipant.GetTimeProvider());
    canController.AddFrameHandler(std::bind(&CanControllerCallbacks::FrameHandler, &callbackProvider, _1, _2), (ib::sim::DirectionMask)ib::sim::TransmitDirection::TX);

    CanFrameEvent testFrameEvent{};
    testFrameEvent.frame.canId = 16;
    testFrameEvent.direction = ib::sim::TransmitDirection::RX;

    EXPECT_CALL(callbackProvider, FrameHandler(&canController, testFrameEvent))
        .Times(0);
    EXPECT_CALL(mockParticipant.mockTimeProvider.mockTime, Now()).Times(1);

    CanController canControllerPlaceholder(&mockParticipant, cfg, mockParticipant.GetTimeProvider());
    canControllerPlaceholder.SetServiceDescriptor(senderDescriptor);
    canController.ReceiveIbMessage(&canControllerPlaceholder, testFrameEvent);
}

TEST(CanControllerTest, receive_can_message_tx_filter1)
{
    using namespace std::placeholders;

    EndpointAddress senderAddress{ 17, 4 };

    MockParticipant mockParticipant;
    CanControllerCallbacks callbackProvider;

    ib::cfg::CanController cfg;
    
    CanController canController(&mockParticipant, cfg, mockParticipant.GetTimeProvider());
    canController.AddFrameHandler(std::bind(&CanControllerCallbacks::FrameHandler, &callbackProvider, _1, _2), (ib::sim::DirectionMask)ib::sim::TransmitDirection::TX);

    CanFrameEvent testFrameEvent{};
    testFrameEvent.transmitId = 1;
    testFrameEvent.frame.canId = 16;
    testFrameEvent.direction = ib::sim::TransmitDirection::TX;

    EXPECT_CALL(callbackProvider, FrameHandler(&canController, testFrameEvent))
        .Times(1);
    EXPECT_CALL(mockParticipant.mockTimeProvider.mockTime, Now()).Times(1);

    CanController canControllerPlaceholder(&mockParticipant, cfg, mockParticipant.GetTimeProvider());
    canControllerPlaceholder.SetServiceDescriptor(from_endpointAddress(senderAddress));
    canController.SendFrame(testFrameEvent.frame);
}


TEST(CanControllerTest, receive_can_message_tx_filter2)
{
    using namespace std::placeholders;

    EndpointAddress senderAddress{ 17, 4 };

    MockParticipant mockParticipant;
    CanControllerCallbacks callbackProvider;

    ib::cfg::CanController cfg;
    CanController canController(&mockParticipant, cfg, mockParticipant.GetTimeProvider());
    canController.AddFrameHandler(std::bind(&CanControllerCallbacks::FrameHandler, &callbackProvider, _1, _2), (ib::sim::DirectionMask)ib::sim::TransmitDirection::RX);

    CanFrameEvent testFrameEvent{};
    testFrameEvent.frame.canId = 16;
    testFrameEvent.direction = ib::sim::TransmitDirection::TX;

    EXPECT_CALL(callbackProvider, FrameHandler(&canController, testFrameEvent))
        .Times(0);
    EXPECT_CALL(mockParticipant.mockTimeProvider.mockTime, Now()).Times(1);

    CanController canControllerPlaceholder(&mockParticipant, cfg, mockParticipant.GetTimeProvider());
    canControllerPlaceholder.SetServiceDescriptor(from_endpointAddress(senderAddress));
    canController.SendFrame(testFrameEvent.frame);
}


/*! \brief Ensure that start, stop, sleep, and reset have no effect
 *
 *  Rationale:
 *   ControllerModes are not supported by the CanController. This feature requires
 *   the usage of a proper CanBusSimulator and CanControllerProxies.
 */
TEST(CanControllerTest, start_stop_sleep_reset)
{
    MockParticipant mockParticipant;
    ib::cfg::CanController cfg;

    CanController canController(&mockParticipant, cfg, mockParticipant.GetTimeProvider());

    EXPECT_CALL(mockParticipant, SendIbMessage(A<const IIbServiceEndpoint*>(), A<const CanSetControllerMode&>()))
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
    MockParticipant mockParticipant;
    ib::cfg::CanController cfg;

    CanController canController(&mockParticipant, cfg, mockParticipant.GetTimeProvider());

    EXPECT_CALL(mockParticipant, SendIbMessage(An<const IIbServiceEndpoint*>(), A<const CanConfigureBaudrate&>()))
        .Times(0);

    canController.SetBaudRate(3000, 500000);
}

TEST(CanControllerTest, receive_ack)
{
    using namespace std::placeholders;

    EndpointAddress controllerAddress = { 3, 8 };

    MockParticipant mockParticipant;
    CanControllerCallbacks callbackProvider;
    ib::cfg::CanController cfg;

    CanController canController(&mockParticipant, cfg, mockParticipant.GetTimeProvider());
    canController.SetServiceDescriptor(from_endpointAddress(controllerAddress));
    canController.AddFrameTransmitHandler(std::bind(&CanControllerCallbacks::FrameTransmitHandler, &callbackProvider, _1, _2));

    CanFrame msg{};
    CanFrameTransmitEvent expectedAck{ 0, msg.canId, 0ns, CanTransmitStatus::Transmitted, nullptr };

    EXPECT_CALL(callbackProvider, FrameTransmitHandler(&canController, CanTransmitAckWithouthTransmitIdMatcher(expectedAck)))
        .Times(2);
    auto txId1 = canController.SendFrame(msg);
    auto txId2 = canController.SendFrame(msg);

    ASSERT_NE(txId1, txId2);
}


TEST(CanControllerTest, DISABLED_cancontroller_uses_tracing)
{
    using namespace ib::extensions;

    ib::test::MockTraceSink traceSink;
    test::DummyParticipant participant;
    ib::cfg::CanController cfg;
    const std::chrono::nanoseconds now = 1337ns;
    const EndpointAddress controllerAddress = {1,2};
    const EndpointAddress otherAddress = {2,2};

    ON_CALL(participant.mockTimeProvider.mockTime, Now())
        .WillByDefault(testing::Return(now));


    auto controller = CanController(&participant, cfg, participant.GetTimeProvider());
    controller.SetServiceDescriptor(from_endpointAddress(controllerAddress));
    controller.AddSink(&traceSink);


    CanFrameEvent testFrameEvent{};

    //Send direction
    EXPECT_CALL(participant.mockTimeProvider.mockTime, Now())
        .Times(1);
    EXPECT_CALL(traceSink,
        Trace(ib::sim::TransmitDirection::TX, controllerAddress, now, testFrameEvent.frame))
        .Times(1);
    controller.SendFrame(testFrameEvent.frame);

    // Receive direction
    EXPECT_CALL(participant.mockTimeProvider.mockTime, Now())
        .Times(1);
    EXPECT_CALL(traceSink,
        Trace(ib::sim::TransmitDirection::RX, controllerAddress, now, testFrameEvent.frame))
        .Times(1);

    CanController canControllerPlaceholder(&participant, cfg, participant.GetTimeProvider());
    canControllerPlaceholder.SetServiceDescriptor(from_endpointAddress(otherAddress));
    controller.ReceiveIbMessage(&canControllerPlaceholder, testFrameEvent);
}
}  // anonymous namespace
