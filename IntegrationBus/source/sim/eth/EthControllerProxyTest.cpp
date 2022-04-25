// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "EthControllerProxy.hpp"

#include <chrono>
#include <functional>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ib/util/functional.hpp"

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

using namespace ib::mw;
using namespace ib::sim;
using namespace ib::sim::eth;

using ::ib::mw::test::DummyParticipant;

auto AnEthMessageWith(std::chrono::nanoseconds timestamp) -> testing::Matcher<const EthernetFrameEvent&>
{
    return testing::Field(&EthernetFrameEvent::timestamp, timestamp);
}

class MockParticipant : public DummyParticipant
{
public:
    void SendIbMessage(const IIbServiceEndpoint* from, EthernetFrameEvent&& msg) override
    {
        SendIbMessage_proxy(from, msg);
    }

    MOCK_METHOD2(SendIbMessage, void(const IIbServiceEndpoint*, const EthernetFrameEvent&));
    MOCK_METHOD2(SendIbMessage_proxy, void(const IIbServiceEndpoint*, const EthernetFrameEvent&));
    MOCK_METHOD2(SendIbMessage, void(const IIbServiceEndpoint*, const EthernetFrameTransmitEvent&));
    MOCK_METHOD2(SendIbMessage, void(const IIbServiceEndpoint*, const EthernetStatus&));
    MOCK_METHOD2(SendIbMessage, void(const IIbServiceEndpoint*, const EthernetSetMode&));
};

class EthernetControllerProxyTest : public testing::Test
{
protected:
    struct Callbacks
    {
        MOCK_METHOD2(FrameHandler, void(eth::IEthernetController*, const eth::EthernetFrameEvent&));
        MOCK_METHOD2(FrameTransmitHandler, void(eth::IEthernetController*, eth::EthernetFrameTransmitEvent));
        MOCK_METHOD2(StateChangeHandler, void(eth::IEthernetController*, eth::EthernetStateChangeEvent));
        MOCK_METHOD2(BitrateChangedHandler, void(eth::IEthernetController*, eth::EthernetBitrateChangeEvent));
    };

protected:
    EthernetControllerProxyTest()
        : proxy(&participant, _config)
        , proxyFrom(&participant, _config)
    {
        proxy.SetServiceDescriptor(from_endpointAddress(proxyAddress));

        proxy.AddFrameHandler(ib::util::bind_method(&callbacks, &Callbacks::FrameHandler));
        proxy.AddFrameTransmitHandler(ib::util::bind_method(&callbacks, &Callbacks::FrameTransmitHandler));
        proxy.AddBitrateChangeHandler(ib::util::bind_method(&callbacks, &Callbacks::BitrateChangedHandler));
        proxy.AddStateChangeHandler(ib::util::bind_method(&callbacks, &Callbacks::StateChangeHandler));

        proxyFrom.SetServiceDescriptor(from_endpointAddress(controllerAddress));
    }

protected:
    const EndpointAddress proxyAddress = {3, 8};
    const EndpointAddress controllerAddress = {7, 8};
    const EndpointAddress otherControllerAddress = { 7, 125 };

    MockParticipant participant;
    Callbacks callbacks;

