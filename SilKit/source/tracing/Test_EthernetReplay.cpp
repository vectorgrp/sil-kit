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

#include "functional.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <fmt/format.h>

#include "MockParticipant.hpp"

#include "Configuration.hpp"
#include "EndpointAddress.hpp"

#include "EthController.hpp"
#include "EthDatatypeUtils.hpp"

namespace {

using namespace SilKit;
using namespace SilKit::Core;
using namespace SilKit::Core::Tests;
using namespace SilKit::Tracing;
using namespace SilKit::Services::Ethernet;

using namespace std::chrono_literals;

using testing::A;
using testing::An;

auto AService(const IServiceEndpoint* service) -> testing::Matcher<const IServiceEndpoint*>
{
    using namespace testing;
    return AllOf(Property(&IServiceEndpoint::GetServiceDescriptor, Eq(service->GetServiceDescriptor())));
}

MATCHER_P(SpanEq, other, fmt::format("spans {}", negation ? "are different" : "are equal"))
{
    return SilKit::Util::ItemsAreEqual(arg, other);
}

MATCHER_P(AsSpanMatches, spanMatcher, "")
{
    using namespace testing;
    return ExplainMatchResult(spanMatcher, arg.AsSpan(), result_listener);
}

auto AnEthernetFrameEvent(const EthernetFrame& msg) -> testing::Matcher<const EthernetFrameEvent&>
{
    using namespace testing;
    return Field(&EthernetFrameEvent::frame, AllOf(Field(&EthernetFrame::raw, SpanEq(msg.raw))));
}

auto AWireEthernetFrameEvent(const EthernetFrame& msg) -> testing::Matcher<const WireEthernetFrameEvent&>
{
    using namespace testing;
    return Field(&WireEthernetFrameEvent::frame, AllOf(Field(&WireEthernetFrame::raw, AsSpanMatches(SpanEq(msg.raw)))));
}

class MockParticipant : public DummyParticipant
{
public:
    MOCK_METHOD(void, SendMsg,
                (const IServiceEndpoint* /*from*/, const Services::Ethernet::WireEthernetFrameEvent& /*msg*/),
                (override));

    MOCK_METHOD(void, SendMsg,
                (const IServiceEndpoint* /*from*/, const Services::Ethernet::EthernetFrameTransmitEvent& /*msg*/),
                (override));

    MOCK_METHOD(void, SendMsg, (const IServiceEndpoint* /*from*/, const Services::Ethernet::EthernetStatus& /*msg*/),
                (override));

    MOCK_METHOD(void, SendMsg, (const IServiceEndpoint* /*from*/, const Services::Ethernet::EthernetSetMode& /*msg*/),
                (override));
};

struct Callbacks
{
    MOCK_METHOD2(ReceiveMessage, void(IEthernetController*, const EthernetFrameEvent&));
};

struct MockReplayMessage : public IReplayMessage
{
    auto Timestamp() const -> std::chrono::nanoseconds override { return _timestamp; }
    auto GetDirection() const -> SilKit::Services::TransmitDirection override { return _direction; }
    auto ServiceDescriptorStr() const -> std::string override { return _serviceDescriptorStr; }
    auto EndpointAddress() const -> SilKit::Core::EndpointAddress override { return _endpointAddress; }
    auto Type() const -> TraceMessageType override { return _type; }

