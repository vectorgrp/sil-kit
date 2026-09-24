// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "core/vasio/SerializedMessage.hpp"
#include "wire/can/WireCanMessages.hpp"
#include "wire/ethernet/WireEthernetMessages.hpp"
#include "wire/flexray/WireFlexrayMessages.hpp"
#include "wire/pubsub/WireDataMessages.hpp"
#include "wire/rpc/WireRpcMessages.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>
#include <array>
#include <string>

#include "gtest/gtest.h"

#pragma pack(push, 1)
struct PackedHandshake
{
    //Implicit message size
    uint32_t messageSize;
    // implicit message kind
    uint8_t messageKind;
    uint8_t registryMessageKind;
    // VasioMsgHeader
    std::array<uint8_t, 4> preambel;
    uint16_t versionHigh;
    uint16_t versionLow;
    //VAsioPeerInfo
    uint32_t participantNameSize;
    std::array<uint8_t, sizeof("SerdesTest") - 1> participantName; //without trailing '\0' byte
    uint64_t participantId;
    //VAsioPeerInfo::acceptorUris
    uint32_t acceptorUrisSize;
    //[0]
    uint32_t acceptorUri0Size;
    std::array<uint8_t, sizeof("https://example.com:1234") - 1> acceptorUri0;
    uint32_t capabilitiesSize;
    //capabilities empty
    uint32_t simulationNameSize;
    std::array<uint8_t, sizeof("test/sim") - 1> simulationName;
};
#pragma pack(pop)

using namespace SilKit::Core;

TEST(Test_SerializedMessage, packed_handshake_message)
{
    //test if network layout of connection handshake changed
    ParticipantAnnouncement announcement;
    announcement.peerInfo.participantId = 1234;
    announcement.peerInfo.participantName = "SerdesTest";
    announcement.peerInfo.acceptorUris = {"https://example.com:1234"};
    announcement.simulationName = "test/sim";

    // check that the default-constructed announcement contains the correct preamble
    ASSERT_EQ(announcement.messageHeader.preamble, REGISTRY_MESSAGE_HEADER_PREAMBLE_VALUE);

    SerializedMessage msg{announcement};
    auto blob = msg.ReleaseStorage();
    const auto* ptr = reinterpret_cast<const PackedHandshake*>(blob.data());

    ASSERT_EQ(blob.size(), sizeof(PackedHandshake));

    ASSERT_EQ(blob.size(), ptr->messageSize);
    ASSERT_EQ(ptr->messageKind, (uint8_t)VAsioMsgKind::SilKitRegistryMessage);
    ASSERT_EQ(ptr->registryMessageKind, (uint8_t)RegistryMessageKind::ParticipantAnnouncement);

    // check that the serialized preamble contains the exact bytes
    ASSERT_EQ(ptr->preambel, REGISTRY_MESSAGE_HEADER_PREAMBLE_BYTES);
    ASSERT_EQ(ptr->versionHigh, announcement.messageHeader.versionHigh);
    ASSERT_EQ(ptr->versionLow, announcement.messageHeader.versionLow);

    ASSERT_EQ(ptr->participantNameSize, announcement.peerInfo.participantName.size());

    auto to_string = [](const auto& data, auto size) { return std::string{(const char*)data.data(), size}; };

    ASSERT_EQ(to_string(ptr->participantName, ptr->participantNameSize), announcement.peerInfo.participantName);

    ASSERT_EQ(ptr->participantId, announcement.peerInfo.participantId);

    ASSERT_EQ(ptr->acceptorUrisSize, announcement.peerInfo.acceptorUris.size());

    ASSERT_EQ(to_string(ptr->acceptorUri0, ptr->acceptorUri0Size), announcement.peerInfo.acceptorUris.at(0));

    ASSERT_EQ(ptr->simulationNameSize, announcement.simulationName.size());
    ASSERT_EQ(to_string(ptr->simulationName, ptr->simulationNameSize), announcement.simulationName);
}

