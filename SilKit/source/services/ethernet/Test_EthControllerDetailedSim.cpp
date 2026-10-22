// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "services/ethernet/EthController.hpp"

#include <chrono>
#include <functional>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "util/functional.hpp"

#include "core/mock/participant/MockParticipant.hpp"

#include "services/ethernet/EthDatatypeUtils.hpp"
#include "config/ParticipantConfiguration.hpp"

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
    MOCK_METHOD(void, SendMsg, (const IServiceEndpoint*, const std::string&, const WireEthernetFrameEvent&),
                (override));
    MOCK_METHOD(void, SendMsg, (const IServiceEndpoint*, const std::string&, const EthernetFrameTransmitEvent&),
                (override));
    MOCK_METHOD(void, SendMsg, (const IServiceEndpoint*, const std::string&, const EthernetStatus&), (override));
    MOCK_METHOD(void, SendMsg, (const IServiceEndpoint*, const std::string&, const EthernetSetMode&), (override));
};

class Test_EthControllerDetailedSim : public testing::Test
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
    Test_EthControllerDetailedSim()
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
    const std::string netsimName = "bussim";

    ServiceDescriptor controllerAddress{"controller", "n1", "c1", 8};
    ServiceDescriptor busSimAddress{netsimName, "n1", "c1", 8};

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
TEST_F(Test_EthControllerDetailedSim, keep_track_of_state)
{
    EthernetSetMode Activate{EthernetMode::Active};
    EthernetSetMode Deactivate{EthernetMode::Inactive};

    EXPECT_CALL(participant, SendMsg(&controller, netsimName, Activate)).Times(1);

    EXPECT_CALL(participant, SendMsg(&controller, netsimName, Deactivate)).Times(1);

    controller.Deactivate();
    controller.Activate();
    controller.ReceiveMsg(&controllerBusSim, EthernetStatus{0ns, EthernetState::LinkUp, 17});
    controller.Activate();
    controller.Deactivate();
    controller.ReceiveMsg(&controllerBusSim, EthernetStatus{0ns, EthernetState::Inactive, 0});
    controller.Deactivate();
}


TEST_F(Test_EthControllerDetailedSim, send_eth_message)
{
    EXPECT_CALL(participant, SendMsg(&controller, netsimName, AnEthMessageWith(0ns))).Times(1);

    EXPECT_CALL(participant.mockTimeProvider, Now()).Times(1);

    EthernetFrame frame{};
    controller.SendFrame(frame);
}

/*! \brief SendFrame must not invoke the TimeProvider and triggers SendMsg on the participant
 */
TEST_F(Test_EthControllerDetailedSim, send_eth_frame)
{
    EXPECT_CALL(participant, SendMsg(&controller, netsimName, AnEthMessageWith(0ns))).Times(1);

    EXPECT_CALL(participant.mockTimeProvider, Now()).Times(1);

    EthernetFrame frame{};
    controller.SendFrame(frame);
}


/*! \brief Passing an EthernetFrameEvent to an EthControllerProxy must trigger the registered callback
 */
TEST_F(Test_EthControllerDetailedSim, trigger_callback_on_receive_message)
{
    WireEthernetFrameEvent msg{};
    msg.frame.raw = SilKit::Util::SharedVector<uint8_t>{std::vector<uint8_t>(123)};
    msg.direction = TransmitDirection::RX;

    EXPECT_CALL(callbacks, FrameHandler(&controller, ToEthernetFrameEvent(msg))).Times(1);

    controller.ReceiveMsg(&controllerBusSim, msg);
}

/*! \brief Passing an Ack to an EthControllerProxy must trigger the registered callback
 */
TEST_F(Test_EthControllerDetailedSim, trigger_callback_on_receive_ack)
{
    EthernetFrameTransmitEvent expectedAck{42ms, EthernetTransmitStatus::Transmitted, reinterpret_cast<void*>(17)};

    EXPECT_CALL(callbacks, FrameTransmitHandler(&controller, expectedAck)).Times(1);

    controller.ReceiveMsg(&controllerBusSim, expectedAck);
}

/*! \brief EthControllerProxy must not generate Acks
 *
 * Rationale:
 *   The EthControllerProxy is used in conjunction with a network simulator, which is
 *   responsible for Ack generation.
 */
