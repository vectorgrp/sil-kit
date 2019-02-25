// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#include "EthController.hpp"

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
using testing::_;
using testing::InSequence;
using testing::NiceMock;

using namespace ib::mw;
using namespace ib::sim;
using namespace ib::sim::eth;

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
        : controller(&comAdapter)
    {
        controller.SetEndpointAddress(controllerAddress);

        controller.RegisterReceiveMessageHandler(ib::util::bind_method(&callbacks, &Callbacks::ReceiveMessage));
        controller.RegisterMessageAckHandler(ib::util::bind_method(&callbacks, &Callbacks::MessageAck));
    }

protected:
    const EndpointAddress controllerAddress = {3, 8};
    const EndpointAddress otherAddress = {7, 2};

    ib::mw::test::MockComAdapter comAdapter;
    Callbacks callbacks;

    EthController controller;
};


TEST_F(EthernetControllerTest, send_eth_message)
{
    EXPECT_CALL(comAdapter, SendIbMessage_proxy(controllerAddress, A<const EthMessage&>()))
        .Times(1);

    EthMessage msg;
    controller.SendMessage(msg);
}


/*! \brief Passing an EthMessage to an EthControllers must trigger the registered callback
 */
TEST_F(EthernetControllerTest, trigger_callback_on_receive_message)
{
    EthMessage msg;

    EXPECT_CALL(callbacks, ReceiveMessage(&controller, msg))
        .Times(1);

    controller.ReceiveIbMessage(otherAddress, msg);
}

/*! \brief Passing an Ack to an EthControllers must trigger the registered callback
 */
TEST_F(EthernetControllerTest, trigger_callback_on_receive_ack)
{
    EthTransmitAcknowledge ack{17, 42ms, EthTransmitStatus::Transmitted};

    EXPECT_CALL(callbacks, MessageAck(&controller, ack))
        .Times(1);

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

    EthTransmitAcknowledge expectedAck{msg.transmitId, msg.timestamp, EthTransmitStatus::Transmitted};

    EXPECT_CALL(comAdapter, SendIbMessage(controllerAddress, expectedAck))
        .Times(1);

    controller.ReceiveIbMessage(otherAddress, msg);
}


} // anonymous namespace
