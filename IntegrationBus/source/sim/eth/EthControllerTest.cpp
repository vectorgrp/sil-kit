// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "EthController.hpp"

#include <chrono>
#include <functional>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ib/util/functional.hpp"

#include "MockParticipant.hpp"
#include "MockTraceSink.hpp"

#include "EthDatatypeUtils.hpp"
#include "ParticipantConfiguration.hpp"

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

using ::ib::mw::test::DummyParticipant;
using ::ib::test::MockTraceSink;

MATCHER_P(EthernetTransmitAckWithouthTransmitIdMatcher, truthAck, "") 
{
    *result_listener << "matches EthernetTransmitAcks without checking the transmit id";
    auto frame1 = truthAck;
    auto frame2 = arg;
    return frame1.sourceMac == frame2.sourceMac && frame1.status == frame2.status && frame1.timestamp == frame2.timestamp;
}

auto AnEthMessageWith(std::chrono::nanoseconds timestamp) -> testing::Matcher<const EthernetFrameEvent&>
{
    return testing::Field(&EthernetFrameEvent::timestamp, timestamp);
}

//void SetDestinationMac(EthernetFrame& frame, const EthernetMac& destination)
//{
//    const size_t MinFrameSize = 64;
//    const size_t DestinationMacStart = 0;
//    if (frame.empty())
//    {
//        frame.resize(MinFrameSize);
//    }
//
//    std::copy(destination.begin(), destination.end(), frame.begin() + DestinationMacStart);
//}

void SetSourceMac(EthernetFrame& frame, const EthernetMac& source)
{
    const size_t MinFrameSize = 64;
    const size_t SourceMacStart = sizeof(EthernetMac);
    if (frame.raw.empty())
    {
        frame.raw.resize(MinFrameSize);
    }

    std::copy(source.begin(), source.end(), frame.raw.begin() + SourceMacStart);
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

class EthernetControllerTest : public testing::Test
{
protected:
    struct Callbacks
    {
        MOCK_METHOD2(ReceiveMessage, void(eth::IEthernetController*, const eth::EthernetFrameEvent&));
        MOCK_METHOD2(MessageAck, void(eth::IEthernetController*, eth::EthernetFrameTransmitEvent));
        MOCK_METHOD2(StateChanged, void(eth::IEthernetController*, eth::EthernetState));
        MOCK_METHOD2(BitRateChanged, void(eth::IEthernetController*, eth::EthernetBitrate));
    };

protected:
    EthernetControllerTest()
        : controller(&participant, _config, participant.GetTimeProvider())
        , controllerOther(&participant, _config, participant.GetTimeProvider())
    {
        controller.SetServiceDescriptor(from_endpointAddress(controllerAddress));

        controller.AddFrameHandler(ib::util::bind_method(&callbacks, &Callbacks::ReceiveMessage));
        controller.AddFrameTransmitHandler(ib::util::bind_method(&callbacks, &Callbacks::MessageAck));

        controllerOther.SetServiceDescriptor(from_endpointAddress(otherAddress));
    }

protected:
    const EndpointAddress controllerAddress = {3, 8};
    const EndpointAddress otherAddress = {7, 2};

    MockTraceSink traceSink;
    MockParticipant participant;
    Callbacks callbacks;

    ib::cfg::EthernetController _config;
    EthController controller;
    EthController controllerOther;
};


TEST_F(EthernetControllerTest, send_eth_message)
{
    const auto now = 12345ns;
    EXPECT_CALL(participant, SendIbMessage_proxy(&controller, AnEthMessageWith(now)))
        .Times(1);

    EXPECT_CALL(participant.mockTimeProvider.mockTime, Now()).Times(0);

    EthernetFrameEvent msg;
    SetSourceMac(msg.frame, EthernetMac{ 0, 0, 0, 0, 0, 0 });
    msg.timestamp = now;
    controller.SendFrameEvent(msg);
}

//! \brief using the new SendFrame API must invoke the TimeProvider
TEST_F(EthernetControllerTest, send_eth_frame)
{
    ON_CALL(participant.mockTimeProvider.mockTime, Now())
        .WillByDefault(testing::Return(42ns));

    const auto now = 42ns;
    EXPECT_CALL(participant, SendIbMessage_proxy(&controller, AnEthMessageWith(now)))
        .Times(1);

    EXPECT_CALL(participant.mockTimeProvider.mockTime, Now()).Times(1);

    EthernetFrame frame{};
    SetSourceMac(frame, EthernetMac{ 0, 0, 0, 0, 0, 0 });
    controller.SendFrame(frame);
}

/*! \brief Passing an EthernetFrameEvent to an EthControllers must trigger the registered callback
 */
TEST_F(EthernetControllerTest, trigger_callback_on_receive_message)
{
    EthernetFrameEvent msg;
    SetSourceMac(msg.frame, EthernetMac{ 0, 0, 0, 0, 0, 0 });

    EXPECT_CALL(callbacks, ReceiveMessage(&controller, msg))
        .Times(1);

    controller.ReceiveIbMessage(&controllerOther, msg);
}

/*! \brief Passing an Ack to an EthControllers must trigger the registered callback, if
 *         it sent a message with corresponding transmit ID and source MAC
 */
TEST_F(EthernetControllerTest, trigger_callback_on_receive_ack)
{
    EthernetFrameEvent msg{};
    SetSourceMac(msg.frame, EthernetMac{ 1, 2, 3, 4, 5, 6 });

    EthernetFrameTransmitEvent ack{ 0, EthernetMac{ 1, 2, 3, 4, 5, 6 }, 0ms, EthernetTransmitStatus::Transmitted };
    EXPECT_CALL(callbacks, MessageAck(&controller, EthernetTransmitAckWithouthTransmitIdMatcher(ack)))
        .Times(1);

    controller.SendFrameEvent(msg);
}


TEST_F(EthernetControllerTest, DISABLED_ethcontroller_uses_tracing)
{
#if (0)
    using namespace ib::extensions;

    const auto now = 1337ns;
    ON_CALL(participant.mockTimeProvider.mockTime, Now())
        .WillByDefault(testing::Return(now));

    ib::cfg::EthernetController config{};
    auto ethController = EthController(&participant, config, participant.GetTimeProvider());
    ethController.SetServiceDescriptor(from_endpointAddress(controllerAddress));
    ethController.AddSink(&traceSink);


    EthernetFrame frame{};
    SetDestinationMac(frame, EthernetMac{ 1, 2, 3, 4, 5, 6 });
    SetSourceMac(frame, EthernetMac{ 9, 8, 7, 6, 5, 4 });

    //Send direction
    EXPECT_CALL(participant.mockTimeProvider.mockTime, Now())
        .Times(1);
    EXPECT_CALL(traceSink,
        Trace(ib::sim::TransmitDirection::TX, controllerAddress, now, frame))
        .Times(1);
    ethController.SendFrame(frame);

    // Receive direction
    EXPECT_CALL(traceSink,
        Trace(ib::sim::TransmitDirection::RX, controllerAddress, now, frame))
        .Times(1);

    EthernetFrameEvent ethMsg{};
    ethMsg.frame = frame;
    ethMsg.timestamp = now;
    ethController.ReceiveIbMessage(&controllerOther, ethMsg);
#endif // 0
}

} // anonymous namespace
