// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <functional>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ib/mw/string_utils.hpp"

#include "MockComAdapter.hpp"

#include "CanControllerProxy.hpp"
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

using ::ib::mw::test::DummyComAdapter;

class MockComAdapter : public DummyComAdapter
{
public:
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const CanMessage&));
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const CanTransmitAcknowledge&));
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const CanConfigureBaudrate&));
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const CanSetControllerMode&));
};

class CanControllerProxyCallbacks
{
public:
    MOCK_METHOD2(ReceiveMessage, void(ICanController*, CanMessage));
    MOCK_METHOD2(StateChanged, void(ICanController*, CanControllerState));
    MOCK_METHOD2(ErrorStateChanged, void(ICanController*, CanErrorState));
    MOCK_METHOD2(ReceiveAck, void(ICanController*, CanTransmitAcknowledge));
};

TEST(CanControllerProxyTest, send_can_message)
{
    EndpointAddress controllerAddress = { 3, 8 };

    MockComAdapter mockComAdapter;
    CanControllerProxy canController(&mockComAdapter);
    canController.SetEndpointAddress(controllerAddress);

    CanMessage msg;
    msg.transmitId = 1;

    EXPECT_CALL(mockComAdapter, SendIbMessage(controllerAddress, msg))
        .Times(1);

    canController.SendMessage(msg);
}

TEST(CanControllerProxyTest, receive_can_message)
{
    using namespace std::placeholders;

    EndpointAddress controllerAddress = { 3, 8 };
    EndpointAddress busSimAddress = { 19 ,8 };

    MockComAdapter mockComAdapter;
    CanControllerProxyCallbacks callbackProvider;

    CanControllerProxy canController(&mockComAdapter);
    canController.SetEndpointAddress(controllerAddress);
    canController.RegisterReceiveMessageHandler(std::bind(&CanControllerProxyCallbacks::ReceiveMessage, &callbackProvider, _1, _2));

    CanMessage msg;

    EXPECT_CALL(callbackProvider, ReceiveMessage(&canController, msg))
        .Times(1);

    canController.ReceiveIbMessage(busSimAddress, msg);
}

TEST(CanControllerProxyTest, start_stop_sleep_reset)
{
    EndpointAddress controllerAddress = { 3, 8 };

    MockComAdapter mockComAdapter;

    CanControllerProxy canController(&mockComAdapter);
    canController.SetEndpointAddress(controllerAddress);

    CanSetControllerMode startCommand = { {0, 0}, CanControllerState::Started };
    CanSetControllerMode stopCommand = { { 0, 0 }, CanControllerState::Stopped };
    CanSetControllerMode sleepCommand = { { 0, 0 }, CanControllerState::Sleep };
    CanSetControllerMode resetCommand = { { 1, 1 }, CanControllerState::Uninit };

    EXPECT_CALL(mockComAdapter, SendIbMessage(controllerAddress, startCommand))
        .Times(1);
    EXPECT_CALL(mockComAdapter, SendIbMessage(controllerAddress, stopCommand))
        .Times(1);
    EXPECT_CALL(mockComAdapter, SendIbMessage(controllerAddress, sleepCommand))
        .Times(1);
    EXPECT_CALL(mockComAdapter, SendIbMessage(controllerAddress, resetCommand))
        .Times(1);

    canController.Start();
    canController.Stop();
    canController.Sleep();
    canController.Reset();
}

TEST(CanControllerProxyTest, set_baudrate)
{
    EndpointAddress controllerAddress = { 3, 8 };

    MockComAdapter mockComAdapter;

    CanControllerProxy canController(&mockComAdapter);
    canController.SetEndpointAddress(controllerAddress);

    CanConfigureBaudrate baudrate1 = { 3000, 0 };
    CanConfigureBaudrate baudrate2 = { 3000, 500000 };

    EXPECT_CALL(mockComAdapter, SendIbMessage(controllerAddress, baudrate1))
        .Times(1);
    EXPECT_CALL(mockComAdapter, SendIbMessage(controllerAddress, baudrate2))
        .Times(1);

    canController.SetBaudRate(baudrate1.baudRate, baudrate1.fdBaudRate);
    canController.SetBaudRate(baudrate2.baudRate, baudrate2.fdBaudRate);
}

