// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <functional>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "MockParticipant.hpp"

#include "CanController.hpp"
#include "CanDatatypesUtils.hpp"

namespace {

using namespace std::chrono_literals;

using ::testing::Return;
using ::testing::A;
using ::testing::An;
using ::testing::_;
using ::testing::InSequence;
using ::testing::NiceMock;

using namespace ib::mw;
using namespace ib::sim::can;

using ::ib::mw::test::DummyParticipant;


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

TEST(CanControllerDetailedSimTest, send_can_message)
{
    EndpointAddress controllerAddress = { 3, 8 };
    EndpointAddress busSimAddress = {19, 8};

    MockParticipant mockParticipant;
    CanController canController(&mockParticipant, {}, mockParticipant.GetTimeProvider());
    canController.SetDetailedBehavior(from_endpointAddress(busSimAddress));
    canController.SetServiceDescriptor(from_endpointAddress(controllerAddress));

    CanFrameEvent testFrameEvent{};
    testFrameEvent.transmitId = 1;

    EXPECT_CALL(mockParticipant, SendIbMessage(&canController, testFrameEvent))
        .Times(1);

    canController.SendFrame(testFrameEvent.frame);
}

TEST(CanControllerDetailedSimTest, receive_can_message)
{
    using namespace std::placeholders;

    EndpointAddress controllerAddress = { 3, 8 };
    EndpointAddress busSimAddress = { 19, 8 };

    MockParticipant mockParticipant;
    CanControllerCallbacks callbackProvider;

    CanController canController(&mockParticipant, {}, mockParticipant.GetTimeProvider());
    canController.SetDetailedBehavior(from_endpointAddress(busSimAddress));
    canController.SetServiceDescriptor(from_endpointAddress(controllerAddress));
    canController.AddFrameHandler(std::bind(&CanControllerCallbacks::FrameHandler, &callbackProvider, _1, _2));

    CanFrameEvent testFrameEvent{};
    testFrameEvent.direction = ib::sim::TransmitDirection::RX;

    EXPECT_CALL(callbackProvider, FrameHandler(&canController, testFrameEvent))
        .Times(1);

    CanController canControllerFrom(&mockParticipant, {}, mockParticipant.GetTimeProvider());
    canControllerFrom.SetServiceDescriptor(from_endpointAddress(busSimAddress));

    canController.ReceiveIbMessage(&canControllerFrom, testFrameEvent);
}

TEST(CanControllerDetailedSimTest, start_stop_sleep_reset)
{
    EndpointAddress controllerAddress = { 3, 8 };
    EndpointAddress busSimAddress = {19, 8};

    MockParticipant mockParticipant;

    CanController canController(&mockParticipant, {}, mockParticipant.GetTimeProvider());
    canController.SetDetailedBehavior(from_endpointAddress(busSimAddress));
    canController.SetServiceDescriptor(from_endpointAddress(controllerAddress));

    CanSetControllerMode startCommand = { {0, 0}, CanControllerState::Started };
    CanSetControllerMode stopCommand = { { 0, 0 }, CanControllerState::Stopped };
    CanSetControllerMode sleepCommand = { { 0, 0 }, CanControllerState::Sleep };
    CanSetControllerMode resetCommand = { { 1, 1 }, CanControllerState::Uninit };

    EXPECT_CALL(mockParticipant, SendIbMessage(&canController, startCommand))
        .Times(1);
    EXPECT_CALL(mockParticipant, SendIbMessage(&canController, stopCommand))
        .Times(1);
    EXPECT_CALL(mockParticipant, SendIbMessage(&canController, sleepCommand))
        .Times(1);
    EXPECT_CALL(mockParticipant, SendIbMessage(&canController, resetCommand))
        .Times(1);

    canController.Start();
    canController.Stop();
    canController.Sleep();
    canController.Reset();
}

TEST(CanControllerDetailedSimTest, set_baudrate)
{
    EndpointAddress controllerAddress = {3, 8};
    EndpointAddress busSimAddress = {19, 8};

    MockParticipant mockParticipant;

    CanController canController(&mockParticipant, {}, mockParticipant.GetTimeProvider());
    canController.SetDetailedBehavior(from_endpointAddress(busSimAddress));
    canController.SetServiceDescriptor(from_endpointAddress(controllerAddress));

    CanConfigureBaudrate baudrate1 = { 3000, 0 };
    CanConfigureBaudrate baudrate2 = { 3000, 500000 };

    EXPECT_CALL(mockParticipant, SendIbMessage(&canController, baudrate1))
        .Times(1);
    EXPECT_CALL(mockParticipant, SendIbMessage(&canController, baudrate2))
        .Times(1);

    canController.SetBaudRate(baudrate1.baudRate, baudrate1.fdBaudRate);
    canController.SetBaudRate(baudrate2.baudRate, baudrate2.fdBaudRate);
}

TEST(CanControllerDetailedSimTest, receive_new_controller_state)
{
    using namespace std::placeholders;

    EndpointAddress controllerAddress = { 3, 8 };
    EndpointAddress busSimAddress = {19, 8};

    MockParticipant mockParticipant;

    CanController canController(&mockParticipant, {}, mockParticipant.GetTimeProvider());
    canController.SetDetailedBehavior(from_endpointAddress(busSimAddress));
    canController.SetServiceDescriptor(from_endpointAddress(controllerAddress));

    CanControllerCallbacks callbackProvider;
    canController.AddStateChangeHandler(std::bind(&CanControllerCallbacks::StateChangeHandler, &callbackProvider, _1, _2));
    canController.AddErrorStateChangeHandler(std::bind(&CanControllerCallbacks::ErrorStateChangeHandler, &callbackProvider, _1, _2));

    EXPECT_CALL(callbackProvider, StateChangeHandler(&canController, CanStateChangeEvent{ 0ns, CanControllerState::Started }))
        .Times(1);

    EXPECT_CALL(callbackProvider, ErrorStateChangeHandler(&canController, CanErrorStateChangeEvent{ 0ns, CanErrorState::ErrorActive }))
        .Times(1);

    CanControllerStatus controllerStatus{};

    CanController canControllerFrom(&mockParticipant, {}, mockParticipant.GetTimeProvider());
    canControllerFrom.SetServiceDescriptor(from_endpointAddress(busSimAddress));

    // should not trigger a callback
    controllerStatus.controllerState = CanControllerState::Uninit;
    controllerStatus.errorState = CanErrorState::NotAvailable;
    canController.ReceiveIbMessage(&canControllerFrom, controllerStatus);

    // only stateChanged should be called
    controllerStatus.controllerState = CanControllerState::Started;
    controllerStatus.errorState = CanErrorState::NotAvailable;
    canController.ReceiveIbMessage(&canControllerFrom, controllerStatus);

    // only errorStateChanged should be called
    controllerStatus.controllerState = CanControllerState::Started;
    controllerStatus.errorState = CanErrorState::ErrorActive;
    canController.ReceiveIbMessage(&canControllerFrom, controllerStatus);
}

TEST(CanControllerDetailedSimTest, receive_ack)
{
    using namespace std::placeholders;

    EndpointAddress controllerAddress = { 3, 8 };
    EndpointAddress busSimAddress = {19, 8};

    MockParticipant mockParticipant;

    CanController canController(&mockParticipant, {}, mockParticipant.GetTimeProvider());
    canController.SetDetailedBehavior(from_endpointAddress(busSimAddress));
    canController.SetServiceDescriptor(from_endpointAddress(controllerAddress));

    CanControllerCallbacks callbackProvider;
    canController.AddFrameTransmitHandler(std::bind(&CanControllerCallbacks::FrameTransmitHandler, &callbackProvider, _1, _2));

    CanFrame msg;
    auto txId1 = canController.SendFrame(msg);
    auto txId2 = canController.SendFrame(msg);
    ASSERT_NE(txId1, txId2);

    CanFrameTransmitEvent ack1{txId1, msg.canId, 0ns, CanTransmitStatus::Transmitted, nullptr};
    CanFrameTransmitEvent ack2{txId2, msg.canId, 0ns, CanTransmitStatus::Transmitted, nullptr};

    EXPECT_CALL(callbackProvider, FrameTransmitHandler(&canController, ack1))
        .Times(1);
    EXPECT_CALL(callbackProvider, FrameTransmitHandler(&canController, ack2))
        .Times(1);

    CanController canControllerFrom(&mockParticipant, {}, mockParticipant.GetTimeProvider());
    canControllerFrom.SetServiceDescriptor(from_endpointAddress(busSimAddress));

    canController.ReceiveIbMessage(&canControllerFrom, ack1);
    canController.ReceiveIbMessage(&canControllerFrom, ack2);
}

/*! \brief Ensure that the proxy does not generate an Ack
*
*  Rationale:
*   CanTransmitAcknowledges are generated by the Network Simulator. Therefore,
*   the controller proxies must not generate acks on their own.
*/
TEST(CanControllerDetailedSimTest, must_not_generate_ack)
{
    using namespace std::placeholders;

    EndpointAddress controllerAddress = { 3, 8 };
    EndpointAddress busSimAddress = {19, 8};

    MockParticipant mockParticipant;
    CanControllerCallbacks callbackProvider;

    CanController canController(&mockParticipant, {}, mockParticipant.GetTimeProvider());
    canController.SetDetailedBehavior(from_endpointAddress(busSimAddress));
    canController.SetServiceDescriptor(from_endpointAddress(controllerAddress));

    CanFrameEvent msg{};
    EXPECT_CALL(mockParticipant, SendIbMessage(An<const IIbServiceEndpoint*>(), A<const CanFrameTransmitEvent&>()))
        .Times(0);

    CanController canControllerFrom(&mockParticipant, {}, mockParticipant.GetTimeProvider());
    canControllerFrom.SetServiceDescriptor(from_endpointAddress(busSimAddress));

    canController.ReceiveIbMessage(&canControllerFrom, msg);
}

} // anonymous namespace