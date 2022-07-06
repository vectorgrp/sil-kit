// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "EthController.hpp"

#include <chrono>
#include <functional>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "silkit/util/functional.hpp"

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

using namespace SilKit::Core;
using namespace SilKit::Services;
using namespace SilKit::Services::Ethernet;

using ::SilKit::Core::Tests::DummyParticipant;
using ::SilKit::Tests::MockTraceSink;

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

    MOCK_METHOD2(SendMsg, void(const IServiceEndpoint*, const EthernetFrameEvent&));
    MOCK_METHOD2(SendMsg, void(const IServiceEndpoint*, const EthernetFrameTransmitEvent&));
    MOCK_METHOD2(SendMsg, void(const IServiceEndpoint*, const EthernetStatus&));
    MOCK_METHOD2(SendMsg, void(const IServiceEndpoint*, const EthernetSetMode&));
};

class EthernetControllerTrivialSimTest : public testing::Test
{
protected:
    struct Callbacks
    {
        MOCK_METHOD2(ReceiveMessage, void(Ethernet::IEthernetController*, const Ethernet::EthernetFrameEvent&));
        MOCK_METHOD2(MessageAck, void(Ethernet::IEthernetController*, Ethernet::EthernetFrameTransmitEvent));
        MOCK_METHOD2(StateChanged, void(Ethernet::IEthernetController*, Ethernet::EthernetState));
        MOCK_METHOD2(BitRateChanged, void(Ethernet::IEthernetController*, Ethernet::EthernetBitrate));
    };

protected:
    EthernetControllerTrivialSimTest()
        : controller(&participant, cfg, participant.GetTimeProvider())
        , controllerOther(&participant, cfg, participant.GetTimeProvider())
    {
        controller.SetServiceDescriptor(from_endpointAddress(controllerAddress));

        controller.AddFrameHandler(SilKit::Util::bind_method(&callbacks, &Callbacks::ReceiveMessage));
        controller.AddFrameTransmitHandler(SilKit::Util::bind_method(&callbacks, &Callbacks::MessageAck));

        controllerOther.SetServiceDescriptor(from_endpointAddress(otherAddress));
    }

protected:
    const EndpointAddress controllerAddress = {3, 8};
    const EndpointAddress otherAddress = {7, 2};

    MockTraceSink traceSink;
    MockParticipant participant;
    Callbacks callbacks;

    SilKit::Config::EthernetController cfg;
    EthController controller;
    EthController controllerOther;
};

/*! \brief SendFrame must invoke the TimeProvider and triggers SendMsg on the participant
*/
TEST_F(EthernetControllerTrivialSimTest, send_eth_frame)
{
    ON_CALL(participant.mockTimeProvider.mockTime, Now())
        .WillByDefault(testing::Return(42ns));

    const auto now = 42ns;
    EXPECT_CALL(participant, SendMsg(&controller, AnEthMessageWith(now))).Times(1);

    EthernetFrameTransmitEvent ack{};
    ack.sourceMac = EthernetMac{0, 0, 0, 0, 0, 0};
    ack.status = EthernetTransmitStatus::Transmitted;
    ack.timestamp = 42ns;
    EXPECT_CALL(callbacks, MessageAck(&controller, EthernetTransmitAckWithouthTransmitIdMatcher(ack))).Times(1);

    // once for activate and once for sending the frame
    EXPECT_CALL(participant.mockTimeProvider.mockTime, Now()).Times(2);

    EthernetFrame frame{};
    SetSourceMac(frame, EthernetMac{ 0, 0, 0, 0, 0, 0 });
    controller.Activate();
    controller.SendFrame(frame);
}

/*! \brief SendFrame without Activate must trigger a nack
*/
TEST_F(EthernetControllerTrivialSimTest, nack_on_inactive_controller)
{
    ON_CALL(participant.mockTimeProvider.mockTime, Now()).WillByDefault(testing::Return(42ns));

    const auto now = 42ns;
    EXPECT_CALL(participant, SendMsg(&controller, AnEthMessageWith(now))).Times(0);

    EthernetFrameTransmitEvent nack{};
    nack.sourceMac = EthernetMac{0, 0, 0, 0, 0, 0};
    nack.status = EthernetTransmitStatus::ControllerInactive;
    nack.timestamp = 42ns;
    EXPECT_CALL(callbacks, MessageAck(&controller, EthernetTransmitAckWithouthTransmitIdMatcher(nack))).Times(1);

    // once for the nack
    EXPECT_CALL(participant.mockTimeProvider.mockTime, Now()).Times(1);


    EthernetFrame frame{};
    SetSourceMac(frame, EthernetMac{0, 0, 0, 0, 0, 0});
    controller.SendFrame(frame);
}


/*! \brief The controller must change it's state when a Call to Activate/Deactivate is triggered
*/
TEST_F(EthernetControllerTrivialSimTest, linkup_controller_inactive_on_activate_deactivate)
{
    controller.Activate();
    ASSERT_TRUE(controller.GetState() == EthernetState::LinkUp);
    controller.Deactivate();
    ASSERT_TRUE(controller.GetState() == EthernetState::Inactive);
}