#pragma pack(push, 1)
struct PackedSimMessage
{
    // implicit message size
    uint32_t messageSize;
    // implicit message kind
    uint8_t messageKind;
    // remoteIndex is the only field that differs between receivers of the same message
    uint64_t remoteIndex;
    // EndpointAddress of the sender
    uint64_t endpointParticipant;
    uint64_t endpointEndpoint;
    // WireDataMessageEvent::data
    uint32_t payloadSize;
    std::array<uint8_t, 4> payload;
    // WireDataMessageEvent::timestamp
    int64_t timestamp;
};
#pragma pack(pop)

TEST(Test_SerializedMessage, packed_sim_message)
{
    // Test if the network layout of a payload carrying simulation message changed.
    // The offsets asserted here are relied upon when a single serialized message is shared
    // between multiple peers and only remoteIndex is patched per peer.
    const std::vector<uint8_t> payload{0x11, 0x22, 0x33, 0x44};
    SilKit::Services::PubSub::WireDataMessageEvent event{std::chrono::nanoseconds{424242}, payload};

    const EndpointAddress endpointAddress{7, 9};
    const EndpointId remoteIndex{5};

    SerializedMessage msg{event, endpointAddress, remoteIndex};

    // The offsets recorded during header serialization must match the actual wire layout.
    ASSERT_EQ(msg.GetRemoteIndexOffset(), offsetof(PackedSimMessage, remoteIndex));
    ASSERT_EQ(msg.GetHeaderSize(), offsetof(PackedSimMessage, payloadSize));

    auto blob = msg.ReleaseStorage();

    ASSERT_EQ(blob.size(), sizeof(PackedSimMessage));

    const auto* ptr = reinterpret_cast<const PackedSimMessage*>(blob.data());

    ASSERT_EQ(ptr->messageSize, blob.size());
    ASSERT_EQ(ptr->messageKind, (uint8_t)VAsioMsgKind::SilKitMwMsg);
    ASSERT_EQ(ptr->remoteIndex, remoteIndex);
    ASSERT_EQ(ptr->endpointParticipant, endpointAddress.participant);
    ASSERT_EQ(ptr->endpointEndpoint, endpointAddress.endpoint);
    ASSERT_EQ(ptr->payloadSize, payload.size());
    ASSERT_EQ(ptr->payload.at(0), payload.at(0));
    ASSERT_EQ(ptr->payload.at(3), payload.at(3));
    ASSERT_EQ(ptr->timestamp, event.timestamp.count());
}

TEST(Test_SerializedMessage, deserialized_payload_aliases_a_shared_blob)
{
    // A payload deserialized from a shared blob must view the blob's bytes rather than a copy.
    const std::vector<uint8_t> payload{0xDE, 0xAD, 0xBE, 0xEF};
    SilKit::Services::PubSub::WireDataMessageEvent event{std::chrono::nanoseconds{7}, payload};

    SerializedMessage sending{event, EndpointAddress{1, 2}, EndpointId{3}};
    auto blobBytes = sending.ReleaseStorage();

    auto blob = std::make_shared<const std::vector<uint8_t>>(std::move(blobBytes));
    const auto* blobStart = blob->data();
    const auto blobEnd = blobStart + blob->size();

    SerializedMessage receiving{SilKit::Util::MakeSharedSpan(blob, 0, blob->size())};
    auto received = receiving.Deserialize<SilKit::Services::PubSub::WireDataMessageEvent>();

    const auto* payloadData = received.data.AsSpan().data();

    ASSERT_EQ(received.data.size(), payload.size());
    // the payload points into the blob, so no copy was made
    ASSERT_GE(payloadData, blobStart);
    ASSERT_LE(payloadData + received.data.size(), blobEnd);
    ASSERT_TRUE(SilKit::Util::ItemsAreEqual(received.data.AsSpan(), SilKit::Util::ToSpan(payload)));
}

