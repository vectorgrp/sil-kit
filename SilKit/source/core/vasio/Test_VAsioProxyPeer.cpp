// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "core/vasio/VAsioProxyPeer.hpp"

#include <vector>

#include "core/vasio/SharedSerializedMessage.hpp"
#include "core/vasio/mock/MockVAsioPeer.hpp"
#include "services/logging/MockLogger.hpp"
#include "wire/pubsub/WireDataMessages.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace {

using namespace SilKit::Core;
using SilKit::Services::PubSub::WireDataMessageEvent;
using testing::_;
using testing::NiceMock;

struct NullPeerListener : IVAsioPeerListener
{
    void OnSocketData(IVAsioPeer*, SerializedMessage&&) override {}
    void OnPeerShutdown(IVAsioPeer*) override {}
};

struct Forwarded
{
    ProxyMessage message;
    MessageAggregationKind aggregationKind;
};

class Test_VAsioProxyPeer : public testing::Test
{
protected:
    Test_VAsioProxyPeer()
    {
        registryInfo.participantName = "Registry";
        destinationInfo.participantName = "Destination";

        ON_CALL(registry, GetInfo()).WillByDefault(testing::ReturnRef(registryInfo));
        ON_CALL(registry, SendSilKitMsg(testing::Matcher<SerializedMessage>(_)))
            .WillByDefault([this](SerializedMessage message) {
            const auto aggregationKind = message.GetAggregationKind();
            forwarded.push_back(Forwarded{message.Deserialize<ProxyMessage>(), aggregationKind});
        });

        proxy = std::make_unique<VAsioProxyPeer>(&listener, "Source", destinationInfo, &registry, &logger);
    }

    const EndpointAddress from{7, 9};

    NiceMock<SilKit::Services::Logging::MockLogger> logger;
    NullPeerListener listener;
    VAsioPeerInfo registryInfo;
    VAsioPeerInfo destinationInfo;
    NiceMock<MockVAsioPeer> registry;
    std::unique_ptr<VAsioProxyPeer> proxy;
    std::vector<Forwarded> forwarded;
};

TEST_F(Test_VAsioProxyPeer, shared_and_plain_send_forward_the_same_payload)
{
    for (const size_t payloadSize : {size_t{3}, size_t{1024}})
    {
        forwarded.clear();

        std::vector<uint8_t> payload(payloadSize);
        for (size_t i = 0; i < payloadSize; ++i)
        {
            payload[i] = static_cast<uint8_t>(i * 31 + 1);
        }
        const WireDataMessageEvent event{std::chrono::nanoseconds{5}, payload};
        const SharedSerializedMessage shared{event, from};

        proxy->SendSilKitMsg(shared, EndpointId{77});
        proxy->SendSilKitMsg(SerializedMessage{event, from, EndpointId{77}});

        ASSERT_EQ(forwarded.size(), 2u);
        const auto& viaShared = forwarded[0];
        const auto& viaPlain = forwarded[1];

        EXPECT_EQ(viaShared.message.payload, SerializedMessage(event, from, EndpointId{77}).ReleaseStorage())
            << "payload size " << payloadSize;
        EXPECT_EQ(viaShared.message.payload, viaPlain.message.payload);
        EXPECT_EQ(viaShared.message.source, "Source");
        EXPECT_EQ(viaShared.message.destination, "Destination");
        EXPECT_EQ(viaShared.message.source, viaPlain.message.source);
        EXPECT_EQ(viaShared.message.destination, viaPlain.message.destination);
        EXPECT_EQ(viaShared.aggregationKind, MessageAggregationKind::UserDataMessage);
        EXPECT_EQ(viaShared.aggregationKind, viaPlain.aggregationKind);
    }
}

} // namespace
