// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <memory>
#include <future>

#include "ib/cfg/Config.hpp"
#include "ib/mw/EndpointAddress.hpp"
#include "ib/util/functional.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "MockComAdapter.hpp"
#include "Timer.hpp"

#include "EthControllerReplay.hpp"
#include "GenericPublisherReplay.hpp"

namespace {

using namespace ib;
using namespace ib::mw;
using namespace ib::mw::test;
using namespace ib::tracing;
using namespace ib::sim::eth;
using namespace ib::sim::generic;

using namespace std::chrono_literals;

using testing::A;
using testing::An;

auto AnEthMessage(const EthFrame& msg) -> testing::Matcher<const EthMessage&>
{
    using namespace testing;
    return Field(&EthMessage::ethFrame,
        AllOf(
            Property(&EthFrame::GetDestinationMac, ElementsAreArray(msg.GetDestinationMac()))
            , Property(&EthFrame::GetSourceMac, ElementsAreArray(msg.GetSourceMac()))
            , Property(&EthFrame::GetEtherType, Eq(msg.GetEtherType()))
            , Property(&EthFrame::GetPayloadSize, Eq(msg.GetPayloadSize()))
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

class MockComAdapter : public DummyComAdapter
{
public:
    //Ethernet calls
    void SendIbMessage(EndpointAddress from, EthMessage&& msg) override
    {
        SendIbMessage_proxy(from, msg);
    }
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const EthMessage&));
    MOCK_METHOD2(SendIbMessage_proxy, void(EndpointAddress, const EthMessage&));
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const EthTransmitAcknowledge&));
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const EthStatus&));
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const EthSetMode&));
    //  Generic Message calls
    MOCK_METHOD2(SendIbMessage_proxy, void(EndpointAddress, const GenericMessage&));
    void SendIbMessage(EndpointAddress from, GenericMessage&& msg) override
    {
        SendIbMessage_proxy(from, msg);
    }
};

struct Callbacks
{
    MOCK_METHOD2(ReceiveMessage, void(IEthController*, const EthMessage&));
};

struct MockReplayMessage
    : public extensions::IReplayMessage
{
    auto Timestamp() const -> std::chrono::nanoseconds override
    {
        return _timestamp;
    }
    auto GetDirection() const -> extensions::Direction
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
    extensions::Direction _direction{extensions::Direction::Receive};
    ib::mw::EndpointAddress _address{0, 0};
    extensions::TraceMessageType _type{extensions::TraceMessageType::InvalidReplayData};
};

struct MockEthFrame 
    : public MockReplayMessage
    , public EthFrame
{
    MockEthFrame()
    {
        SetSourceMac(EthMac{1,2,3,4,5,6});
        SetDestinationMac(EthMac{7,8,9,0xa,0xb,0xc});
        _type = extensions::TraceMessageType::EthFrame;
    }
};

TEST(ReplayTest, ethcontroller_replay_config_send)
{
    MockComAdapter comAdapter{};

    cfg::EthernetController cfg{};

    MockEthFrame msg;
    msg._address = {1,2};

    // Replay Send / Send
    {
        msg._direction = extensions::Direction::Send;
        cfg.replay.direction = cfg::Replay::Direction::Send;

        EthControllerReplay ctrl{&comAdapter, cfg, comAdapter.GetTimeProvider()};
        ctrl.SetEndpointAddress(msg._address);
        EXPECT_CALL(comAdapter, SendIbMessage_proxy(msg._address, AnEthMessage(msg)))
            .Times(1);
        EXPECT_CALL(comAdapter.mockTimeProvider.mockTime, Now()).Times(1);
        ctrl.ReplayMessage(&msg);
    }

    // Replay Send / Both
    {
        msg._direction = extensions::Direction::Send;
        cfg.replay.direction = cfg::Replay::Direction::Both;
        EthControllerReplay ctrl{&comAdapter, cfg, comAdapter.GetTimeProvider()};
        ctrl.SetEndpointAddress(msg._address);
        EXPECT_CALL(comAdapter, SendIbMessage_proxy(msg._address, AnEthMessage(msg)))
            .Times(1);
        EXPECT_CALL(comAdapter.mockTimeProvider.mockTime, Now()).Times(1);
        ctrl.ReplayMessage(&msg);
    }

    // Block Send 
    {
        msg._direction = extensions::Direction::Receive;
        cfg.replay.direction = cfg::Replay::Direction::Send;
        EthControllerReplay ctrl{&comAdapter, cfg, comAdapter.GetTimeProvider()};
        ctrl.SetEndpointAddress(msg._address);
        EXPECT_CALL(comAdapter, SendIbMessage_proxy(msg._address, AnEthMessage(msg)))
            .Times(0);
        EXPECT_CALL(comAdapter.mockTimeProvider.mockTime, Now()).Times(0);
        ctrl.ReplayMessage(&msg);
    }

}