TEST(Test_SerializedMessage, aliased_payload_outlives_the_serialized_message)
{
    const std::vector<uint8_t> payload{1, 2, 3, 4, 5, 6, 7, 8};
    SilKit::Services::PubSub::WireDataMessageEvent event{std::chrono::nanoseconds{11}, payload};

    SerializedMessage sending{event, EndpointAddress{1, 2}, EndpointId{3}};
    auto blob = std::make_shared<const std::vector<uint8_t>>(sending.ReleaseStorage());

    SilKit::Services::PubSub::WireDataMessageEvent received{};
    {
        SerializedMessage receiving{SilKit::Util::MakeSharedSpan(blob, 0, blob->size())};
        received = receiving.Deserialize<SilKit::Services::PubSub::WireDataMessageEvent>();
    }
    // drop the last external handle; only the payload keeps the blob alive now
    blob.reset();

    ASSERT_EQ(received.data.size(), payload.size());
    ASSERT_TRUE(SilKit::Util::ItemsAreEqual(received.data.AsSpan(), SilKit::Util::ToSpan(payload)));
}

TEST(Test_SerializedMessage, deserializing_from_a_private_blob_still_copies)
{
    // Without a shared blob the payload must be copied out, since the storage is not kept alive.
    const std::vector<uint8_t> payload{0x0A, 0x0B};
    SilKit::Services::PubSub::WireDataMessageEvent event{std::chrono::nanoseconds{5}, payload};

    SerializedMessage sending{event, EndpointAddress{1, 2}, EndpointId{3}};
    auto blobBytes = sending.ReleaseStorage();
    const auto* blobStart = blobBytes.data();
    const auto blobEnd = blobStart + blobBytes.size();

    SerializedMessage receiving{std::move(blobBytes)};
    auto received = receiving.Deserialize<SilKit::Services::PubSub::WireDataMessageEvent>();

    const auto* payloadData = received.data.AsSpan().data();

    ASSERT_EQ(received.data.size(), payload.size());
    ASSERT_TRUE(payloadData < blobStart || payloadData >= blobEnd);
    ASSERT_TRUE(SilKit::Util::ItemsAreEqual(received.data.AsSpan(), SilKit::Util::ToSpan(payload)));
}

TEST(Test_SerializedMessage, a_shared_blob_buffer_rejects_writes)
{
    const std::vector<uint8_t> payload{1, 2, 3};
    SilKit::Services::PubSub::WireDataMessageEvent event{std::chrono::nanoseconds{1}, payload};

    SerializedMessage sending{event, EndpointAddress{1, 2}, EndpointId{3}};
    auto blob = std::make_shared<const std::vector<uint8_t>>(sending.ReleaseStorage());

    SerializedMessage receiving{SilKit::Util::MakeSharedSpan(blob, 0, blob->size())};

    ASSERT_THROW(receiving.ReleaseStorage(), SilKit::SilKitError);
}

namespace {

auto MakeTestPayload(size_t size) -> std::vector<uint8_t>
{
    std::vector<uint8_t> payload(size);
    for (size_t i = 0; i < size; ++i)
    {
        payload[i] = static_cast<uint8_t>(i * 31 + 7);
    }
    return payload;
}

template <typename T>
void AppendRaw(std::vector<uint8_t>& bytes, const T& value)
{
    const auto* first = reinterpret_cast<const uint8_t*>(&value);
    bytes.insert(bytes.end(), first, first + sizeof(value));
}

void AppendLengthPrefixed(std::vector<uint8_t>& bytes, const std::vector<uint8_t>& payload)
{
    AppendRaw(bytes, static_cast<uint32_t>(payload.size()));
    bytes.insert(bytes.end(), payload.begin(), payload.end());
}

auto ShareBlob(SerializedMessage sending) -> std::shared_ptr<const std::vector<uint8_t>>
{
    return std::make_shared<const std::vector<uint8_t>>(sending.ReleaseStorage());
}

//! Round-trip msg through a shared blob and check that the payload selected by payloadOf aliases it.
template <typename MsgT, typename PayloadOf>
void ExpectPayloadAliasesTheBlob(const MsgT& msg, PayloadOf payloadOf)
{
    const auto blob = ShareBlob(SerializedMessage{msg, EndpointAddress{1, 2}, EndpointId{3}});

    SerializedMessage receiving{SilKit::Util::MakeSharedSpan(blob, 0, blob->size())};
    const auto received = receiving.Deserialize<MsgT>();
    const auto& payload = payloadOf(received);
    const auto view = payload.AsSpan();

    EXPECT_TRUE(payload.HasOwner());
    EXPECT_GE(view.data(), blob->data());
    EXPECT_LE(view.data() + view.size(), blob->data() + blob->size());
    EXPECT_TRUE(SilKit::Util::ItemsAreEqual(view, payloadOf(msg).AsSpan()));
}

} // namespace