TEST(CanControllerProxyTest, receive_new_controller_state)
{
    using namespace std::placeholders;

    EndpointAddress controllerAddress = { 3, 8 };
    EndpointAddress busSimAddress = { 19 ,8 };

    MockComAdapter mockComAdapter;

    CanControllerProxy canController(&mockComAdapter);
    canController.SetEndpointAddress(controllerAddress);

    CanControllerProxyCallbacks callbackProvider;
    canController.RegisterStateChangedHandler(std::bind(&CanControllerProxyCallbacks::StateChanged, &callbackProvider, _1, _2));
    canController.RegisterErrorStateChangedHandler(std::bind(&CanControllerProxyCallbacks::ErrorStateChanged, &callbackProvider, _1, _2));

    EXPECT_CALL(callbackProvider, StateChanged(&canController, CanControllerState::Started))
        .Times(1);

    EXPECT_CALL(callbackProvider, ErrorStateChanged(&canController, CanErrorState::ErrorActive))
        .Times(1);

    CanControllerStatus controllerStatus;

    // should not trigger a callback
    controllerStatus.controllerState = CanControllerState::Uninit;
    controllerStatus.errorState = CanErrorState::NotAvailable;
    canController.ReceiveIbMessage(busSimAddress, controllerStatus);

    // only stateChanged should be called
    controllerStatus.controllerState = CanControllerState::Started;
    controllerStatus.errorState = CanErrorState::NotAvailable;
    canController.ReceiveIbMessage(busSimAddress, controllerStatus);

    // only errorStateChanged should be called
    controllerStatus.controllerState = CanControllerState::Started;
    controllerStatus.errorState = CanErrorState::ErrorActive;
    canController.ReceiveIbMessage(busSimAddress, controllerStatus);
}

TEST(CanControllerProxyTest, receive_ack)
{
    using namespace std::placeholders;

    EndpointAddress controllerAddress = { 3, 8 };
    EndpointAddress busSimAddress = { 19 ,8 };

    MockComAdapter mockComAdapter;

    CanControllerProxy canController(&mockComAdapter);
    canController.SetEndpointAddress(controllerAddress);

    CanControllerProxyCallbacks callbackProvider;
    canController.RegisterTransmitStatusHandler(std::bind(&CanControllerProxyCallbacks::ReceiveAck, &callbackProvider, _1, _2));

    CanMessage msg;
    auto txId1 = canController.SendMessage(msg);
    auto txId2 = canController.SendMessage(msg);
    ASSERT_NE(txId1, txId2);

    CanTransmitAcknowledge ack1{txId1, 0ns, CanTransmitStatus::Transmitted};
    CanTransmitAcknowledge ack2{txId2, 0ns, CanTransmitStatus::Transmitted};

    EXPECT_CALL(callbackProvider, ReceiveAck(&canController, ack1))
        .Times(1);
    EXPECT_CALL(callbackProvider, ReceiveAck(&canController, ack2))
        .Times(1);

    canController.ReceiveIbMessage(busSimAddress, ack1);
    canController.ReceiveIbMessage(busSimAddress, ack2);
}

/*! \brief Ensure that the proxy does not generate an Ack
*
*  Rationale:
*   CanTransmitAcknowledges are generated by the Network Simulator. Therefore,
*   the controller proxies must not generate acks on their own.
*/
TEST(CanControllerProxyTest, must_not_generate_ack)
{
    using namespace std::placeholders;

    EndpointAddress controllerAddress = { 3, 8 };
    EndpointAddress busSimAddress = { 19 ,8 };

    MockComAdapter mockComAdapter;
    CanControllerProxyCallbacks callbackProvider;

    CanControllerProxy canController(&mockComAdapter);
    canController.SetEndpointAddress(controllerAddress);

    CanMessage msg;

    EXPECT_CALL(mockComAdapter, SendIbMessage(An<EndpointAddress>(), A<const CanTransmitAcknowledge&>()))
        .Times(0);

    canController.ReceiveIbMessage(busSimAddress, msg);
}

} // anonymous namespace
