// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "EthControllerProxy.hpp"

#include <chrono>
#include <functional>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ib/mw/string_utils.hpp"
#include "ib/util/functional.hpp"

#include "MockComAdapter.hpp"

#include "EthDatatypeUtils.hpp"

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

using ::ib::mw::test::DummyComAdapter;

class MockComAdapter : public DummyComAdapter
{
public:
    void SendIbMessage(EndpointAddress from, EthMessage&& msg) override
    {
        SendIbMessage_proxy(from, msg);
    }

    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const EthMessage&));
    MOCK_METHOD2(SendIbMessage_proxy, void(EndpointAddress, const EthMessage&));
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const EthTransmitAcknowledge&));
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const EthStatus&));
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const EthSetMode&));
};

class EthernetControllerProxyTest : public testing::Test
{
protected:
    struct Callbacks
    {
        MOCK_METHOD2(ReceiveMessage, void(eth::IEthController*, const eth::EthMessage&));
        MOCK_METHOD2(MessageAck, void(eth::IEthController*, eth::EthTransmitAcknowledge));
        MOCK_METHOD2(StateChanged, void(eth::IEthController*, eth::EthState));
        MOCK_METHOD2(BitRateChanged, void(eth::IEthController*, uint32_t));
    };

protected:
    EthernetControllerProxyTest()
        : proxy(&comAdapter)
    {
        proxy.SetEndpointAddress(proxyAddress);

        proxy.RegisterReceiveMessageHandler(ib::util::bind_method(&callbacks, &Callbacks::ReceiveMessage));
        proxy.RegisterMessageAckHandler(ib::util::bind_method(&callbacks, &Callbacks::MessageAck));
        proxy.RegisterBitRateChangedHandler(ib::util::bind_method(&callbacks, &Callbacks::BitRateChanged));
        proxy.RegisterStateChangedHandler(ib::util::bind_method(&callbacks, &Callbacks::StateChanged));
    }

protected:
    const EndpointAddress proxyAddress = {3, 8};
    const EndpointAddress controllerAddress = {7, 8};
    const EndpointAddress otherControllerAddress = { 7, 125 };

    MockComAdapter comAdapter;
    Callbacks callbacks;

    EthControllerProxy proxy;
};

/*! \brief EthControllerProxy must keep track of state and generates EthSetMode messages when necessary
*
* Test steps:
*   - EthControllerProxy should initially be in Inactive state, calling Deactivate() should have no effect
*   - call Activate()
*     - CHECK that EthSetMessage with EthMode::Active is sent
*     - deliver EthState::LinkUp to controller to mark it as active
*   - call Activate() again
*     - CHECK that NO EthSetMessage is sent
*   - call Deactivate()
*     - CHECK that EthSetMessage with EthMode::Active is sent
*     - deliver EthState::Inactive to controller to mark it inactive again
*   - call Deactivate() again
*     - CHECK that NO EthSetMessage is sent
*/
TEST_F(EthernetControllerProxyTest, keep_track_of_state)
{
    EthSetMode Activate{ EthMode::Active };
    EthSetMode Deactivate{ EthMode::Inactive };

    EXPECT_CALL(comAdapter, SendIbMessage(proxyAddress, Activate))
        .Times(1);

    EXPECT_CALL(comAdapter, SendIbMessage(proxyAddress, Deactivate))
        .Times(1);

    proxy.Deactivate();
    proxy.Activate();
    proxy.ReceiveIbMessage(controllerAddress, EthStatus{ 0ns, EthState::LinkUp, 17 });
    proxy.Activate();
    proxy.Deactivate();
    proxy.ReceiveIbMessage(controllerAddress, EthStatus{ 0ns, EthState::Inactive, 0 });
    proxy.Deactivate();
}


TEST_F(EthernetControllerProxyTest, send_eth_message)
{
    EXPECT_CALL(comAdapter, SendIbMessage_proxy(proxyAddress, A<const EthMessage&>()))
        .Times(1);

    EthMessage msg;
    proxy.SendMessage(msg);
}


/*! \brief Passing an EthMessage to an EthControllerProxy must trigger the registered callback
 */
TEST_F(EthernetControllerProxyTest, trigger_callback_on_receive_message)
{
    EthMessage msg;

    EXPECT_CALL(callbacks, ReceiveMessage(&proxy, msg))
        .Times(1);

    proxy.ReceiveIbMessage(controllerAddress, msg);
}