/*! \brief Passing an EthernetFrameEvent to an EthControllers must trigger the registered callback
 */
TEST_F(EthernetControllerTrivialSimTest, trigger_callback_on_receive_message)
{
    EthernetFrameEvent msg;
    SetSourceMac(msg.frame, EthernetMac{ 0, 0, 0, 0, 0, 0 });

    EXPECT_CALL(callbacks, ReceiveMessage(&controller, msg))
        .Times(1);

    controller.Activate();
    controller.ReceiveSilKitMessage(&controllerOther, msg);
}

/*! \brief Passing an Ack to an EthControllers must trigger the registered callback, if
 *         it sent a message with corresponding transmit ID and source MAC
 */
TEST_F(EthernetControllerTrivialSimTest, trigger_callback_on_receive_ack)
{
    EthernetFrameEvent msg{};
    SetSourceMac(msg.frame, EthernetMac{ 1, 2, 3, 4, 5, 6 });

    EXPECT_CALL(participant, SendMsg(&controller, AnEthMessageWith(0ns))).Times(1);
    EthernetFrameTransmitEvent ack{ 0, EthernetMac{ 1, 2, 3, 4, 5, 6 }, 0ms, EthernetTransmitStatus::Transmitted };
    EXPECT_CALL(callbacks, MessageAck(&controller, EthernetTransmitAckWithouthTransmitIdMatcher(ack)))
        .Times(1);

    // once for activate and once for sending the frame
    EXPECT_CALL(participant.mockTimeProvider.mockTime, Now()).Times(2);
    controller.Activate();
    controller.SendFrameEvent(msg);
}

/*! \brief Multiple handlers added and removed
 */
TEST_F(EthernetControllerTrivialSimTest, add_remove_handler)
{
    EthController testController{&participant, cfg, participant.GetTimeProvider()};

    const int numHandlers = 10;
    std::vector<SilKit::Services::HandlerId> handlerIds;
    for (int i = 0; i < numHandlers; i++)
    {
        handlerIds.push_back(testController.AddFrameHandler(SilKit::Util::bind_method(&callbacks, &Callbacks::ReceiveMessage)));
    }

    EthernetFrameEvent msg;
    SetSourceMac(msg.frame, EthernetMac{0, 0, 0, 0, 0, 0});
    EXPECT_CALL(callbacks, ReceiveMessage(&testController, msg)).Times(numHandlers);
    testController.ReceiveSilKitMessage(&controllerOther, msg);

    for (auto&& handlerId : handlerIds)
    {
        testController.RemoveFrameHandler(handlerId);
    }

    EXPECT_CALL(callbacks, ReceiveMessage(&testController, msg)).Times(0);
    testController.ReceiveSilKitMessage(&controllerOther, msg);
}

/*! \brief Removed handler in handler
 */
TEST_F(EthernetControllerTrivialSimTest, remove_handler_in_handler)
{
    EthController testController{&participant, cfg, participant.GetTimeProvider()};

    auto handlerIdToRemove =
        testController.AddFrameHandler(SilKit::Util::bind_method(&callbacks, &Callbacks::ReceiveMessage));

    auto testHandler = [handlerIdToRemove](Ethernet::IEthernetController* ctrl, const Ethernet::EthernetFrameEvent&) {
        ctrl->RemoveFrameHandler(handlerIdToRemove);
    };
    testController.AddFrameHandler(testHandler);

    EthernetFrameEvent msg;
    SetSourceMac(msg.frame, EthernetMac{0, 0, 0, 0, 0, 0});
    EXPECT_CALL(callbacks, ReceiveMessage(&testController, msg)).Times(1);
    // Calls testHandler and Callbacks::ReceiveMessage, the latter is removed in testHandler 
    testController.ReceiveSilKitMessage(&controllerOther, msg);
    EXPECT_CALL(callbacks, ReceiveMessage(&testController, msg)).Times(0);
    // Call testHandler again, handlerIdToRemove is invalid now but should only result in a warning
    testController.ReceiveSilKitMessage(&controllerOther, msg);
}


TEST_F(EthernetControllerTrivialSimTest, DISABLED_ethcontroller_uses_tracing)
{
#if (0)
    

    const auto now = 1337ns;
    ON_CALL(participant.mockTimeProvider.mockTime, Now())
        .WillByDefault(testing::Return(now));

    SilKit::Config::EthernetController config{};
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
        Trace(SilKit::Services::TransmitDirection::TX, controllerAddress, now, frame))
        .Times(1);
    ethController.SendFrame(frame);

    // Receive direction
    EXPECT_CALL(traceSink,
        Trace(SilKit::Services::TransmitDirection::RX, controllerAddress, now, frame))
        .Times(1);

    EthernetFrameEvent ethMsg{};
    ethMsg.frame = frame;
    ethMsg.timestamp = now;
    ethController.ReceiveSilKitMessage(&controllerOther, ethMsg);
#endif // 0
}

} // anonymous namespace