    ib::cfg::EthernetController _config;
    EthControllerProxy proxy;
    EthControllerProxy proxyFrom;
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
TEST_F(EthernetControllerProxyTest, keep_track_of_state)
{
    EthernetSetMode Activate{ EthernetMode::Active };
    EthernetSetMode Deactivate{ EthernetMode::Inactive };

    EXPECT_CALL(participant, SendIbMessage(&proxy, Activate))
        .Times(1);

    EXPECT_CALL(participant, SendIbMessage(&proxy, Deactivate))
        .Times(1);

    proxy.Deactivate();
    proxy.Activate();
    proxy.ReceiveIbMessage(&proxyFrom, EthernetStatus{ 0ns, EthernetState::LinkUp, 17 });
    proxy.Activate();
    proxy.Deactivate();
    proxy.ReceiveIbMessage(&proxyFrom, EthernetStatus{ 0ns, EthernetState::Inactive, 0 });
    proxy.Deactivate();
}


TEST_F(EthernetControllerProxyTest, send_eth_message)
{
    const auto now = 12345ns;
    EXPECT_CALL(participant, SendIbMessage_proxy(&proxy, AnEthMessageWith(now)))
        .Times(1);

    EXPECT_CALL(participant.mockTimeProvider.mockTime, Now()).Times(0);

    EthernetFrameEvent msg{};
    msg.timestamp = now;
    proxy.SendFrameEvent(msg);
}


/*! \brief Passing an EthernetFrameEvent to an EthControllerProxy must trigger the registered callback
 */
TEST_F(EthernetControllerProxyTest, trigger_callback_on_receive_message)
{
    EthernetFrameEvent msg;

    EXPECT_CALL(callbacks, FrameHandler(&proxy, msg))
        .Times(1);

    proxy.ReceiveIbMessage(&proxyFrom, msg);
}

/*! \brief Passing an Ack to an EthControllerProxy must trigger the registered callback
 */
TEST_F(EthernetControllerProxyTest, trigger_callback_on_receive_ack)
{
    EthernetFrameTransmitEvent expectedAck{ 17,  EthernetMac{}, 42ms, EthernetTransmitStatus::Transmitted };

    EXPECT_CALL(callbacks, FrameTransmitHandler(&proxy, expectedAck))
        .Times(1);

    proxy.ReceiveIbMessage(&proxyFrom, expectedAck);
}

/*! \brief EthControllerProxy must not generate Acks
 *
 * Rationale:
 *   The EthControllerProxy is used in conjunction with a Network Simulator, which is
 *   responsible for Ack generation.
 */
TEST_F(EthernetControllerProxyTest, must_not_generate_ack)
{
    EthernetFrameEvent msg;
    msg.transmitId = 17;

    EXPECT_CALL(participant, SendIbMessage(An<const IIbServiceEndpoint*>(), A<const EthernetFrameTransmitEvent&>()))
        .Times(0);

    proxy.ReceiveIbMessage(&proxyFrom, msg);
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
TEST_F(EthernetControllerProxyTest, trigger_callback_on_bitrate_change)
{
    EXPECT_CALL(callbacks, BitrateChangedHandler(&proxy, EthernetBitrateChangeEvent{ 0ns, 100 }))
        .Times(1);
    EXPECT_CALL(callbacks, BitrateChangedHandler(&proxy, Ne<EthernetBitrateChangeEvent>(EthernetBitrateChangeEvent{ 0ns, 100 })))
        .Times(0);

    EthernetStatus newStatus = { 0ns, EthernetState::Inactive, 0 };
    proxy.ReceiveIbMessage(&proxyFrom, newStatus);

    newStatus.bitrate = 100;
    proxy.ReceiveIbMessage(&proxyFrom, newStatus);
    proxy.ReceiveIbMessage(&proxyFrom, newStatus);
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
TEST_F(EthernetControllerProxyTest, trigger_callback_on_state_change)
{
    InSequence executionSequence;
    EXPECT_CALL(callbacks, StateChangeHandler(&proxy, eth::EthernetStateChangeEvent{ 0ns, EthernetState::LinkUp }))
        .Times(1);
    EXPECT_CALL(callbacks, StateChangeHandler(&proxy, eth::EthernetStateChangeEvent{ 0ns, EthernetState::LinkDown }))
        .Times(1);
    EXPECT_CALL(callbacks, StateChangeHandler(&proxy, eth::EthernetStateChangeEvent{ 0ns, EthernetState::Inactive }))
        .Times(1);


    EthernetStatus newStatus = { 0ns, EthernetState::Inactive, 0 };
    proxy.ReceiveIbMessage(&proxyFrom, newStatus);

    newStatus.state = EthernetState::LinkUp;
    proxy.ReceiveIbMessage(&proxyFrom, newStatus);
    proxy.ReceiveIbMessage(&proxyFrom, newStatus);

    newStatus.state = EthernetState::LinkDown;
    proxy.ReceiveIbMessage(&proxyFrom, newStatus);
    proxy.ReceiveIbMessage(&proxyFrom, newStatus);

    newStatus.state = EthernetState::Inactive;
    proxy.ReceiveIbMessage(&proxyFrom, newStatus);
}



} // anonymous namespace
