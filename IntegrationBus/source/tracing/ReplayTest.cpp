// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <memory>
#include <future>

#include "ib/cfg/Config.hpp"
#include "ib/mw/EndpointAddress.hpp"
#include "ib/util/functional.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "MockParticipant.hpp"
#include "Timer.hpp"

#include "EthControllerReplay.hpp"
#if 0 // Replay is inactive for now
#include "GenericPublisherReplay.hpp"
#include "GenericSubscriberReplay.hpp"
namespace {

using namespace ib;
using namespace ib::mw;
using namespace ib::mw::test;
using namespace ib::tracing;
using namespace ib::sim::eth;
using namespace ib::sim::can;
using namespace ib::sim::generic;

using namespace std::chrono_literals;

using testing::A;
using testing::An;

auto ACanMessage(const CanMessage& msg) -> testing::Matcher<const CanMessage&>
{
  using namespace testing;
  return AllOf(
    Field(&CanMessage::canId, Eq(msg.canId))
    , Field(&CanMessage::dataField, Eq(msg.dataField))
    , Field(&CanMessage::timestamp, Eq(msg.timestamp))
  );
}

auto AService(const IIbServiceEndpoint* service) -> testing::Matcher<const IIbServiceEndpoint*>
{
  using namespace testing;
  return AllOf(
    Property(&IIbServiceEndpoint::GetServiceDescriptor, Eq(service->GetServiceDescriptor()))
  );
}

auto AnEthMessage(const EthernetFrame& msg) -> testing::Matcher<const EthernetFrameEvent&>
{
    using namespace testing;
    return Field(&EthernetFrameEvent::frame,
        AllOf(
            Field(&EthernetFrame::raw, ContainerEq(msg))
        )
    );
}


auto AGenericMessage(const GenericMessage& msg) -> testing::Matcher<const GenericMessage&>
{
    using namespace testing;
    return Field(&GenericMessage::data, Eq(msg.data));
}

TEST(ReplayTest, ensure_util_timer_works)
{

    {
        //Make sure DTor is able to stop a running timer
        ib::util::Timer timer;
        timer.WithPeriod(std::chrono::milliseconds(50), [](const auto) {});
    }

    {
        std::promise<void> done;
        auto isDone = done.get_future();
        ib::util::Timer timer;
        auto numCalls = 0u;
        auto cb = [&](const auto) {
            numCalls++;
            if (numCalls == 5)
            {
                timer.Stop();
                done.set_value();
            }
        };
        timer.WithPeriod(std::chrono::milliseconds(50), cb);
        ASSERT_TRUE(timer.IsActive());
        isDone.wait_for(1s);
        ASSERT_TRUE(!timer.IsActive());
        ASSERT_EQ(numCalls, 5);
    }
}

class MockParticipant : public DummyParticipant
{
public:
    //Ethernet calls
    void SendIbMessage(const IIbServiceEndpoint* from, EthernetFrameEvent&& msg) override
    {
        SendIbMessage_proxy(from, msg);
    }
    MOCK_METHOD2(SendIbMessage, void(IIbServiceEndpoint*, const EthernetFrameEvent&));
    MOCK_METHOD2(SendIbMessage_proxy, void(const IIbServiceEndpoint*, const EthernetFrameEvent&));
    MOCK_METHOD2(SendIbMessage, void(IIbServiceEndpoint*, const EthernetFrameTransmitEvent&));
    MOCK_METHOD2(SendIbMessage, void(IIbServiceEndpoint*, const EthernetStatus&));
    MOCK_METHOD2(SendIbMessage, void(IIbServiceEndpoint*, const EthernetSetMode&));
    //  Generic Message calls
    MOCK_METHOD2(SendIbMessage_proxy, void(const IIbServiceEndpoint*, const GenericMessage&));
    void SendIbMessage(const IIbServiceEndpoint* from, GenericMessage&& msg) override
    {
        SendIbMessage_proxy(from, msg);
    }
    MOCK_METHOD2(ReceiveIbMessage, void(EndpointAddress, const GenericMessage&));