TEST_F(Test_EthControllerDetailedSim, must_not_generate_ack)
{
    WireEthernetFrameEvent msg{};
    msg.userContext = reinterpret_cast<void*>(17);

    EXPECT_CALL(participant, SendMsg(An<const IServiceEndpoint*>(), netsimName, A<const EthernetFrameTransmitEvent&>()))
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
TEST_F(Test_EthControllerDetailedSim, trigger_callback_on_bitrate_change)
{
    EXPECT_CALL(callbacks, BitrateChangedHandler(&controller, EthernetBitrateChangeEvent{0ns, 100})).Times(1);
    EXPECT_CALL(callbacks, BitrateChangedHandler(&controller,
                                                 Ne<EthernetBitrateChangeEvent>(EthernetBitrateChangeEvent{0ns, 100})))
        .Times(0);

    EthernetStatus newStatus = {0ns, EthernetState::Inactive, 0};
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
TEST_F(Test_EthControllerDetailedSim, trigger_callback_on_state_change)
{
    InSequence executionSequence;
    EXPECT_CALL(callbacks,
                StateChangeHandler(&controller, Ethernet::EthernetStateChangeEvent{0ns, EthernetState::LinkUp}))
        .Times(1);
    EXPECT_CALL(callbacks,
                StateChangeHandler(&controller, Ethernet::EthernetStateChangeEvent{0ns, EthernetState::LinkDown}))
        .Times(1);
    EXPECT_CALL(callbacks,
                StateChangeHandler(&controller, Ethernet::EthernetStateChangeEvent{0ns, EthernetState::Inactive}))
        .Times(1);


    EthernetStatus newStatus = {0ns, EthernetState::Inactive, 0};
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

/*! \brief The controller routes frames to a network simulator of its network once it appears, and falls back to
 *         the trivial simulation once it leaves.
 */
TEST_F(Test_EthControllerDetailedSim, network_simulator_discovery_switches_simulation_behavior)
{
    SilKit::Core::Discovery::ServiceDiscoveryHandler discoveryHandler;
    EXPECT_CALL(participant.mockServiceDiscovery, RegisterServiceDiscoveryHandler(_))
        .WillOnce(testing::SaveArg<0>(&discoveryHandler));

    EthController trivialController{&participant, cfg, participant.GetTimeProvider()};
    trivialController.SetServiceDescriptor(controllerAddress);
    trivialController.RegisterServiceDiscovery();
    ASSERT_TRUE(discoveryHandler);

    ServiceDescriptor netsimLink{netsimName, "n1", "n1", 9};
    netsimLink.SetServiceType(ServiceType::Link);
    ServiceDescriptor otherNetsimLink{netsimName, "n2", "n2", 10};
    otherNetsimLink.SetServiceType(ServiceType::Link);

    using EventType = SilKit::Core::Discovery::ServiceDiscoveryEvent::Type;

    // Only the send while the network simulator is present goes to the network simulator
    EXPECT_CALL(participant, SendMsg(&trivialController, netsimName, A<const WireEthernetFrameEvent&>())).Times(1);

    // A link on another network is ignored
    discoveryHandler(EventType::ServiceCreated, otherNetsimLink);
    trivialController.SendFrame(EthernetFrame{});

    discoveryHandler(EventType::ServiceCreated, netsimLink);
    trivialController.SendFrame(EthernetFrame{});

    // The network simulator of another network leaving does not matter
    discoveryHandler(EventType::ServiceRemoved, otherNetsimLink);
    discoveryHandler(EventType::ServiceRemoved, netsimLink);
    trivialController.SendFrame(EthernetFrame{});
}

/*! \brief In the detailed simulation, status updates and acknowledgements from anyone but the network simulator
 *         are dropped.
 */
TEST_F(Test_EthControllerDetailedSim, reject_messages_not_from_network_simulator)
{
    EthController otherController{&participant, cfg, participant.GetTimeProvider()};
    otherController.SetServiceDescriptor({"other", "n1", "c1", 8});

    EXPECT_CALL(callbacks, FrameTransmitHandler(_, _)).Times(0);
    EXPECT_CALL(callbacks, StateChangeHandler(_, _)).Times(0);
    EXPECT_CALL(callbacks, BitrateChangedHandler(_, _)).Times(0);

    controller.ReceiveMsg(&otherController,
                          EthernetFrameTransmitEvent{0ns, EthernetTransmitStatus::Transmitted, nullptr});
    controller.ReceiveMsg(&otherController, EthernetStatus{0ns, EthernetState::LinkUp, 100});
}

/*! \brief Removed handlers are no longer called, and removing unknown handlers does not throw.
 */
TEST_F(Test_EthControllerDetailedSim, removed_handlers_are_not_called)
{
    EthController detailedController{&participant, cfg, participant.GetTimeProvider()};
    detailedController.SetDetailedBehavior(busSimAddress);
    detailedController.SetServiceDescriptor(controllerAddress);

    int numCalls = 0;
    const auto frameTransmitHandlerId =
        detailedController.AddFrameTransmitHandler([&numCalls](auto*, const auto&) { ++numCalls; });
    const auto stateChangeHandlerId =
        detailedController.AddStateChangeHandler([&numCalls](auto*, const auto&) { ++numCalls; });
    const auto bitrateChangeHandlerId =
        detailedController.AddBitrateChangeHandler([&numCalls](auto*, const auto&) { ++numCalls; });

    detailedController.RemoveFrameTransmitHandler(frameTransmitHandlerId);
    detailedController.RemoveStateChangeHandler(stateChangeHandlerId);
    detailedController.RemoveBitrateChangeHandler(bitrateChangeHandlerId);

    detailedController.ReceiveMsg(&controllerBusSim,
                                  EthernetFrameTransmitEvent{0ns, EthernetTransmitStatus::Transmitted, nullptr});
    detailedController.ReceiveMsg(&controllerBusSim, EthernetStatus{0ns, EthernetState::LinkUp, 100});
    EXPECT_EQ(numCalls, 0);

    // Removing unknown handlers only warns
    EXPECT_NO_THROW(detailedController.RemoveFrameTransmitHandler(frameTransmitHandlerId));
    EXPECT_NO_THROW(detailedController.RemoveStateChangeHandler(stateChangeHandlerId));
    EXPECT_NO_THROW(detailedController.RemoveBitrateChangeHandler(bitrateChangeHandlerId));
}


} // anonymous namespace
