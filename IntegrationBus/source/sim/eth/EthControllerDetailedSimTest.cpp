// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "EthController.hpp"

#include <chrono>
#include <functional>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "silkit/util/functional.hpp"

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

auto AnEthMessageWith(std::chrono::nanoseconds timestamp) -> testing::Matcher<const EthernetFrameEvent&>
{
    return testing::Field(&EthernetFrameEvent::timestamp, timestamp);
}

class MockParticipant : public DummyParticipant
{
public:
    MOCK_METHOD2(SendMsg, void(const IServiceEndpoint*, const EthernetFrameEvent&));
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
        : controller(&participant, cfg, participant.GetTimeProvider())
        , controllerBusSim(&participant, cfg, participant.GetTimeProvider())
    {
        controller.SetDetailedBehavior(from_endpointAddress(busSimAddress));
        controller.SetServiceDescriptor(from_endpointAddress(controllerAddress));

        controller.AddFrameHandler(SilKit::Util::bind_method(&callbacks, &Callbacks::FrameHandler));
        controller.AddFrameTransmitHandler(SilKit::Util::bind_method(&callbacks, &Callbacks::FrameTransmitHandler));
        controller.AddBitrateChangeHandler(SilKit::Util::bind_method(&callbacks, &Callbacks::BitrateChangedHandler));
        controller.AddStateChangeHandler(SilKit::Util::bind_method(&callbacks, &Callbacks::StateChangeHandler));

        controllerBusSim.SetServiceDescriptor(from_endpointAddress(busSimAddress));
    }

protected:
    const EndpointAddress controllerAddress = {3, 8};
    const EndpointAddress busSimAddress = {7, 8};

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
    controller.ReceiveSilKitMessage(&controllerBusSim, EthernetStatus{ 0ns, EthernetState::LinkUp, 17 });
    controller.Activate();
    controller.Deactivate();
    controller.ReceiveSilKitMessage(&controllerBusSim, EthernetStatus{ 0ns, EthernetState::Inactive, 0 });
    controller.Deactivate();
}


TEST_F(EthernetControllerDetailedSimTest, send_eth_message)
{
    const auto now = 12345ns;
    EXPECT_CALL(participant, SendMsg(&controller, AnEthMessageWith(now)))
        .Times(1);

    EXPECT_CALL(participant.mockTimeProvider.mockTime, Now()).Times(0);

    EthernetFrameEvent msg{};
    msg.timestamp = now;
    controller.SendFrameEvent(msg);
}

/*! \brief SendFrame must not invoke the TimeProvider and triggers SendMsg on the participant
 */
TEST_F(EthernetControllerDetailedSimTest, send_eth_frame)
{
    EXPECT_CALL(participant, SendMsg(&controller, AnEthMessageWith(0ns))).Times(1);

    EXPECT_CALL(participant.mockTimeProvider.mockTime, Now()).Times(0);

    EthernetFrame frame{};
    controller.SendFrame(frame);
}


/*! \brief Passing an EthernetFrameEvent to an EthControllerProxy must trigger the registered callback
 */
TEST_F(EthernetControllerDetailedSimTest, trigger_callback_on_receive_message)
{
    EthernetFrameEvent msg;

    EXPECT_CALL(callbacks, FrameHandler(&controller, msg))
        .Times(1);

    controller.ReceiveSilKitMessage(&controllerBusSim, msg);
}

/*! \brief Passing an Ack to an EthControllerProxy must trigger the registered callback
 */
TEST_F(EthernetControllerDetailedSimTest, trigger_callback_on_receive_ack)
{
    EthernetFrameTransmitEvent expectedAck{ 17,  EthernetMac{}, 42ms, EthernetTransmitStatus::Transmitted };

    EXPECT_CALL(callbacks, FrameTransmitHandler(&controller, expectedAck))
        .Times(1);

    controller.ReceiveSilKitMessage(&controllerBusSim, expectedAck);
}

/*! \brief EthControllerProxy must not generate Acks
 *
 * Rationale:
 *   The EthControllerProxy is used in conjunction with a network simulator, which is
 *   responsible for Ack generation.
 */
TEST_F(EthernetControllerDetailedSimTest, must_not_generate_ack)
{
    EthernetFrameEvent msg;
    msg.transmitId = 17;

    EXPECT_CALL(participant, SendMsg(An<const IServiceEndpoint*>(), A<const EthernetFrameTransmitEvent&>()))
        .Times(0);

    controller.ReceiveSilKitMessage(&controllerBusSim, msg);
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
    controller.ReceiveSilKitMessage(&controllerBusSim, newStatus);

    newStatus.bitrate = 100;
    controller.ReceiveSilKitMessage(&controllerBusSim, newStatus);
    controller.ReceiveSilKitMessage(&controllerBusSim, newStatus);
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
    controller.ReceiveSilKitMessage(&controllerBusSim, newStatus);

    newStatus.state = EthernetState::LinkUp;
    controller.ReceiveSilKitMessage(&controllerBusSim, newStatus);
    controller.ReceiveSilKitMessage(&controllerBusSim, newStatus);

    newStatus.state = EthernetState::LinkDown;
    controller.ReceiveSilKitMessage(&controllerBusSim, newStatus);
    controller.ReceiveSilKitMessage(&controllerBusSim, newStatus);

    newStatus.state = EthernetState::Inactive;
    controller.ReceiveSilKitMessage(&controllerBusSim, newStatus);
}



} // anonymous namespace
