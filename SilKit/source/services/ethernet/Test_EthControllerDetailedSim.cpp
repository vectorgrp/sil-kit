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

#include "EthController.hpp"

#include <chrono>
#include <functional>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "functional.hpp"

#include "MockParticipant.hpp"

#include "EthDatatypeUtils.hpp"
#include "ParticipantConfiguration.hpp"

namespace {

using namespace std::chrono_literals;

using testing::Return;
using testing::A;
using testing::An;
using testing::Ne;
using testing::_;
using testing::InSequence;
using testing::NiceMock;

using namespace SilKit::Core;
using namespace SilKit::Services;
using namespace SilKit::Services::Ethernet;

using ::SilKit::Core::Tests::DummyParticipant;

auto AnEthMessageWith(std::chrono::nanoseconds timestamp) -> testing::Matcher<const WireEthernetFrameEvent&>
{
    return testing::Field(&WireEthernetFrameEvent::timestamp, timestamp);
}

class MockParticipant : public DummyParticipant
{
public:
    MOCK_METHOD2(SendMsg, void(const IServiceEndpoint*, const WireEthernetFrameEvent&));
    MOCK_METHOD2(SendMsg, void(const IServiceEndpoint*, const EthernetFrameTransmitEvent&));
    MOCK_METHOD2(SendMsg, void(const IServiceEndpoint*, const EthernetStatus&));
    MOCK_METHOD2(SendMsg, void(const IServiceEndpoint*, const EthernetSetMode&));
};

class EthernetControllerDetailedSimTest : public testing::Test
{
protected:
    struct Callbacks
    {
        MOCK_METHOD2(FrameHandler, void(Ethernet::IEthernetController*, const Ethernet::EthernetFrameEvent&));
        MOCK_METHOD2(FrameTransmitHandler, void(Ethernet::IEthernetController*, Ethernet::EthernetFrameTransmitEvent));
        MOCK_METHOD2(StateChangeHandler, void(Ethernet::IEthernetController*, Ethernet::EthernetStateChangeEvent));
        MOCK_METHOD2(BitrateChangedHandler, void(Ethernet::IEthernetController*, Ethernet::EthernetBitrateChangeEvent));
    };

protected:
    EthernetControllerDetailedSimTest()
        : controller{&participant, cfg, participant.GetTimeProvider()}
        , controllerBusSim{&participant, cfg, participant.GetTimeProvider()}
    {
        controller.SetDetailedBehavior(busSimAddress);
        controller.SetServiceDescriptor(controllerAddress);

        controller.AddFrameHandler(SilKit::Util::bind_method(&callbacks, &Callbacks::FrameHandler));
        controller.AddFrameTransmitHandler(SilKit::Util::bind_method(&callbacks, &Callbacks::FrameTransmitHandler));
        controller.AddBitrateChangeHandler(SilKit::Util::bind_method(&callbacks, &Callbacks::BitrateChangedHandler));
        controller.AddStateChangeHandler(SilKit::Util::bind_method(&callbacks, &Callbacks::StateChangeHandler));

        controllerBusSim.SetServiceDescriptor(busSimAddress);
    }

protected:
    ServiceDescriptor controllerAddress{ "controller", "n1", "c1", 8 };
    ServiceDescriptor busSimAddress{ "bussim", "n1", "c1", 8};

    MockParticipant participant;
    Callbacks callbacks;

