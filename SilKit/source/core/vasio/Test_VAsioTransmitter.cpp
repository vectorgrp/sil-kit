// SPDX-FileCopyrightText: 2025 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "core/vasio/VAsioTransmitter.hpp"

#include <cstring>
#include <memory>
#include <vector>

#include "core/vasio/mock/MockVAsioPeer.hpp"
#include "services/logging/MockLogger.hpp"
#include "wire/pubsub/WireDataMessages.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace {

using namespace SilKit::Core;
using namespace SilKit::Services::PubSub;
using testing::NiceMock;

struct MockServiceEndpoint : IServiceEndpoint
{
    ServiceDescriptor descriptor;

    MockServiceEndpoint()
    {
        descriptor.SetParticipantNameAndComputeId("SenderParticipant");
        descriptor.SetServiceId(2);
    }

    void SetServiceDescriptor(const ServiceDescriptor& value) override
    {
        descriptor = value;
    }

    auto GetServiceDescriptor() const -> const ServiceDescriptor& override
    {
        return descriptor;
    }
};

//! What one peer observed, captured while the shared message is still alive.
struct Observation
{
    const void* messageIdentity{nullptr};
    const void* bodyData{nullptr};
    size_t bodySize{0};
    size_t totalSize{0};
    EndpointId remoteIdx{0};
    //! the remote index as it would appear on the wire, read back from the patched header
    EndpointId patchedRemoteIndex{0};
    std::vector<uint8_t> header;
};

//! Reproduce what VAsioPeer does with the shared message for one peer.
auto Observe(const SharedSerializedMessage& msg, EndpointId remoteIdx) -> Observation
{
    Observation observation;
    observation.messageIdentity = &msg;
    observation.bodyData = msg.Body().AsSpan().data();
    observation.bodySize = msg.Body().size();
    observation.totalSize = msg.TotalSize();
    observation.remoteIdx = remoteIdx;

    observation.header.assign(msg.Header().begin(), msg.Header().end());
    std::memcpy(observation.header.data() + msg.RemoteIndexOffset(), &remoteIdx, sizeof(remoteIdx));
    std::memcpy(&observation.patchedRemoteIndex, observation.header.data() + msg.RemoteIndexOffset(),
                sizeof(observation.patchedRemoteIndex));

    return observation;
}

TEST(Test_VAsioTransmitter, serializes_once_and_shares_the_body_across_peers)
{
    NiceMock<SilKit::Services::Logging::MockLogger> logger;
    VAsioTransmitter<WireDataMessageEvent> transmitter{&logger};

    constexpr size_t numPeers = 4;

    std::vector<VAsioPeerInfo> peerInfos(numPeers);
    for (size_t i = 0; i < numPeers; ++i)
    {
        peerInfos[i].participantName = "Peer" + std::to_string(i);
        peerInfos[i].participantId = i + 1;
    }

    std::vector<Observation> observations;
    std::vector<std::unique_ptr<NiceMock<MockVAsioPeer>>> peers;

    for (size_t i = 0; i < numPeers; ++i)
    {
        auto peer = std::make_unique<NiceMock<MockVAsioPeer>>();
        ON_CALL(*peer, GetInfo()).WillByDefault(testing::ReturnRef(peerInfos[i]));

        EXPECT_CALL(*peer, SendSilKitMsg(testing::Matcher<const SharedSerializedMessage&>(testing::_), testing::_))
            .Times(1)
            .WillOnce([&observations](const SharedSerializedMessage& msg, EndpointId remoteIdx) {
            observations.push_back(Observe(msg, remoteIdx));
        });

        // fan-out must no longer go through the per-peer serializing overload
        EXPECT_CALL(*peer, SendSilKitMsg(testing::Matcher<SerializedMessage>(testing::_))).Times(0);

        peers.emplace_back(std::move(peer));
    }

    for (size_t i = 0; i < numPeers; ++i)
    {
        transmitter.AddRemoteReceiver(peers[i].get(), static_cast<EndpointId>(100 + i));
    }
    ASSERT_EQ(transmitter.GetNumberOfRemoteReceivers(), numPeers);

    MockServiceEndpoint from;
    const std::vector<uint8_t> payload(512, 0x5A);
    WireDataMessageEvent event{std::chrono::nanoseconds{1234}, payload};

    transmitter.ReceiveMsg(&from, event);

    ASSERT_EQ(observations.size(), numPeers);

    for (size_t i = 1; i < numPeers; ++i)
    {
        // one serialized message object was handed to every peer
        EXPECT_EQ(observations[i].messageIdentity, observations[0].messageIdentity);
        // and every peer views the very same body bytes, so the payload was not copied per peer
        EXPECT_EQ(observations[i].bodyData, observations[0].bodyData);
        EXPECT_EQ(observations[i].bodySize, observations[0].bodySize);
    }

    // the payload is actually in the shared body
    EXPECT_GE(observations[0].bodySize, payload.size());

    for (size_t i = 0; i < numPeers; ++i)
    {
        const auto expected = static_cast<EndpointId>(100 + i);
        EXPECT_EQ(observations[i].remoteIdx, expected);
        // each peer's private header carries its own remote index
        EXPECT_EQ(observations[i].patchedRemoteIndex, expected);
    }

    // the per-peer headers differ only in the remote index
    for (size_t i = 1; i < numPeers; ++i)
    {
        ASSERT_EQ(observations[i].header.size(), observations[0].header.size());
        EXPECT_NE(observations[i].header, observations[0].header);
    }
}

TEST(Test_VAsioTransmitter, header_and_body_partition_the_message)
{
    const std::vector<uint8_t> payload(256, 0x33);
    WireDataMessageEvent event{std::chrono::nanoseconds{7}, payload};

    const SharedSerializedMessage shared{event, EndpointAddress{1, 2}};

    EXPECT_EQ(shared.HeaderSize() + shared.Body().size(), shared.TotalSize());
    EXPECT_EQ(shared.Body().AsSpan().data(), shared.Header().data() + shared.HeaderSize());
    EXPECT_LE(shared.RemoteIndexOffset() + sizeof(EndpointId), shared.HeaderSize());

    // copying the body handle shares the bytes rather than copying them
    auto bodyCopy = shared.Body();
    EXPECT_EQ(bodyCopy.AsSpan().data(), shared.Body().AsSpan().data());
}

TEST(Test_VAsioTransmitter, does_not_serialize_without_remote_receivers)
{
    NiceMock<SilKit::Services::Logging::MockLogger> logger;
    VAsioTransmitter<WireDataMessageEvent> transmitter{&logger};

    MockServiceEndpoint from;
    WireDataMessageEvent event{std::chrono::nanoseconds{1}, std::vector<uint8_t>{1, 2, 3}};

    EXPECT_EQ(transmitter.GetNumberOfRemoteReceivers(), 0u);
    EXPECT_NO_THROW(transmitter.ReceiveMsg(&from, event));
}

} // namespace
