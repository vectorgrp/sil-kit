// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "EthController.hpp"

#include <chrono>
#include <functional>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ib/mw/string_utils.hpp"
#include "ib/util/functional.hpp"

#include "MockComAdapter.hpp"
#include "MockTraceSink.hpp"

#include "EthDatatypeUtils.hpp"

namespace {

using namespace std::chrono_literals;

using testing::Return;
using testing::A;
using testing::An;
using testing::_;
using testing::InSequence;
using testing::NiceMock;

using namespace ib::mw;
using namespace ib::sim;
using namespace ib::sim::eth;

using ::ib::mw::test::DummyComAdapter;
using ::ib::test::MockTraceSink;

auto AnEthMessageWith(std::chrono::nanoseconds timestamp) -> testing::Matcher<const EthMessage&>
{
    return testing::Field(&EthMessage::timestamp, timestamp);
}

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

class EthernetControllerTest : public testing::Test
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
    EthernetControllerTest()
        : controller(&comAdapter, _config, comAdapter.GetTimeProvider())
    {
        controller.SetEndpointAddress(controllerAddress);

        controller.RegisterReceiveMessageHandler(ib::util::bind_method(&callbacks, &Callbacks::ReceiveMessage));
        controller.RegisterMessageAckHandler(ib::util::bind_method(&callbacks, &Callbacks::MessageAck));
    }

protected:
    const EndpointAddress controllerAddress = {3, 8};
    const EndpointAddress otherAddress = {7, 2};

    MockTraceSink traceSink;
    MockComAdapter comAdapter;
    Callbacks callbacks;

    ib::cfg::EthernetController _config;
    EthController controller;
};


TEST_F(EthernetControllerTest, send_eth_message)
{
    const auto now = 12345ns;
    EXPECT_CALL(comAdapter, SendIbMessage_proxy(controllerAddress, AnEthMessageWith(now)))
        .Times(1);

    EXPECT_CALL(comAdapter.mockTimeProvider.mockTime, Now()).Times(0);

    EthMessage msg;
    msg.ethFrame.SetSourceMac(EthMac{ 0,0,0,0,0,0 });
    msg.timestamp = now;
    controller.SendMessage(msg);
}

//! \brief using the new SendFrame API must invoke the TimeProvider
TEST_F(EthernetControllerTest, send_eth_frame)
{
    ON_CALL(comAdapter.mockTimeProvider.mockTime, Now())
        .WillByDefault(testing::Return(42ns));

    const auto now = 42ns;
    EXPECT_CALL(comAdapter, SendIbMessage_proxy(controllerAddress, AnEthMessageWith(now)))
        .Times(1);

    EXPECT_CALL(comAdapter.mockTimeProvider.mockTime, Now()).Times(1);

    EthFrame ethFrame;
    ethFrame.SetSourceMac(EthMac{ 0,0,0,0,0,0 });
    controller.SendFrame(ethFrame);
}

/*! \brief Passing an EthMessage to an EthControllers must trigger the registered callback
 */
TEST_F(EthernetControllerTest, trigger_callback_on_receive_message)
{
    EthMessage msg;
    msg.ethFrame.SetSourceMac(EthMac{ 0,0,0,0,0,0 });

    EXPECT_CALL(callbacks, ReceiveMessage(&controller, msg))
        .Times(1);

    controller.ReceiveIbMessage(otherAddress, msg);
}

/*! \brief Passing an Ack to an EthControllers must trigger the registered callback, if
 *         it sent a message with corresponding transmit ID and source MAC
 */
TEST_F(EthernetControllerTest, trigger_callback_on_receive_ack)
{
    EthMessage msg;
    msg.ethFrame.SetSourceMac(EthMac{1,2,3,4,5,6});
    auto tid = controller.SendMessage(msg);

    EthTransmitAcknowledge ack{tid, EthMac{1,2,3,4,5,6}, 42ms, EthTransmitStatus::Transmitted};
    EXPECT_CALL(callbacks, MessageAck(&controller, ack))
        .Times(1);

    controller.ReceiveIbMessage(otherAddress, ack);
}

/*!\brief Passing an Ack to an EthControllers must NOT trigger the registered callback, if
 * it did not send a message with corresponding source MAC
 */
TEST_F(EthernetControllerTest, dont_trigger_callback_for_unknown_acks)
{
    EthMessage msg;
    msg.ethFrame.SetSourceMac(EthMac{1,2,3,4,5,6});
    auto tid = controller.SendMessage(msg);

    EthTransmitAcknowledge ack{tid, EthMac{3,4,5,6,7,8}, 42ms, EthTransmitStatus::Transmitted};
    EXPECT_CALL(callbacks, MessageAck(&controller, ack))
        .Times(0);

    controller.ReceiveIbMessage(otherAddress, ack);
}

/*! \brief EthControllers must generate Acks upon EthMessage reception
 *
 * Idea of Test generate_ack_on_receive_msg:
 *   The EthController is intended for stand alone usage without a Network Simulator.
 *   I.e., the acks must be generated from the controllers themselves upon reception.
 *
 */
TEST_F(EthernetControllerTest, generate_ack_on_receive_msg)
{
    EthMessage msg;
    msg.timestamp  = 42ms;
    msg.transmitId = 17;
    msg.ethFrame.SetSourceMac(EthMac{1,2,3,4,5,6});

    EthTransmitAcknowledge expectedAck{ msg.transmitId, EthMac{1,2,3,4,5,6}, msg.timestamp, EthTransmitStatus::Transmitted };

    EXPECT_CALL(comAdapter, SendIbMessage(controllerAddress, expectedAck))
        .Times(1);

    controller.ReceiveIbMessage(otherAddress, msg);
}

TEST_F(EthernetControllerTest, ethcontroller_uses_tracing)
{
    using namespace ib::extensions;

    const auto now = 1337ns;
    ON_CALL(comAdapter.mockTimeProvider.mockTime, Now())
        .WillByDefault(testing::Return(now));

    ib::cfg::EthernetController config{};
    auto controller = EthController(&comAdapter, config, comAdapter.GetTimeProvider());
    controller.SetEndpointAddress(controllerAddress);
    controller.AddSink(&traceSink);


    EthFrame ethFrame{};
    ethFrame.SetDestinationMac(EthMac{1,2,3,4,5,6});
    ethFrame.SetSourceMac(EthMac{9,8,7,6,5,4});

    //Send direction
    EXPECT_CALL(comAdapter.mockTimeProvider.mockTime, Now())
        .Times(1);
    EXPECT_CALL(traceSink,
        Trace(Direction::Send, controllerAddress, now, ethFrame))
        .Times(1);
    controller.SendFrame(ethFrame);

    // Receive direction
    EXPECT_CALL(traceSink,
        Trace(Direction::Receive, controllerAddress, now, ethFrame))
        .Times(1);

    EthMessage ethMsg{};
    ethMsg.ethFrame = ethFrame;
    ethMsg.timestamp = now;
    controller.ReceiveIbMessage(otherAddress, ethMsg);
}

} // anonymous namespace