    SilKit::Config::EthernetController cfg;
    EthController controller;
    EthController controllerBusSim;
};

/*! \brief EthControllerProxy must keep track of state and generates EthernetSetMode messages when necessary
*
* Test steps:
*   - EthControllerProxy should initially be in Inactive state, calling Deactivate() should have no effect
*   - call Activate()
*     - CHECK that EthSetMessage with EthernetMode::Active is sent
*     - deliver EthernetState::LinkUp to controller to mark it as active
*   - call Activate() again
*     - CHECK that NO EthSetMessage is sent
*   - call Deactivate()
*     - CHECK that EthSetMessage with EthernetMode::Active is sent
*     - deliver EthernetState::Inactive to controller to mark it inactive again
*   - call Deactivate() again
*     - CHECK that NO EthSetMessage is sent
*/
TEST_F(EthernetControllerDetailedSimTest, keep_track_of_state)
{
    EthernetSetMode Activate{ EthernetMode::Active };
    EthernetSetMode Deactivate{ EthernetMode::Inactive };

    EXPECT_CALL(participant, SendMsg(&controller, Activate))
        .Times(1);

    EXPECT_CALL(participant, SendMsg(&controller, Deactivate))
        .Times(1);

    controller.Deactivate();
    controller.Activate();
    controller.ReceiveMsg(&controllerBusSim, EthernetStatus{ 0ns, EthernetState::LinkUp, 17 });
    controller.Activate();
    controller.Deactivate();
    controller.ReceiveMsg(&controllerBusSim, EthernetStatus{ 0ns, EthernetState::Inactive, 0 });
    controller.Deactivate();
}


TEST_F(EthernetControllerDetailedSimTest, send_eth_message)
{
    EXPECT_CALL(participant, SendMsg(&controller, AnEthMessageWith(0ns)))
        .Times(1);

    EXPECT_CALL(participant.mockTimeProvider, Now()).Times(1);

    EthernetFrame frame{};
    controller.SendFrame(frame);
}

/*! \brief SendFrame must not invoke the TimeProvider and triggers SendMsg on the participant
 */
TEST_F(EthernetControllerDetailedSimTest, send_eth_frame)
{
    EXPECT_CALL(participant, SendMsg(&controller, AnEthMessageWith(0ns))).Times(1);

    EXPECT_CALL(participant.mockTimeProvider, Now()).Times(1);

    EthernetFrame frame{};
    controller.SendFrame(frame);
}


/*! \brief Passing an EthernetFrameEvent to an EthControllerProxy must trigger the registered callback
 */
TEST_F(EthernetControllerDetailedSimTest, trigger_callback_on_receive_message)
{
    WireEthernetFrameEvent msg{};
    msg.frame.raw = SilKit::Util::SharedVector<uint8_t>{std::vector<uint8_t>(123)};
    msg.direction = TransmitDirection::RX;

    EXPECT_CALL(callbacks, FrameHandler(&controller, ToEthernetFrameEvent(msg)))
        .Times(1);

    controller.ReceiveMsg(&controllerBusSim, msg);
}

/*! \brief Passing an Ack to an EthControllerProxy must trigger the registered callback
 */
TEST_F(EthernetControllerDetailedSimTest, trigger_callback_on_receive_ack)
{
    EthernetFrameTransmitEvent expectedAck{ 42ms, EthernetTransmitStatus::Transmitted, reinterpret_cast<void *>(17) };

    EXPECT_CALL(callbacks, FrameTransmitHandler(&controller, expectedAck))
        .Times(1);

    controller.ReceiveMsg(&controllerBusSim, expectedAck);
}

/*! \brief EthControllerProxy must not generate Acks
 *
 * Rationale:
 *   The EthControllerProxy is used in conjunction with a network simulator, which is
 *   responsible for Ack generation.
 */
TEST_F(EthernetControllerDetailedSimTest, must_not_generate_ack)
{
    WireEthernetFrameEvent msg{};
    msg.userContext = reinterpret_cast<void *>(17);

    EXPECT_CALL(participant, SendMsg(An<const IServiceEndpoint*>(), A<const EthernetFrameTransmitEvent&>()))
        .Times(0);

    controller.ReceiveMsg(&controllerBusSim, msg);
}

/*! \brief EthControllerProxy must trigger bitrate changed callbacks when necessary
*
* Rationale:
*   The callback should not be triggered upon reception of any EthernetStatus message. Only if
*   The bit rate changes.
*
* Test steps:
*   - Deliver an EthernetStatus with bitrate == 0 directly to the EthControllerProxy; this should NOT trigger the callback
*   - Deliver an EthernetStatus with bitrate != 0 directly to the EthControllerProxy; this should trigger the callback
*   - Check that the callback is executed
*   - Deliver an EthernetStatus with the same bitrate again 0 directly to the EthControllerProxy; this should NOT trigger the callback
*/
TEST_F(EthernetControllerDetailedSimTest, trigger_callback_on_bitrate_change)
{
    EXPECT_CALL(callbacks, BitrateChangedHandler(&controller, EthernetBitrateChangeEvent{ 0ns, 100 }))
        .Times(1);
    EXPECT_CALL(callbacks, BitrateChangedHandler(&controller, Ne<EthernetBitrateChangeEvent>(EthernetBitrateChangeEvent{ 0ns, 100 })))
        .Times(0);

    EthernetStatus newStatus = { 0ns, EthernetState::Inactive, 0 };
    controller.ReceiveMsg(&controllerBusSim, newStatus);

    newStatus.bitrate = 100;
    controller.ReceiveMsg(&controllerBusSim, newStatus);
    controller.ReceiveMsg(&controllerBusSim, newStatus);
}

/*! \brief EthControllerProxy must trigger state changed callbacks when necessary
*
* Rationale:
*   The callback should not be triggered upon reception of any EthernetStatus message. Only if
*   the state changes.
*
* Test steps:
*   - Deliver an EthernetStatus with Inactive directly to the EthControllerProxy; NO callback is triggered
*   - Deliver an EthernetStatus with LinkUp directly to the EthControllerProxy
*      -> this should trigger the callback
*   - Deliver an EthernetStatus with LinkUp again; NO callback is triggered
*   - Deliver an EthernetStatus with LinkDown directly to the EthControllerProxy
*      -> this should trigger the callback
*   - Deliver an EthernetStatus with LinkDown again; NO callback is triggered
*   - Deliver an EthernetStatus with Inactive directly to the EthControllerProxy
*      -> this should trigger the callback
*/
TEST_F(EthernetControllerDetailedSimTest, trigger_callback_on_state_change)
{
    InSequence executionSequence;
    EXPECT_CALL(callbacks, StateChangeHandler(&controller, Ethernet::EthernetStateChangeEvent{ 0ns, EthernetState::LinkUp }))
        .Times(1);
    EXPECT_CALL(callbacks, StateChangeHandler(&controller, Ethernet::EthernetStateChangeEvent{ 0ns, EthernetState::LinkDown }))
        .Times(1);
    EXPECT_CALL(callbacks, StateChangeHandler(&controller, Ethernet::EthernetStateChangeEvent{ 0ns, EthernetState::Inactive }))
        .Times(1);


    EthernetStatus newStatus = { 0ns, EthernetState::Inactive, 0 };
    controller.ReceiveMsg(&controllerBusSim, newStatus);

    newStatus.state = EthernetState::LinkUp;
    controller.ReceiveMsg(&controllerBusSim, newStatus);
    controller.ReceiveMsg(&controllerBusSim, newStatus);

    newStatus.state = EthernetState::LinkDown;
    controller.ReceiveMsg(&controllerBusSim, newStatus);
    controller.ReceiveMsg(&controllerBusSim, newStatus);

    newStatus.state = EthernetState::Inactive;
    controller.ReceiveMsg(&controllerBusSim, newStatus);
}



} // anonymous namespace
