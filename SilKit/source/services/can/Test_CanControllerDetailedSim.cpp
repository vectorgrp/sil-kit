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

using namespace SilKit::Core;
using namespace SilKit::Services::Can;

using ::SilKit::Core::Tests::DummyParticipant;


class MockParticipant : public DummyParticipant
{
public:
    MOCK_METHOD2(SendMsg, void(const IServiceEndpoint*, const WireCanFrameEvent&));
    MOCK_METHOD2(SendMsg, void(const IServiceEndpoint*, const CanFrameTransmitEvent&));
    MOCK_METHOD2(SendMsg, void(const IServiceEndpoint*, const CanConfigureBaudrate&));
    MOCK_METHOD2(SendMsg, void(const IServiceEndpoint*, const CanSetControllerMode&));
};

class CanControllerCallbacks
{
public:
    MOCK_METHOD2(FrameHandler, void(ICanController*, CanFrameEvent));
    MOCK_METHOD2(StateChangeHandler, void(ICanController*, CanStateChangeEvent));
    MOCK_METHOD2(ErrorStateChangeHandler, void(ICanController*, CanErrorStateChangeEvent));
    MOCK_METHOD2(FrameTransmitHandler, void(ICanController*, CanFrameTransmitEvent));
};

TEST(Test_CanControllerDetailedSim, send_can_message)
{
    MockParticipant mockParticipant;
    CanController canController(&mockParticipant, {}, mockParticipant.GetTimeProvider());
    canController.SetDetailedBehavior({ "bussim", "n1", "c1", 8 });
    canController.SetServiceDescriptor({ "p1", "n1", "c1", 8 });

    WireCanFrameEvent testFrameEvent{};

    EXPECT_CALL(mockParticipant, SendMsg(&canController, testFrameEvent))
        .Times(1);

    canController.SendFrame(ToCanFrame(testFrameEvent.frame));
}

TEST(Test_CanControllerDetailedSim, receive_can_message)
{
    using namespace std::placeholders;

    ServiceDescriptor busSimAddress { "bussim", "n1", "c1", 8 };

    MockParticipant mockParticipant;
    CanControllerCallbacks callbackProvider;

    CanController canController(&mockParticipant, {}, mockParticipant.GetTimeProvider());
    canController.SetDetailedBehavior(busSimAddress);
    canController.SetServiceDescriptor({ "p1", "n1", "c1", 8 });
    canController.AddFrameHandler(std::bind(&CanControllerCallbacks::FrameHandler, &callbackProvider, _1, _2));

    WireCanFrameEvent testFrameEvent{};
    testFrameEvent.direction = SilKit::Services::TransmitDirection::RX;

    EXPECT_CALL(callbackProvider, FrameHandler(&canController, ToCanFrameEvent(testFrameEvent)))
        .Times(1);

    CanController canControllerFrom(&mockParticipant, {}, mockParticipant.GetTimeProvider());
    canControllerFrom.SetServiceDescriptor(busSimAddress);

    canController.ReceiveMsg(&canControllerFrom, testFrameEvent);
}

TEST(Test_CanControllerDetailedSim, start_stop_sleep_reset)
{
    MockParticipant mockParticipant;

    CanController canController(&mockParticipant, {}, mockParticipant.GetTimeProvider());
    canController.SetDetailedBehavior({ "bussim", "n1", "c1", 8 });
    canController.SetServiceDescriptor({ "p1", "n1", "c1", 8 });

    CanSetControllerMode startCommand = { {0, 0}, CanControllerState::Started };
    CanSetControllerMode stopCommand = { { 0, 0 }, CanControllerState::Stopped };
    CanSetControllerMode sleepCommand = { { 0, 0 }, CanControllerState::Sleep };
    CanSetControllerMode resetCommand = { { 1, 1 }, CanControllerState::Uninit };

    EXPECT_CALL(mockParticipant, SendMsg(&canController, startCommand))
        .Times(1);
    EXPECT_CALL(mockParticipant, SendMsg(&canController, stopCommand))
        .Times(1);
    EXPECT_CALL(mockParticipant, SendMsg(&canController, sleepCommand))
        .Times(1);
    EXPECT_CALL(mockParticipant, SendMsg(&canController, resetCommand))
        .Times(1);

    canController.Start();
    canController.Stop();
    canController.Sleep();
    canController.Reset();
}

TEST(Test_CanControllerDetailedSim, set_baudrate)
{
    MockParticipant mockParticipant;

    CanController canController(&mockParticipant, {}, mockParticipant.GetTimeProvider());
    canController.SetDetailedBehavior({ "bussim", "n1", "c1", 8 });
    canController.SetServiceDescriptor({ "p1", "n1", "c1", 8 });

    CanConfigureBaudrate baudrate1 = { 3000, 0, 0 };
    CanConfigureBaudrate baudrate2 = { 3000, 500000, 0 };

    EXPECT_CALL(mockParticipant, SendMsg(&canController, baudrate1))
        .Times(1);
    EXPECT_CALL(mockParticipant, SendMsg(&canController, baudrate2))
        .Times(1);

    canController.SetBaudRate(baudrate1.baudRate, baudrate1.fdBaudRate, baudrate1.xlBaudRate);
    canController.SetBaudRate(baudrate2.baudRate, baudrate2.fdBaudRate, baudrate2.xlBaudRate);
}