TEST(Test_SerializedMessage, can_frame_payload_aliases_a_shared_blob)
{
    SilKit::Services::Can::WireCanFrameEvent event{};
    event.frame.canId = 0x123;
    event.frame.dataField = MakeTestPayload(64);

    ExpectPayloadAliasesTheBlob(event, [](const auto& msg) -> const auto& { return msg.frame.dataField; });
}

TEST(Test_SerializedMessage, ethernet_frame_payload_aliases_a_shared_blob)
{
    SilKit::Services::Ethernet::WireEthernetFrameEvent event{};
    event.frame.raw = MakeTestPayload(1500);

    ExpectPayloadAliasesTheBlob(event, [](const auto& msg) -> const auto& { return msg.frame.raw; });
}

TEST(Test_SerializedMessage, flexray_frame_payload_aliases_a_shared_blob)
{
    SilKit::Services::Flexray::WireFlexrayFrameEvent event{};
    event.channel = SilKit::Services::Flexray::FlexrayChannel::A;
    event.frame.payload = MakeTestPayload(254);

    ExpectPayloadAliasesTheBlob(event, [](const auto& msg) -> const auto& { return msg.frame.payload; });
}

TEST(Test_SerializedMessage, flexray_tx_buffer_update_payload_aliases_a_shared_blob)
{
    SilKit::Services::Flexray::WireFlexrayTxBufferUpdate update{};
    update.txBufferIndex = 3;
    update.payloadDataValid = true;
    update.payload = MakeTestPayload(254);

    ExpectPayloadAliasesTheBlob(update, [](const auto& msg) -> const auto& { return msg.payload; });
}

TEST(Test_SerializedMessage, rpc_payloads_alias_a_shared_blob)
{
    SilKit::Services::Rpc::FunctionCall call{std::chrono::nanoseconds{1}, SilKit::Util::Uuid{1, 2},
                                             MakeTestPayload(300)};
    ExpectPayloadAliasesTheBlob(call, [](const auto& msg) -> const auto& { return msg.data; });

    SilKit::Services::Rpc::FunctionCallResponse response{
        std::chrono::nanoseconds{2}, SilKit::Util::Uuid{3, 4}, MakeTestPayload(300),
        SilKit::Services::Rpc::FunctionCallResponse::Status::Success};
    ExpectPayloadAliasesTheBlob(response, [](const auto& msg) -> const auto& { return msg.data; });
}