    std::chrono::nanoseconds _timestamp{0};
    SilKit::Services::TransmitDirection _direction{SilKit::Services::TransmitDirection::RX};
    TraceMessageType _type{TraceMessageType::InvalidReplayData};
    std::string _serviceDescriptorStr{ "ServiceDescriptorString" };
    SilKit::Core::EndpointAddress _endpointAddress; //! Deprecated
};

struct MockEthFrame
    : public MockReplayMessage
    , public WireEthernetFrame
    , public EthernetFrame
{
    MockEthFrame()
    {
        std::vector<uint8_t> rawData;

        SetSourceMac(rawData, EthernetMac{1, 2, 3, 4, 5, 6});
        SetDestinationMac(rawData, EthernetMac{7, 8, 9, 0xa, 0xb, 0xc});
        _type = TraceMessageType::EthernetFrame;

        static_cast<WireEthernetFrame*>(this)->raw = SilKit::Util::SharedVector<uint8_t>{rawData};
        static_cast<EthernetFrame*>(this)->raw = static_cast<WireEthernetFrame*>(this)->raw.AsSpan();
    }
};
struct Test_EthernetReplay : public testing::Test
{
    ServiceDescriptor _serviceDescriptor{ "EthernetReplay", "Eth0", "EthController0", 2};
    ServiceDescriptor _otherServiceDescriptor{ "EthernetReplay2", "Eth1", "EthController2", 4 };
};

TEST_F(Test_EthernetReplay, ethcontroller_replay_config_send)
{
    MockParticipant participant{};

    Config::EthernetController cfg{};
    cfg.replay.useTraceSource = "ReplayTest.TraceSource";

    MockEthFrame msg;

    // Replay Send / Send
    {
        msg._direction = SilKit::Services::TransmitDirection::TX;
        cfg.replay.direction = Config::Replay::Direction::Send;

        EthController ctrl{&participant, cfg, participant.GetTimeProvider()};
        ctrl.SetServiceDescriptor(_serviceDescriptor);

        EXPECT_CALL(participant.mockTimeProvider, Now()).Times(1);
        ctrl.Activate();

        EXPECT_CALL(participant, SendMsg(AService(&ctrl), AWireEthernetFrameEvent(msg))).Times(1);
        EXPECT_CALL(participant.mockTimeProvider, Now()).Times(2);
        ctrl.ReplayMessage(&msg);
    }

    // Replay Send / Both
    {
        msg._direction = SilKit::Services::TransmitDirection::TX;
        cfg.replay.direction = Config::Replay::Direction::Both;

        EthController ctrl{&participant, cfg, participant.GetTimeProvider()};
        ctrl.SetServiceDescriptor(_serviceDescriptor);

        EXPECT_CALL(participant.mockTimeProvider, Now()).Times(1);
        ctrl.Activate();

        EXPECT_CALL(participant, SendMsg(AService(&ctrl), AWireEthernetFrameEvent(msg))).Times(1);
        EXPECT_CALL(participant.mockTimeProvider, Now()).Times(2);
        ctrl.ReplayMessage(&msg);
    }

    // Block Send
    {
        msg._direction = SilKit::Services::TransmitDirection::RX;
        cfg.replay.direction = Config::Replay::Direction::Send;

        EthController ctrl{&participant, cfg, participant.GetTimeProvider()};
        ctrl.SetServiceDescriptor(_serviceDescriptor);

        EXPECT_CALL(participant.mockTimeProvider, Now()).Times(1);
        ctrl.Activate();

        EXPECT_CALL(participant, SendMsg(AService(&ctrl), AWireEthernetFrameEvent(msg))).Times(0);
        EXPECT_CALL(participant.mockTimeProvider, Now()).Times(0);
        ctrl.ReplayMessage(&msg);
    }
}

TEST_F(Test_EthernetReplay, ethcontroller_replay_config_receive)
{
    Callbacks callbacks;
    MockParticipant participant{};

    ON_CALL(callbacks, ReceiveMessage(A<IEthernetController*>(), A<const EthernetFrameEvent&>()))
        .WillByDefault([](IEthernetController* controller, const EthernetFrameEvent& ethernetFrameEvent) {
            SILKIT_UNUSED_ARG(controller);
            SILKIT_UNUSED_ARG(ethernetFrameEvent);
        });

    Config::EthernetController cfg{};
    cfg.replay.useTraceSource = "ReplayTest.TraceSource";

    MockEthFrame msg;


    // Replay Receive / Receive
    {
        msg._direction = SilKit::Services::TransmitDirection::RX;
        cfg.replay.direction = Config::Replay::Direction::Receive;

        EthController controller{&participant, cfg, participant.GetTimeProvider()};
        controller.SetServiceDescriptor({ "p1","n1", "c1", 4 });
        controller.AddFrameHandler(SilKit::Util::bind_method(&callbacks, &Callbacks::ReceiveMessage));

        EXPECT_CALL(callbacks, ReceiveMessage(A<IEthernetController*>(), AnEthernetFrameEvent(msg))).Times(1);
        controller.ReplayMessage(&msg);
    }

    // Replay Receive / Both
    {
        msg._direction = SilKit::Services::TransmitDirection::RX;
        cfg.replay.direction = Config::Replay::Direction::Both;

        EthController controller{&participant, cfg, participant.GetTimeProvider()};
        controller.SetServiceDescriptor(_otherServiceDescriptor);
        controller.AddFrameHandler(SilKit::Util::bind_method(&callbacks, &Callbacks::ReceiveMessage));

        EXPECT_CALL(callbacks, ReceiveMessage(A<IEthernetController*>(), AnEthernetFrameEvent(msg))).Times(1);
        controller.ReplayMessage(&msg);
    }

    // Block Receive
    {
        msg._direction = SilKit::Services::TransmitDirection::TX;
        cfg.replay.direction = Config::Replay::Direction::Receive;

        EthController controller{&participant, cfg, participant.GetTimeProvider()};
        controller.SetServiceDescriptor(_otherServiceDescriptor);
        controller.AddFrameHandler(SilKit::Util::bind_method(&callbacks, &Callbacks::ReceiveMessage));

        EXPECT_CALL(callbacks, ReceiveMessage(A<IEthernetController*>(), AnEthernetFrameEvent(msg))).Times(0);
        controller.ReplayMessage(&msg);
    }
}

} // namespace