/*! \brief Passing an Ack to an EthControllerProxy must trigger the registered callback
 */
TEST_F(EthernetControllerProxyTest, trigger_callback_on_receive_ack)
{
    EthTransmitAcknowledge expectedAck{17, 42ms, EthTransmitStatus::Transmitted };

    EXPECT_CALL(callbacks, MessageAck(&proxy, expectedAck))
        .Times(1);

    proxy.ReceiveIbMessage(controllerAddress, expectedAck);
}

/*! \brief EthControllerProxy must not generate Acks
 *
 * Rationale:
 *   The EthControllerProxy is used in conjunction with a Network Simulator, which is
 *   responsible for Ack generation.
 */
TEST_F(EthernetControllerProxyTest, must_not_generate_ack)
{
    EthMessage msg;
    msg.transmitId = 17;

    EXPECT_CALL(comAdapter, SendIbMessage(An<EndpointAddress>(), A<const EthTransmitAcknowledge&>()))
        .Times(0);

    proxy.ReceiveIbMessage(controllerAddress, msg);
}

/*! \brief EthControllerProxy must trigger bitrate changed callbacks when necessary
*
* Rationale:
*   The callback should not be triggered upon reception of any EthStatus message. Only if
*   The bit rate changes.
*
* Test steps:
*   - Deliver an EthStatus with bitrate == 0 directly to the EthControllerProxy; this should NOT trigger the callback
*   - Deliver an EthStatus with bitrate != 0 directly to the EthControllerProxy; this should trigger the callback
*   - Check that the callback is executed
*   - Deliver an EthStatus with the same bitrate again 0 directly to the EthControllerProxy; this should NOT trigger the callback
*/
TEST_F(EthernetControllerProxyTest, trigger_callback_on_bitrate_change)
{
    EXPECT_CALL(callbacks, BitRateChanged(&proxy, 100))
        .Times(1);
    EXPECT_CALL(callbacks, BitRateChanged(&proxy, Ne<uint32_t>(100)))
        .Times(0);

    EthStatus newStatus = { 0ns, EthState::Inactive, 0 };
    proxy.ReceiveIbMessage(controllerAddress, newStatus);

    newStatus.bitRate = 100;
    proxy.ReceiveIbMessage(controllerAddress, newStatus);
    proxy.ReceiveIbMessage(controllerAddress, newStatus);
}

/*! \brief EthControllerProxy must trigger state changed callbacks when necessary
*
* Rationale:
*   The callback should not be triggered upon reception of any EthStatus message. Only if
*   the state changes.
*
* Test steps:
*   - Deliver an EthStatus with Inactive directly to the EthControllerProxy; NO callback is triggered
*   - Deliver an EthStatus with LinkUp directly to the EthControllerProxy
*      -> this should trigger the callback
*   - Deliver an EthStatus with LinkUp again; NO callback is triggered
*   - Deliver an EthStatus with LinkDown directly to the EthControllerProxy
*      -> this should trigger the callback
*   - Deliver an EthStatus with LinkDown again; NO callback is triggered
*   - Deliver an EthStatus with Inactive directly to the EthControllerProxy
*      -> this should trigger the callback
*/
TEST_F(EthernetControllerProxyTest, trigger_callback_on_state_change)
{
    InSequence executionSequence;
    EXPECT_CALL(callbacks, StateChanged(&proxy, EthState::LinkUp))
        .Times(1);
    EXPECT_CALL(callbacks, StateChanged(&proxy, EthState::LinkDown))
        .Times(1);
    EXPECT_CALL(callbacks, StateChanged(&proxy, EthState::Inactive))
        .Times(1);


    EthStatus newStatus = { 0ns, EthState::Inactive, 0 };
    proxy.ReceiveIbMessage(controllerAddress, newStatus);

    newStatus.state = EthState::LinkUp;
    proxy.ReceiveIbMessage(controllerAddress, newStatus);
    proxy.ReceiveIbMessage(controllerAddress, newStatus);

    newStatus.state = EthState::LinkDown;
    proxy.ReceiveIbMessage(controllerAddress, newStatus);
    proxy.ReceiveIbMessage(controllerAddress, newStatus);

    newStatus.state = EthState::Inactive;
    proxy.ReceiveIbMessage(controllerAddress, newStatus);
}



} // anonymous namespace