TEST(Test_CanControllerDetailedSim, receive_new_controller_state)
{
    using namespace std::placeholders;

    ServiceDescriptor busSimAddress{ "bussim", "n1", "c1", 8 };

    MockParticipant mockParticipant;

    CanController canController(&mockParticipant, {}, mockParticipant.GetTimeProvider());
    canController.SetDetailedBehavior(busSimAddress);
    canController.SetServiceDescriptor({ "p1", "n1", "c1", 8 });

    CanControllerCallbacks callbackProvider;
    canController.AddStateChangeHandler(std::bind(&CanControllerCallbacks::StateChangeHandler, &callbackProvider, _1, _2));
    canController.AddErrorStateChangeHandler(std::bind(&CanControllerCallbacks::ErrorStateChangeHandler, &callbackProvider, _1, _2));

    EXPECT_CALL(callbackProvider, StateChangeHandler(&canController, CanStateChangeEvent{ 0ns, CanControllerState::Started }))
        .Times(1);

    EXPECT_CALL(callbackProvider, ErrorStateChangeHandler(&canController, CanErrorStateChangeEvent{ 0ns, CanErrorState::ErrorActive }))
        .Times(1);

    CanControllerStatus controllerStatus{};

    CanController canControllerFrom(&mockParticipant, {}, mockParticipant.GetTimeProvider());
    canControllerFrom.SetServiceDescriptor(busSimAddress);

    // should not trigger a callback
    controllerStatus.controllerState = CanControllerState::Uninit;
    controllerStatus.errorState = CanErrorState::NotAvailable;
    canController.ReceiveMsg(&canControllerFrom, controllerStatus);

    // only stateChanged should be called
    controllerStatus.controllerState = CanControllerState::Started;
    controllerStatus.errorState = CanErrorState::NotAvailable;
    canController.ReceiveMsg(&canControllerFrom, controllerStatus);

    // only errorStateChanged should be called
    controllerStatus.controllerState = CanControllerState::Started;
    controllerStatus.errorState = CanErrorState::ErrorActive;
    canController.ReceiveMsg(&canControllerFrom, controllerStatus);
}

TEST(Test_CanControllerDetailedSim, receive_ack)
{
    using namespace std::placeholders;
    ServiceDescriptor busSimAddress{ "bussim", "n1", "c1", 8 };

    MockParticipant mockParticipant;

    CanController canController(&mockParticipant, {}, mockParticipant.GetTimeProvider());
    canController.SetDetailedBehavior(busSimAddress);
    canController.SetServiceDescriptor({ "p1", "n1", "c1", 8 });

    CanControllerCallbacks callbackProvider;
    canController.AddFrameTransmitHandler(std::bind(&CanControllerCallbacks::FrameTransmitHandler, &callbackProvider, _1, _2));

    CanFrame msg{};

    CanFrameTransmitEvent ack1{msg.canId, 0ns, CanTransmitStatus::Transmitted, (void *)1};
    CanFrameTransmitEvent ack2{msg.canId, 0ns, CanTransmitStatus::Transmitted, (void *)2};

    EXPECT_CALL(callbackProvider, FrameTransmitHandler(&canController, ack1))
        .Times(1);
    EXPECT_CALL(callbackProvider, FrameTransmitHandler(&canController, ack2))
        .Times(1);

    CanController canControllerFrom(&mockParticipant, {}, mockParticipant.GetTimeProvider());
    canControllerFrom.SetServiceDescriptor(busSimAddress);

    canController.ReceiveMsg(&canControllerFrom, ack1);
    canController.ReceiveMsg(&canControllerFrom, ack2);
}

/*! \brief Ensure that the proxy does not generate an Ack
*
*  Rationale:
*   CanTransmitAcknowledges are generated by a network simulator. Therefore,
*   the controller proxies must not generate acks on their own.
*/
TEST(Test_CanControllerDetailedSim, must_not_generate_ack)
{
    using namespace std::placeholders;

    ServiceDescriptor controllerAddress{ "P1", "N1", "C1",8 };
    ServiceDescriptor busSimAddress{ "P2", "N1", "C1", 8 };

    MockParticipant mockParticipant;
    CanControllerCallbacks callbackProvider;

    CanController canController(&mockParticipant, {}, mockParticipant.GetTimeProvider());
    canController.SetDetailedBehavior(busSimAddress);
    canController.SetServiceDescriptor(controllerAddress);

    WireCanFrameEvent msg{};
    EXPECT_CALL(mockParticipant, SendMsg(An<const IServiceEndpoint*>(), A<const CanFrameTransmitEvent&>()))
        .Times(0);

    CanController canControllerFrom(&mockParticipant, {}, mockParticipant.GetTimeProvider());
    canControllerFrom.SetServiceDescriptor(busSimAddress);

    canController.ReceiveMsg(&canControllerFrom, msg);
}

} // anonymous namespace
