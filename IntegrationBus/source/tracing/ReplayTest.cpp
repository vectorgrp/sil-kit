// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <memory>

#include "ib/cfg/Config.hpp"
#include "ib/mw/EndpointAddress.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "MockComAdapter.hpp"
#include "Timer.hpp"
#include "ReplayController.hpp"

namespace {

using namespace ib;
using namespace ib::mw;
using namespace ib::mw::test;
using namespace ib::tracing;
using namespace ib::sim::eth;

using testing::A;
using testing::An;

TEST(ReplayTest, ensure_util_timer_works)
{

    {
        ib::util::Timer timer;
        timer.WithPeriod(std::chrono::milliseconds(50), [](const auto) {});
    }

    {
        ib::util::Timer timer;
        auto numCalls = 0u;
        auto cb = [&](const auto now) {
            numCalls++;
            if (numCalls == 5)
            {
                timer.Stop();
            }
        };
        timer.WithPeriod(std::chrono::milliseconds(50), cb);
        ASSERT_TRUE(timer.IsActive());
        std::this_thread::sleep_for(std::chrono::milliseconds(50 * (5 + 1)));
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
};

struct MockReplayMessage 
    : public extensions::IReplayMessage
    , public EthFrame
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

TEST(ReplayTest, ethcontroller_replay_config_send)
{
    MockComAdapter comAdapter{};

    cfg::EthernetController cfg{};
    cfg.replay.direction = cfg::Replay::Direction::Send;

    EthControllerReplay ctrl{&comAdapter, cfg, comAdapter.GetTimeProvider()};

    MockReplayMessage msg;
    msg._address = {1,2};
    msg._type = extensions::TraceMessageType::EthFrame;

    msg._direction = extensions::Direction::Send;
    EXPECT_CALL(comAdapter, SendIbMessage_proxy(msg._address, An<const EthMessage&>()))
        .Times(1);
    EXPECT_CALL(comAdapter.mockTimeProvider.mockTime, Now()).Times(1);
    ctrl.ReplayMessage(&msg);

}

}