    // CAN
    MOCK_METHOD2(SendIbMessage_proxy, void(const IIbServiceEndpoint*, const CanMessage&));
    MOCK_METHOD2(ReceiveIbMessage, void(EndpointAddress, const CanMessage&));
    void SendIbMessage(const IIbServiceEndpoint* from, CanMessage&& msg) override
    {
        SendIbMessage_proxy(from, msg);
    }
};

struct Callbacks
{
    MOCK_METHOD2(ReceiveMessage, void(IEthernetController*, const EthernetFrameEvent&));
    MOCK_METHOD2(ReceiveMessageCan, void(ICanController*, const CanMessage&));
};

struct MockReplayMessage
    : public extensions::IReplayMessage
{
    auto Timestamp() const -> std::chrono::nanoseconds override
    {
        return _timestamp;
    }
    auto GetDirection() const -> ib::sim::TransmitDirection
    {
        return _direction;
    }
    auto EndpointAddress() const -> ib::mw::EndpointAddress
    {
        return _address;
    }
    auto Type() const -> extensions::TraceMessageType
    {
        return _type;
    }

    std::chrono::nanoseconds _timestamp{0};
    ib::sim::TransmitDirection _direction{ ib::sim::TransmitDirection::RX};
    ib::mw::EndpointAddress _address{0, 0};
    extensions::TraceMessageType _type{extensions::TraceMessageType::InvalidReplayData};
};

struct MockEthFrame 
    : public MockReplayMessage
    , public EthernetFrame
{
    MockEthFrame()
    {
        SetSourceMac(EthernetMac{1,2,3,4,5,6});
        SetDestinationMac(EthernetMac{7,8,9,0xa,0xb,0xc});
        _type = extensions::TraceMessageType::EthernetFrame;
    }
};

TEST(ReplayTest, ethcontroller_replay_config_send)
{
    MockParticipant participant{};

    cfg::EthernetController cfg{};

    MockEthFrame msg;
    msg._address = {1,2};

    // Replay Send / Send
    {
        msg._direction = ib::sim::TransmitDirection::TX;
        cfg.replay.direction = cfg::Replay::Direction::Send;

        EthControllerReplay ctrl{&participant, cfg, participant.GetTimeProvider()};
        ctrl.SetServiceDescriptor(from_endpointAddress(msg._address));
        EXPECT_CALL(participant, SendIbMessage_proxy(AService(&ctrl), AnEthMessage(msg)))
            .Times(1);
        EXPECT_CALL(participant.mockTimeProvider.mockTime, Now()).Times(1);
        ctrl.ReplayMessage(&msg);
    }

    // Replay Send / Both
    {
        msg._direction = ib::sim::TransmitDirection::TX;
        cfg.replay.direction = cfg::Replay::Direction::Both;
        EthControllerReplay ctrl{&participant, cfg, participant.GetTimeProvider()};
        ctrl.SetServiceDescriptor(from_endpointAddress(msg._address));
        EXPECT_CALL(participant, SendIbMessage_proxy(AService(&ctrl), AnEthMessage(msg)))
            .Times(1);
        EXPECT_CALL(participant.mockTimeProvider.mockTime, Now()).Times(1);
        ctrl.ReplayMessage(&msg);
    }

    // Block Send 
    {
        msg._direction = ib::sim::TransmitDirection::RX;
        cfg.replay.direction = cfg::Replay::Direction::Send;
        EthControllerReplay ctrl{&participant, cfg, participant.GetTimeProvider()};
        ctrl.SetServiceDescriptor(from_endpointAddress(msg._address));
        EXPECT_CALL(participant, SendIbMessage_proxy(AService(&ctrl), AnEthMessage(msg)))
            .Times(0);
        EXPECT_CALL(participant.mockTimeProvider.mockTime, Now()).Times(0);
        ctrl.ReplayMessage(&msg);
    }

}


TEST(ReplayTest, ethcontroller_replay_config_receive)
{
    Callbacks callbacks;
    MockParticipant participant{};

    cfg::EthernetController cfg{};


    MockEthFrame msg;

    msg._address = {1,2};


    // Replay Receive / Receive
    {
        msg._direction = ib::sim::TransmitDirection::RX;
        cfg.replay.direction = cfg::Replay::Direction::Receive;
        EthControllerReplay controller{&participant, cfg, participant.GetTimeProvider()};
        controller.SetServiceDescriptor(from_endpointAddress({3,4}));
        controller.AddFrameHandler(ib::util::bind_method(&callbacks, &Callbacks::ReceiveMessage));
        EXPECT_CALL(callbacks, ReceiveMessage(A<IEthernetController*>(), AnEthMessage(msg)))
            .Times(1);
        controller.ReplayMessage(&msg);
    }

    // Replay Receive / Both
    {
        msg._direction = ib::sim::TransmitDirection::RX;
        cfg.replay.direction = cfg::Replay::Direction::Both;
        EthControllerReplay controller{&participant, cfg, participant.GetTimeProvider()};
        controller.SetServiceDescriptor(from_endpointAddress({3,4}));
        controller.AddFrameHandler(ib::util::bind_method(&callbacks, &Callbacks::ReceiveMessage));
        EXPECT_CALL(callbacks, ReceiveMessage(A<IEthernetController*>(), AnEthMessage(msg)))
            .Times(1);
        controller.ReplayMessage(&msg);
    }
    // Block Receive 
    {
        msg._direction = ib::sim::TransmitDirection::TX;
        cfg.replay.direction = cfg::Replay::Direction::Receive;
        EthControllerReplay controller{&participant, cfg, participant.GetTimeProvider()};
        controller.SetServiceDescriptor(from_endpointAddress({3,4}));
        controller.AddFrameHandler(ib::util::bind_method(&callbacks, &Callbacks::ReceiveMessage));
        EXPECT_CALL(callbacks, ReceiveMessage(A<IEthernetController*>(), AnEthMessage(msg)))
            .Times(0);
        controller.ReplayMessage(&msg);
    }
}

// // // CAN

struct MockCanMessage
    : public MockReplayMessage
    , public CanMessage
{
    MockCanMessage()
    {
        timestamp = std::chrono::nanoseconds{0};
        canId = 0;
        
        dataField.resize(8);
        size_t i = 0u;
        for (auto& d : dataField)
        {
            d = 'a' + (i++ % 26);
        }
    }
};

TEST(ReplayTest, DISABLED_cancontroller_replay_config_send)
{
    MockParticipant participant{};

    cfg::CanController cfg{};

    MockCanMessage msg;
    msg._address = {1,2};
    /*
    // Replay Send / Send
    {
        msg._direction = ib::sim::TransmitDirection::TX;
        cfg.replay.direction = cfg::Replay::Direction::Send;
    
        CanControllerReplay can{&participant, cfg, participant.GetTimeProvider()};
        can.SetServiceDescriptor(from_endpointAddress(msg._address));
        EXPECT_CALL(participant, SendIbMessage_proxy(AService(&can), ACanMessage(msg)))
            .Times(1);
        EXPECT_CALL(participant.mockTimeProvider.mockTime, Now()).Times(1);
        can.ReplayMessage(&msg);
    }
    // Replay Send / Both
    {
        msg._direction = ib::sim::TransmitDirection::TX;
        cfg.replay.direction = cfg::Replay::Direction::Both;
    
        CanControllerReplay can{&participant, cfg, participant.GetTimeProvider()};
        can.SetServiceDescriptor(from_endpointAddress(msg._address));
        EXPECT_CALL(participant, SendIbMessage_proxy(AService(&can), ACanMessage(msg)))
            .Times(1);
        EXPECT_CALL(participant.mockTimeProvider.mockTime, Now()).Times(1);
        can.ReplayMessage(&msg);
    }
    // Replay Send / Receive
    {
        msg._direction = ib::sim::TransmitDirection::TX;
        cfg.replay.direction = cfg::Replay::Direction::Receive;
    
        CanControllerReplay can{&participant, cfg, participant.GetTimeProvider()};
        can.SetServiceDescriptor(from_endpointAddress(msg._address));
        EXPECT_CALL(participant, SendIbMessage_proxy(AService(&can), ACanMessage(msg)))
            .Times(0);
        EXPECT_CALL(participant.mockTimeProvider.mockTime, Now()).Times(0);
        can.ReplayMessage(&msg);
    }
    // Replay Receive / Send
    {
        msg._direction = ib::sim::TransmitDirection::RX;
        cfg.replay.direction = cfg::Replay::Direction::Send;
    
        CanControllerReplay can{&participant, cfg, participant.GetTimeProvider()};
        EXPECT_CALL(participant, SendIbMessage_proxy(AService(&can), ACanMessage(msg)))
            .Times(0);
        EXPECT_CALL(participant.mockTimeProvider.mockTime, Now()).Times(0);
        can.SetServiceDescriptor(from_endpointAddress(msg._address));
        can.ReplayMessage(&msg);
    }
    // Replay Receive / Receive
    {
        msg._direction = ib::sim::TransmitDirection::RX;
        cfg.replay.direction = cfg::Replay::Direction::Receive;
        msg._address = tracing::ReplayEndpointAddress();
    
        CanControllerReplay can{&participant, cfg, participant.GetTimeProvider()};
        EXPECT_CALL(participant, SendIbMessage_proxy(AService(&can), ACanMessage(msg)))
            .Times(0);
        EXPECT_CALL(participant.mockTimeProvider.mockTime, Now()).Times(1);
        can.SetServiceDescriptor(from_endpointAddress(msg._address));
        can.ReplayMessage(&msg);
    }
    */
}

TEST(ReplayTest, DISABLED_cancontroller_replay_config_receive)
{
    Callbacks callbacks;
    MockParticipant participant{};

    cfg::CanController cfg{};


    MockCanMessage msg;

    msg._address = {1,2};

    /*
    // Replay Receive / Receive
    {
        msg._direction = ib::sim::TransmitDirection::RX;
        cfg.replay.direction = cfg::Replay::Direction::Receive;
        CanControllerReplay controller{&participant, cfg, participant.GetTimeProvider()};
        controller.SetServiceDescriptor(from_endpointAddress({3,4}));
        controller.AddFrameHandler(ib::util::bind_method(&callbacks, &Callbacks::ReceiveMessageCan));
        EXPECT_CALL(callbacks, ReceiveMessageCan(A<ICanController*>(), ACanMessage(msg)))
            .Times(1);
        controller.ReplayMessage(&msg);
    }
    
    // Replay Receive / Both
    {
        msg._direction = ib::sim::TransmitDirection::RX;
        cfg.replay.direction = cfg::Replay::Direction::Both;
        CanControllerReplay controller{&participant, cfg, participant.GetTimeProvider()};
        controller.SetServiceDescriptor(from_endpointAddress({3,4}));
        controller.AddFrameHandler(ib::util::bind_method(&callbacks, &Callbacks::ReceiveMessageCan));
        EXPECT_CALL(callbacks, ReceiveMessageCan(A<ICanController*>(), ACanMessage(msg)))
            .Times(1);
        controller.ReplayMessage(&msg);
    }
    // Block Receive 
    {
        msg._direction = ib::sim::TransmitDirection::TX;
        cfg.replay.direction = cfg::Replay::Direction::Receive;
        CanControllerReplay controller{&participant, cfg, participant.GetTimeProvider()};
        controller.SetServiceDescriptor(from_endpointAddress({3,4}));
        controller.AddFrameHandler(ib::util::bind_method(&callbacks, &Callbacks::ReceiveMessageCan));
        EXPECT_CALL(callbacks, ReceiveMessageCan(A<ICanController*>(), ACanMessage(msg)))
            .Times(0);
        controller.ReplayMessage(&msg);
    }
    */
}

struct MockGenericMessage
    : public MockReplayMessage
    , public sim::generic::GenericMessage
{
    MockGenericMessage()
    {
        _type = extensions::TraceMessageType::GenericMessage;
        data.resize(1024);
        size_t i = 0u;
        for (auto& d : data)
        {
            d = 'a' + (i++ % 26);
        }
    }
};

TEST(ReplayTest, genericpublisher_replay_config_send)
{
    MockParticipant participant{};

    cfg::GenericPort cfg{};

    MockGenericMessage msg;
    msg._address = {1,2};

    // Replay Send / Send
    {
        msg._direction = ib::sim::TransmitDirection::TX;
        cfg.replay.direction = cfg::Replay::Direction::Send;

        sim::generic::GenericPublisherReplay pub{&participant, cfg, participant.GetTimeProvider()};
        pub.SetServiceDescriptor(from_endpointAddress(msg._address));
        EXPECT_CALL(participant, SendIbMessage_proxy(AService(&pub), AGenericMessage(msg)))
            .Times(1);
        EXPECT_CALL(participant.mockTimeProvider.mockTime, Now()).Times(1);
        pub.ReplayMessage(&msg);
    }
    // Replay Send / Both
    {
        msg._direction = ib::sim::TransmitDirection::TX;
        cfg.replay.direction = cfg::Replay::Direction::Both;

        sim::generic::GenericPublisherReplay pub{&participant, cfg, participant.GetTimeProvider()};
        pub.SetServiceDescriptor(from_endpointAddress(msg._address));
        EXPECT_CALL(participant, SendIbMessage_proxy(AService(&pub), AGenericMessage(msg)))
            .Times(1);
        EXPECT_CALL(participant.mockTimeProvider.mockTime, Now()).Times(1);
        pub.ReplayMessage(&msg);
    }
    // Replay Receive / Both
    {
        msg._direction = ib::sim::TransmitDirection::RX;
        cfg.replay.direction = cfg::Replay::Direction::Send;

        sim::generic::GenericPublisherReplay pub{&participant, cfg, participant.GetTimeProvider()};
        EXPECT_CALL(participant, SendIbMessage_proxy(AService(&pub), AGenericMessage(msg)))
            .Times(0);
        EXPECT_CALL(participant.mockTimeProvider.mockTime, Now()).Times(0);
        pub.SetServiceDescriptor(from_endpointAddress(msg._address));
        pub.ReplayMessage(&msg);
    }
    // Replay Receive / Receive
    {
        msg._direction = ib::sim::TransmitDirection::RX;
        cfg.replay.direction = cfg::Replay::Direction::Receive;

        sim::generic::GenericPublisherReplay pub{&participant, cfg, participant.GetTimeProvider()};
        EXPECT_CALL(participant, SendIbMessage_proxy(AService(&pub), AGenericMessage(msg)))
            .Times(0);
        EXPECT_CALL(participant.mockTimeProvider.mockTime, Now()).Times(0);
        pub.SetServiceDescriptor(from_endpointAddress(msg._address));
        pub.ReplayMessage(&msg);
    }
}

TEST(ReplayTest, genericsubscriber_replay_config_send)
{
    MockParticipant participant{};

    cfg::GenericPort cfg{};

    MockGenericMessage msg;
    msg._address = {1,2};

    // Replay Send / Send
    {
        msg._direction = ib::sim::TransmitDirection::TX;
        cfg.replay.direction = cfg::Replay::Direction::Send;

        sim::generic::GenericSubscriberReplay sub{&participant, cfg, participant.GetTimeProvider()};
        sub.SetServiceDescriptor(from_endpointAddress(msg._address));
        EXPECT_CALL(participant, ReceiveIbMessage(msg._address, AGenericMessage(msg)))
            .Times(0);
        EXPECT_CALL(participant.mockTimeProvider.mockTime, Now()).Times(0);
        sub.ReplayMessage(&msg);
    }
    // Replay Send / Both
    {
        msg._direction = ib::sim::TransmitDirection::TX;
        cfg.replay.direction = cfg::Replay::Direction::Both;

        sim::generic::GenericSubscriberReplay sub{&participant, cfg, participant.GetTimeProvider()};
        sub.SetServiceDescriptor(from_endpointAddress(msg._address));
        EXPECT_CALL(participant, ReceiveIbMessage(msg._address, AGenericMessage(msg)))
            .Times(0);
        EXPECT_CALL(participant.mockTimeProvider.mockTime, Now()).Times(0);
        sub.ReplayMessage(&msg);
    }
    // Replay Receive / Both
    {
        msg._direction = ib::sim::TransmitDirection::RX;
        cfg.replay.direction = cfg::Replay::Direction::Both;

        sim::generic::GenericSubscriberReplay sub{&participant, cfg, participant.GetTimeProvider()};
        sub.SetServiceDescriptor(from_endpointAddress({1,3}));
        EXPECT_CALL(participant, ReceiveIbMessage(msg._address, AGenericMessage(msg)))
            .Times(0);
        EXPECT_CALL(participant.mockTimeProvider.mockTime, Now()).Times(1);
        sub.ReplayMessage(&msg);
    }
    // Replay Receive / Receive
    {
        msg._direction = ib::sim::TransmitDirection::RX;
        cfg.replay.direction = cfg::Replay::Direction::Receive;

        sim::generic::GenericSubscriberReplay sub{&participant, cfg, participant.GetTimeProvider()};
        sub.SetServiceDescriptor(from_endpointAddress({1,3}));
        EXPECT_CALL(participant, ReceiveIbMessage(msg._address, AGenericMessage(msg)))
            .Times(0);
        EXPECT_CALL(participant.mockTimeProvider.mockTime, Now()).Times(1);
        sub.ReplayMessage(&msg);
    }
}

} //end anonymous namespace
#endif
