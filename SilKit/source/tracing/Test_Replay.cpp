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

#include <memory>
#include <future>

#include "functional.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "MockParticipant.hpp"
#include "Timer.hpp"

#include "Configuration.hpp"
#include "EndpointAddress.hpp"

#include "EthControllerReplay.hpp"
#include "GenericPublisherReplay.hpp"
#include "GenericSubscriberReplay.hpp"
namespace {

using namespace SilKit;
using namespace SilKit::Core;
using namespace SilKit::Core::Tests;
using namespace SilKit::tracing;
using namespace SilKit::Services::Ethernet;
using namespace SilKit::Services::Can;
using namespace SilKit::Services::generic;

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

auto AService(const IServiceEndpoint* service) -> testing::Matcher<const IServiceEndpoint*>
{
  using namespace testing;
  return AllOf(
    Property(&IServiceEndpoint::GetServiceDescriptor, Eq(service->GetServiceDescriptor()))
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
        SilKit::Util::Timer timer;
        timer.WithPeriod(std::chrono::milliseconds(50), [](const auto) {});
    }

    {
        std::promise<void> done;
        auto isDone = done.get_future();
        SilKit::Util::Timer timer;
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
    void SendMsg(const IServiceEndpoint* from, EthernetFrameEvent&& msg) override
    {
        SendMsg_proxy(from, msg);
    }
    MOCK_METHOD2(SendMsg, void(IServiceEndpoint*, const EthernetFrameEvent&));
    MOCK_METHOD2(SendMsg_proxy, void(const IServiceEndpoint*, const EthernetFrameEvent&));
    MOCK_METHOD2(SendMsg, void(IServiceEndpoint*, const EthernetFrameTransmitEvent&));
    MOCK_METHOD2(SendMsg, void(IServiceEndpoint*, const EthernetStatus&));
    MOCK_METHOD2(SendMsg, void(IServiceEndpoint*, const EthernetSetMode&));
    //  Generic Message calls
    MOCK_METHOD2(SendMsg_proxy, void(const IServiceEndpoint*, const GenericMessage&));
    void SendMsg(const IServiceEndpoint* from, GenericMessage&& msg) override
    {
        SendMsg_proxy(from, msg);
    }

    // CAN
    MOCK_METHOD2(SendMsg_proxy, void(const IServiceEndpoint*, const CanMessage&));
    void SendMsg(const IServiceEndpoint* from, CanMessage&& msg) override
    {
        SendMsg_proxy(from, msg);
    }
};

struct Callbacks
{
    MOCK_METHOD2(ReceiveMessage, void(IEthernetController*, const EthernetFrameEvent&));
    MOCK_METHOD2(ReceiveMessageCan, void(ICanController*, const CanMessage&));
};

struct MockReplayMessage
    : public IReplayMessage
{
    auto Timestamp() const -> std::chrono::nanoseconds override
    {
        return _timestamp;
    }
    auto GetDirection() const -> SilKit::Services::TransmitDirection
    {
        return _direction;
    }

    auto Type() const -> TraceMessageType
    {
        return _type;
    }

    std::chrono::nanoseconds _timestamp{0};
    SilKit::Services::TransmitDirection _direction{ SilKit::Services::TransmitDirection::RX};
    TraceMessageType _type{TraceMessageType::InvalidReplayData};
};

struct MockEthFrame 
    : public MockReplayMessage
    , public EthernetFrame
{
    MockEthFrame()
    {
        SetSourceMac(EthernetMac{1,2,3,4,5,6});
        SetDestinationMac(EthernetMac{7,8,9,0xa,0xb,0xc});
        _type = TraceMessageType::EthernetFrame;
    }
};

TEST(ReplayTest, ethcontroller_replay_config_send)
{
    MockParticipant participant{};

    Config::EthernetController cfg{};

    MockEthFrame msg;

    // Replay Send / Send
    {
        msg._direction = SilKit::Services::TransmitDirection::TX;
        cfg.replay.direction = Config::Replay::Direction::Send;

        EthControllerReplay ctrl{&participant, cfg, participant.GetTimeProvider()};
        ctrl.SetServiceDescriptor(from_endpointAddress(msg._address));
        EXPECT_CALL(participant, SendMsg_proxy(AService(&ctrl), AnEthMessage(msg)))
            .Times(1);
        EXPECT_CALL(participant.mockTimeProvider.mockTime, Now()).Times(1);
        ctrl.ReplayMessage(&msg);
    }

    // Replay Send / Both
    {
        msg._direction = SilKit::Services::TransmitDirection::TX;
        cfg.replay.direction = Config::Replay::Direction::Both;
        EthControllerReplay ctrl{&participant, cfg, participant.GetTimeProvider()};
        ctrl.SetServiceDescriptor(from_endpointAddress(msg._address));
        EXPECT_CALL(participant, SendMsg_proxy(AService(&ctrl), AnEthMessage(msg)))
            .Times(1);
        EXPECT_CALL(participant.mockTimeProvider.mockTime, Now()).Times(1);
        ctrl.ReplayMessage(&msg);
    }

    // Block Send 
    {
        msg._direction = SilKit::Services::TransmitDirection::RX;
        cfg.replay.direction = Config::Replay::Direction::Send;
        EthControllerReplay ctrl{&participant, cfg, participant.GetTimeProvider()};
        ctrl.SetServiceDescriptor(from_endpointAddress(msg._address));
        EXPECT_CALL(participant, SendMsg_proxy(AService(&ctrl), AnEthMessage(msg)))
            .Times(0);
        EXPECT_CALL(participant.mockTimeProvider.mockTime, Now()).Times(0);
        ctrl.ReplayMessage(&msg);
    }

}


TEST(ReplayTest, ethcontroller_replay_config_receive)
{
    Callbacks callbacks;
    MockParticipant participant{};

    Config::EthernetController cfg{};


    MockEthFrame msg;

    msg._address = {1,2};


    // Replay Receive / Receive
    {
        msg._direction = SilKit::Services::TransmitDirection::RX;
        cfg.replay.direction = Config::Replay::Direction::Receive;
        EthControllerReplay controller{&participant, cfg, participant.GetTimeProvider()};
        controller.SetServiceDescriptor(from_endpointAddress({3,4}));
        controller.AddFrameHandler(SilKit::Util::bind_method(&callbacks, &Callbacks::ReceiveMessage));
        EXPECT_CALL(callbacks, ReceiveMessage(A<IEthernetController*>(), AnEthMessage(msg)))
            .Times(1);
        controller.ReplayMessage(&msg);
    }

    // Replay Receive / Both
    {
        msg._direction = SilKit::Services::TransmitDirection::RX;
        cfg.replay.direction = Config::Replay::Direction::Both;
        EthControllerReplay controller{&participant, cfg, participant.GetTimeProvider()};
        controller.SetServiceDescriptor(from_endpointAddress({3,4}));
        controller.AddFrameHandler(SilKit::Util::bind_method(&callbacks, &Callbacks::ReceiveMessage));
        EXPECT_CALL(callbacks, ReceiveMessage(A<IEthernetController*>(), AnEthMessage(msg)))
            .Times(1);
        controller.ReplayMessage(&msg);
    }
    // Block Receive 
    {
        msg._direction = SilKit::Services::TransmitDirection::TX;
        cfg.replay.direction = Config::Replay::Direction::Receive;
        EthControllerReplay controller{&participant, cfg, participant.GetTimeProvider()};
        controller.SetServiceDescriptor(from_endpointAddress({3,4}));
        controller.AddFrameHandler(SilKit::Util::bind_method(&callbacks, &Callbacks::ReceiveMessage));
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

    Config::CanController cfg{};

    MockCanMessage msg;
    msg._address = {1,2};
    // Replay Send / Send
    {
        msg._direction = SilKit::Services::TransmitDirection::TX;
        cfg.replay.direction = Config::Replay::Direction::Send;
    
        CanControllerReplay can{&participant, cfg, participant.GetTimeProvider()};
        can.SetServiceDescriptor(from_endpointAddress(msg._address));
        EXPECT_CALL(participant, SendMsg_proxy(AService(&can), ACanMessage(msg)))
            .Times(1);
        EXPECT_CALL(participant.mockTimeProvider.mockTime, Now()).Times(1);
        can.ReplayMessage(&msg);
    }
    // Replay Send / Both
    {
        msg._direction = SilKit::Services::TransmitDirection::TX;
        cfg.replay.direction = Config::Replay::Direction::Both;
    
        CanControllerReplay can{&participant, cfg, participant.GetTimeProvider()};
        can.SetServiceDescriptor(from_endpointAddress(msg._address));
        EXPECT_CALL(participant, SendMsg_proxy(AService(&can), ACanMessage(msg)))
            .Times(1);
        EXPECT_CALL(participant.mockTimeProvider.mockTime, Now()).Times(1);
        can.ReplayMessage(&msg);
    }
    // Replay Send / Receive
    {
        msg._direction = SilKit::Services::TransmitDirection::TX;
        cfg.replay.direction = Config::Replay::Direction::Receive;
    
        CanControllerReplay can{&participant, cfg, participant.GetTimeProvider()};
        can.SetServiceDescriptor(from_endpointAddress(msg._address));
        EXPECT_CALL(participant, SendMsg_proxy(AService(&can), ACanMessage(msg)))
            .Times(0);
        EXPECT_CALL(participant.mockTimeProvider.mockTime, Now()).Times(0);
        can.ReplayMessage(&msg);
    }
    // Replay Receive / Send
    {
        msg._direction = SilKit::Services::TransmitDirection::RX;
        cfg.replay.direction = Config::Replay::Direction::Send;
    
        CanControllerReplay can{&participant, cfg, participant.GetTimeProvider()};
        EXPECT_CALL(participant, SendMsg_proxy(AService(&can), ACanMessage(msg)))
            .Times(0);
        EXPECT_CALL(participant.mockTimeProvider.mockTime, Now()).Times(0);
        can.SetServiceDescriptor(from_endpointAddress(msg._address));
        can.ReplayMessage(&msg);
    }
    // Replay Receive / Receive
    {
        msg._direction = SilKit::Services::TransmitDirection::RX;
        cfg.replay.direction = Config::Replay::Direction::Receive;
        msg._address = tracing::ReplayEndpointAddress();
    
        CanControllerReplay can{&participant, cfg, participant.GetTimeProvider()};
        EXPECT_CALL(participant, SendMsg_proxy(AService(&can), ACanMessage(msg)))
            .Times(0);
        EXPECT_CALL(participant.mockTimeProvider.mockTime, Now()).Times(1);
        can.SetServiceDescriptor(from_endpointAddress(msg._address));
        can.ReplayMessage(&msg);
    }
}

TEST(ReplayTest, DISABLED_cancontroller_replay_config_receive)
{
    Callbacks callbacks;
    MockParticipant participant{};

    Config::CanController cfg{};


    MockCanMessage msg;

    msg._address = {1,2};

    // Replay Receive / Receive
    {
        msg._direction = SilKit::Services::TransmitDirection::RX;
        cfg.replay.direction = Config::Replay::Direction::Receive;
        CanControllerReplay controller{&participant, cfg, participant.GetTimeProvider()};
        controller.SetServiceDescriptor(from_endpointAddress({3,4}));
        controller.AddFrameHandler(SilKit::Util::bind_method(&callbacks, &Callbacks::ReceiveMessageCan));
        EXPECT_CALL(callbacks, ReceiveMessageCan(A<ICanController*>(), ACanMessage(msg)))
            .Times(1);
        controller.ReplayMessage(&msg);
    }
    
    // Replay Receive / Both
    {
        msg._direction = SilKit::Services::TransmitDirection::RX;
        cfg.replay.direction = Config::Replay::Direction::Both;
        CanControllerReplay controller{&participant, cfg, participant.GetTimeProvider()};
        controller.SetServiceDescriptor(from_endpointAddress({3,4}));
        controller.AddFrameHandler(SilKit::Util::bind_method(&callbacks, &Callbacks::ReceiveMessageCan));
        EXPECT_CALL(callbacks, ReceiveMessageCan(A<ICanController*>(), ACanMessage(msg)))
            .Times(1);
        controller.ReplayMessage(&msg);
    }
    // Block Receive 
    {
        msg._direction = SilKit::Services::TransmitDirection::TX;
        cfg.replay.direction = Config::Replay::Direction::Receive;
        CanControllerReplay controller{&participant, cfg, participant.GetTimeProvider()};
        controller.SetServiceDescriptor(from_endpointAddress({3,4}));
        controller.AddFrameHandler(SilKit::Util::bind_method(&callbacks, &Callbacks::ReceiveMessageCan));
        EXPECT_CALL(callbacks, ReceiveMessageCan(A<ICanController*>(), ACanMessage(msg)))
            .Times(0);
        controller.ReplayMessage(&msg);
    }
}

struct MockGenericMessage
    : public MockReplayMessage
    , public Services::generic::GenericMessage
{
    MockGenericMessage()
    {
        _type = TraceMessageType::GenericMessage;
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

    Config::GenericPort cfg{};

    MockGenericMessage msg;
    msg._address = {1,2};

    // Replay Send / Send
    {
        msg._direction = SilKit::Services::TransmitDirection::TX;
        cfg.replay.direction = Config::Replay::Direction::Send;

        Services::generic::GenericPublisherReplay pub{&participant, cfg, participant.GetTimeProvider()};
        pub.SetServiceDescriptor(from_endpointAddress(msg._address));
        EXPECT_CALL(participant, SendMsg_proxy(AService(&pub), AGenericMessage(msg)))
            .Times(1);
        EXPECT_CALL(participant.mockTimeProvider.mockTime, Now()).Times(1);
        pub.ReplayMessage(&msg);
    }
    // Replay Send / Both
    {
        msg._direction = SilKit::Services::TransmitDirection::TX;
        cfg.replay.direction = Config::Replay::Direction::Both;

        Services::generic::GenericPublisherReplay pub{&participant, cfg, participant.GetTimeProvider()};
        pub.SetServiceDescriptor(from_endpointAddress(msg._address));
        EXPECT_CALL(participant, SendMsg_proxy(AService(&pub), AGenericMessage(msg)))
            .Times(1);
        EXPECT_CALL(participant.mockTimeProvider.mockTime, Now()).Times(1);
        pub.ReplayMessage(&msg);
    }
    // Replay Receive / Both
    {
        msg._direction = SilKit::Services::TransmitDirection::RX;
        cfg.replay.direction = Config::Replay::Direction::Send;

        Services::generic::GenericPublisherReplay pub{&participant, cfg, participant.GetTimeProvider()};
        EXPECT_CALL(participant, SendMsg_proxy(AService(&pub), AGenericMessage(msg)))
            .Times(0);
        EXPECT_CALL(participant.mockTimeProvider.mockTime, Now()).Times(0);
        pub.SetServiceDescriptor(from_endpointAddress(msg._address));
        pub.ReplayMessage(&msg);
    }
    // Replay Receive / Receive
    {
        msg._direction = SilKit::Services::TransmitDirection::RX;
        cfg.replay.direction = Config::Replay::Direction::Receive;

        Services::generic::GenericPublisherReplay pub{&participant, cfg, participant.GetTimeProvider()};
        EXPECT_CALL(participant, SendMsg_proxy(AService(&pub), AGenericMessage(msg)))
            .Times(0);
        EXPECT_CALL(participant.mockTimeProvider.mockTime, Now()).Times(0);
        pub.SetServiceDescriptor(from_endpointAddress(msg._address));
        pub.ReplayMessage(&msg);
    }
}

TEST(ReplayTest, genericsubscriber_replay_config_send)
{
    MockParticipant participant{};

    Config::GenericPort cfg{};

    MockGenericMessage msg;
    msg._address = {1,2};

    // Replay Send / Send
    {
        msg._direction = SilKit::Services::TransmitDirection::TX;
        cfg.replay.direction = Config::Replay::Direction::Send;

        Services::generic::GenericSubscriberReplay sub{&participant, cfg, participant.GetTimeProvider()};
        sub.SetServiceDescriptor(from_endpointAddress(msg._address));
        EXPECT_CALL(participant, ReceiveMsg(msg._address, AGenericMessage(msg)))
            .Times(0);
        EXPECT_CALL(participant.mockTimeProvider.mockTime, Now()).Times(0);
        sub.ReplayMessage(&msg);
    }
    // Replay Send / Both
    {
        msg._direction = SilKit::Services::TransmitDirection::TX;
        cfg.replay.direction = Config::Replay::Direction::Both;

        Services::generic::GenericSubscriberReplay sub{&participant, cfg, participant.GetTimeProvider()};
        sub.SetServiceDescriptor(from_endpointAddress(msg._address));
        EXPECT_CALL(participant, ReceiveMsg(msg._address, AGenericMessage(msg)))
            .Times(0);
        EXPECT_CALL(participant.mockTimeProvider.mockTime, Now()).Times(0);
        sub.ReplayMessage(&msg);
    }
    // Replay Receive / Both
    {
        msg._direction = SilKit::Services::TransmitDirection::RX;
        cfg.replay.direction = Config::Replay::Direction::Both;

        Services::generic::GenericSubscriberReplay sub{&participant, cfg, participant.GetTimeProvider()};
        sub.SetServiceDescriptor(from_endpointAddress({1,3}));
        EXPECT_CALL(participant, ReceiveMsg(msg._address, AGenericMessage(msg)))
            .Times(0);
        EXPECT_CALL(participant.mockTimeProvider.mockTime, Now()).Times(1);
        sub.ReplayMessage(&msg);
    }
    // Replay Receive / Receive
    {
        msg._direction = SilKit::Services::TransmitDirection::RX;
        cfg.replay.direction = Config::Replay::Direction::Receive;

        Services::generic::GenericSubscriberReplay sub{&participant, cfg, participant.GetTimeProvider()};
        sub.SetServiceDescriptor(from_endpointAddress({1,3}));
        EXPECT_CALL(participant, ReceiveMsg(msg._address, AGenericMessage(msg)))
            .Times(0);
        EXPECT_CALL(participant.mockTimeProvider.mockTime, Now()).Times(1);
        sub.ReplayMessage(&msg);
    }
}

} //end anonymous namespace