TEST(Test_SerializedMessage, rpc_wire_format_is_unchanged)
{
    // The payload used to be a std::vector<uint8_t>, which was written as a uint32 length prefix
    // followed by the bytes. The SharedSpan payload must produce the very same bytes.
    using SilKit::Services::Rpc::FunctionCall;
    using SilKit::Services::Rpc::FunctionCallResponse;

    const auto payload = MakeTestPayload(17);
    const FunctionCall call{std::chrono::nanoseconds{0x0102030405060708}, SilKit::Util::Uuid{0x1111, 0x2222},
                            payload};
    const FunctionCallResponse response{std::chrono::nanoseconds{-5}, SilKit::Util::Uuid{0x3333, 0x4444}, payload,
                                        FunctionCallResponse::Status::InternalError};

    std::vector<uint8_t> expectedCall;
    AppendRaw(expectedCall, int64_t{0x0102030405060708});
    AppendRaw(expectedCall, uint64_t{0x1111});
    AppendRaw(expectedCall, uint64_t{0x2222});
    AppendLengthPrefixed(expectedCall, payload);

    std::vector<uint8_t> expectedResponse;
    AppendRaw(expectedResponse, int64_t{-5});
    AppendRaw(expectedResponse, uint64_t{0x3333});
    AppendRaw(expectedResponse, uint64_t{0x4444});
    AppendLengthPrefixed(expectedResponse, payload);
    AppendRaw(expectedResponse, static_cast<uint32_t>(FunctionCallResponse::Status::InternalError));

    MessageBuffer callBuffer;
    Serialize(callBuffer, call);
    EXPECT_EQ(callBuffer.ReleaseStorage(), expectedCall);

    MessageBuffer responseBuffer;
    Serialize(responseBuffer, response);
    EXPECT_EQ(responseBuffer.ReleaseStorage(), expectedResponse);

    // and bytes written by an older version are read back
    MessageBuffer callReader{expectedCall};
    FunctionCall readCall{};
    Deserialize(callReader, readCall);
    EXPECT_EQ(readCall, call);
    EXPECT_EQ(readCall.timestamp, call.timestamp);

    MessageBuffer responseReader{expectedResponse};
    FunctionCallResponse readResponse{};
    Deserialize(responseReader, readResponse);
    EXPECT_EQ(readResponse, response);
    EXPECT_EQ(readResponse.timestamp, response.timestamp);
}

TEST(Test_SerializedMessage, empty_payload_from_a_shared_blob)
{
    SilKit::Services::PubSub::WireDataMessageEvent event{std::chrono::nanoseconds{1}, std::vector<uint8_t>{}};
    const auto blob = ShareBlob(SerializedMessage{event, EndpointAddress{1, 2}, EndpointId{3}});

    SerializedMessage receiving{SilKit::Util::MakeSharedSpan(blob, 0, blob->size())};
    SilKit::Services::PubSub::WireDataMessageEvent received{};
    ASSERT_NO_THROW(received = receiving.Deserialize<SilKit::Services::PubSub::WireDataMessageEvent>());

    EXPECT_TRUE(received.data.empty());
    EXPECT_EQ(received.timestamp, event.timestamp);
}

TEST(Test_SerializedMessage, payload_ending_at_the_blob_end)
{
    // The payload is the last field of a FunctionCall, so its view ends exactly where the blob ends.
    const SilKit::Services::Rpc::FunctionCall call{std::chrono::nanoseconds{1}, SilKit::Util::Uuid{1, 2},
                                                   MakeTestPayload(40)};
    const auto blob = ShareBlob(SerializedMessage{call, EndpointAddress{1, 2}, EndpointId{3}});

    SerializedMessage receiving{SilKit::Util::MakeSharedSpan(blob, 0, blob->size())};
    const auto received = receiving.Deserialize<SilKit::Services::Rpc::FunctionCall>();

    EXPECT_EQ(received.data.AsSpan().data() + received.data.size(), blob->data() + blob->size());
    EXPECT_TRUE(SilKit::Util::ItemsAreEqual(received.data.AsSpan(), call.data.AsSpan()));
}

TEST(Test_SerializedMessage, truncated_payload_throws_end_of_buffer)
{
    const SilKit::Services::Rpc::FunctionCall call{std::chrono::nanoseconds{1}, SilKit::Util::Uuid{1, 2},
                                                   MakeTestPayload(40)};
    const auto blob = ShareBlob(SerializedMessage{call, EndpointAddress{1, 2}, EndpointId{3}});

    SerializedMessage receiving{SilKit::Util::MakeSharedSpan(blob, 0, blob->size() - 1)};

    EXPECT_THROW(receiving.Deserialize<SilKit::Services::Rpc::FunctionCall>(), end_of_buffer);
}