TEST(ReplayTest, ethcontroller_replay_config_receive)
{
    Callbacks callbacks;
    MockComAdapter comAdapter{};

    cfg::EthernetController cfg{};


    MockEthFrame msg;

    msg._address = {1,2};


    // Replay Receive / Receive
    {
        msg._direction = extensions::Direction::Receive;
        cfg.replay.direction = cfg::Replay::Direction::Receive;
        EthControllerReplay controller{&comAdapter, cfg, comAdapter.GetTimeProvider()};
        controller.SetEndpointAddress({3,4});
        controller.RegisterReceiveMessageHandler(ib::util::bind_method(&callbacks, &Callbacks::ReceiveMessage));
        EXPECT_CALL(callbacks, ReceiveMessage(A<IEthController*>(), AnEthMessage(msg)))
            .Times(1);
        controller.ReplayMessage(&msg);
    }

    // Replay Receive / Both
    {
        msg._direction = extensions::Direction::Receive;
        cfg.replay.direction = cfg::Replay::Direction::Both;
        EthControllerReplay controller{&comAdapter, cfg, comAdapter.GetTimeProvider()};
        controller.SetEndpointAddress({3,4});
        controller.RegisterReceiveMessageHandler(ib::util::bind_method(&callbacks, &Callbacks::ReceiveMessage));
        EXPECT_CALL(callbacks, ReceiveMessage(A<IEthController*>(), AnEthMessage(msg)))
            .Times(1);
        controller.ReplayMessage(&msg);
    }
    // Block Receive 
    {
        msg._direction = extensions::Direction::Send;
        cfg.replay.direction = cfg::Replay::Direction::Receive;
        EthControllerReplay controller{&comAdapter, cfg, comAdapter.GetTimeProvider()};
        controller.SetEndpointAddress({3,4});
        controller.RegisterReceiveMessageHandler(ib::util::bind_method(&callbacks, &Callbacks::ReceiveMessage));
        EXPECT_CALL(callbacks, ReceiveMessage(A<IEthController*>(), AnEthMessage(msg)))
            .Times(0);
        controller.ReplayMessage(&msg);
    }
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
    MockComAdapter comAdapter{};

    cfg::GenericPort cfg{};

    MockGenericMessage msg;
    msg._address = {1,2};

    // Replay Send / Send
    {
        msg._direction = extensions::Direction::Send;
        cfg.replay.direction = cfg::Replay::Direction::Send;

        sim::generic::GenericPublisherReplay pub{&comAdapter, cfg, comAdapter.GetTimeProvider()};
        pub.SetEndpointAddress(msg._address);
        EXPECT_CALL(comAdapter, SendIbMessage_proxy(msg._address, AGenericMessage(msg)))
            .Times(1);
        EXPECT_CALL(comAdapter.mockTimeProvider.mockTime, Now()).Times(1);
        pub.ReplayMessage(&msg);
    }
    // Replay Send / Both
    {
        msg._direction = extensions::Direction::Send;
        cfg.replay.direction = cfg::Replay::Direction::Both;

        sim::generic::GenericPublisherReplay pub{&comAdapter, cfg, comAdapter.GetTimeProvider()};
        pub.SetEndpointAddress(msg._address);
        EXPECT_CALL(comAdapter, SendIbMessage_proxy(msg._address, AGenericMessage(msg)))
            .Times(1);
        EXPECT_CALL(comAdapter.mockTimeProvider.mockTime, Now()).Times(1);
        pub.ReplayMessage(&msg);
    }
    // Replay Send / Both
    {
        msg._direction = extensions::Direction::Receive;
        cfg.replay.direction = cfg::Replay::Direction::Send;

        sim::generic::GenericPublisherReplay pub{&comAdapter, cfg, comAdapter.GetTimeProvider()};
        pub.SetEndpointAddress(msg._address);
        EXPECT_THROW(pub.ReplayMessage(&msg), std::runtime_error) << "Only sending should be supported";
    }
}

} //end anonymous namespace
