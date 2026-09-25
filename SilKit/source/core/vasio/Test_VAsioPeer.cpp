// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "core/vasio/VAsioPeer.hpp"

#include <algorithm>
#include <cstring>
#include <deque>
#include <functional>
#include <memory>
#include <vector>

#include "core/internal/OrchestrationDatatypes.hpp"
#include "core/vasio/PeerMetrics.hpp"
#include "core/vasio/ReceiveBlobPool.hpp"
#include "core/vasio/SharedSerializedMessage.hpp"
#include "core/vasio/io/mock/MockIoContext.hpp"
#include "core/vasio/io/mock/MockRawByteStream.hpp"
#include "core/vasio/io/mock/MockTimer.hpp"
#include "services/logging/MockLogger.hpp"
#include "wire/pubsub/WireDataMessages.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace {

using namespace SilKit::Core;
using SilKit::Services::PubSub::WireDataMessageEvent;
using SilKit::Util::ToStdVector;
using testing::_;
using testing::NiceMock;

const EndpointAddress From{7, 9};

//! Non-uniform, so that shifted, duplicated or skipped bytes are detected.
auto MakePayload(size_t size, uint8_t seed) -> std::vector<uint8_t>
{
    std::vector<uint8_t> payload(size);
    for (size_t i = 0; i < size; ++i)
    {
        payload[i] = static_cast<uint8_t>(i * 31 + seed);
    }
    return payload;
}

auto MakeEvent(size_t payloadSize, uint8_t seed) -> WireDataMessageEvent
{
    return WireDataMessageEvent{std::chrono::nanoseconds{seed}, MakePayload(payloadSize, seed)};
}

template <typename MsgT>
auto Reference(const MsgT& msg, EndpointId remoteIdx) -> std::vector<uint8_t>
{
    return SerializedMessage{msg, From, remoteIdx}.ReleaseStorage();
}

auto Concat(std::initializer_list<std::vector<uint8_t>> parts) -> std::vector<uint8_t>
{
    std::vector<uint8_t> result;
    for (const auto& part : parts)
    {
        result.insert(result.end(), part.begin(), part.end());
    }
    return result;
}

struct TestPeerListener : IVAsioPeerListener
{
    std::function<void(SerializedMessage&)> onData;

    void OnSocketData(IVAsioPeer*, SerializedMessage&& message) override
    {
        if (onData)
        {
            onData(message);
        }
    }

    void OnPeerShutdown(IVAsioPeer*) override {}
};

//! A VAsioPeer on top of a mocked stream. Writes and reads complete only when the test says so.
class PeerUnderTest
{
public:
    explicit PeerUnderTest(bool useReceiveBlobPool = true)
        : _pool{useReceiveBlobPool}
    {
        ON_CALL(_ioContext, Dispatch(_)).WillByDefault([](std::function<void()> function) { function(); });
        ON_CALL(_ioContext, Post(_)).WillByDefault([](std::function<void()> function) { function(); });
        ON_CALL(_ioContext, MakeTimer()).WillByDefault([]() -> std::unique_ptr<VSilKit::ITimer> {
            return std::make_unique<NiceMock<VSilKit::MockTimer>>();
        });

        auto stream = std::make_unique<NiceMock<VSilKit::MockRawByteStream>>();
        _stream = stream.get();
        ON_CALL(*_stream, SetListener(_)).WillByDefault([this](VSilKit::IRawByteStreamListener& listener) {
            _streamListener = &listener;
        });
        ON_CALL(*_stream, AsyncWriteSome(_)).WillByDefault([this](VSilKit::ConstBufferSequence sequence) {
            _writes.emplace_back(sequence.begin(), sequence.end());
        });
        ON_CALL(*_stream, AsyncReadSome(_)).WillByDefault([this](VSilKit::MutableBufferSequence sequence) {
            _reads.emplace_back(sequence.begin(), sequence.end());
        });

        _peer = std::make_unique<VAsioPeer>(&_listener, &_ioContext, std::move(stream), &_logger,
                                            std::make_unique<VSilKit::NoMetrics>(), &_pool);
    }

    auto Peer() -> VAsioPeer&
    {
        return *_peer;
    }

    auto Listener() -> TestPeerListener&
    {
        return _listener;
    }

    auto WriteCount() const -> size_t
    {
        return _writes.size();
    }

    auto LastWrite() const -> const std::vector<VSilKit::ConstBuffer>&
    {
        return _writes.back();
    }

    //! Complete every pending write, the first ones with the given byte counts and the rest in full.
    //! Returns the bytes as they appeared on the wire.
    auto DrainWrites(std::deque<size_t> transferred = {}) -> std::vector<uint8_t>
    {
        std::vector<uint8_t> wire;
        while (_completedWrites < _writes.size())
        {
            const auto buffers = _writes[_completedWrites++];

            size_t total{0};
            for (const auto& buffer : buffers)
            {
                total += buffer.GetSize();
            }

            size_t count = total;
            if (!transferred.empty())
            {
                count = (std::min)(transferred.front(), total);
                transferred.pop_front();
            }

            auto remaining = count;
            for (const auto& buffer : buffers)
            {
                const auto n = (std::min)(remaining, buffer.GetSize());
                const auto* data = static_cast<const uint8_t*>(buffer.GetData());
                wire.insert(wire.end(), data, data + n);
                remaining -= n;
            }

            _streamListener->OnAsyncWriteSomeDone(*_stream, count);
        }
        return wire;
    }

    //! Deliver bytes to the peer, the first reads with the given byte counts and the rest as large
    //! as the offered read buffers allow.
    void Feed(const std::vector<uint8_t>& bytes, std::deque<size_t> chunks = {})
    {
        size_t offset{0};
        while (offset < bytes.size())
        {
            ASSERT_FALSE(_reads.empty()) << "the peer did not request a read";
            const auto buffers = _reads.back();
            _reads.clear();

            size_t capacity{0};
            for (const auto& buffer : buffers)
            {
                capacity += buffer.GetSize();
            }

            auto count = (std::min)(capacity, bytes.size() - offset);
            if (!chunks.empty())
            {
                count = (std::min)(count, chunks.front());
                chunks.pop_front();
            }

            auto remaining = count;
            for (const auto& buffer : buffers)
            {
                const auto n = (std::min)(remaining, buffer.GetSize());
                std::memcpy(buffer.GetData(), bytes.data() + offset + (count - remaining), n);
                remaining -= n;
            }

            offset += count;
            _streamListener->OnAsyncReadSomeDone(*_stream, count);
        }
    }

private:
    ReceiveBlobPool _pool;
    NiceMock<VSilKit::MockIoContext> _ioContext;
    NiceMock<SilKit::Services::Logging::MockLogger> _logger;
    TestPeerListener _listener;

    NiceMock<VSilKit::MockRawByteStream>* _stream{nullptr};
    VSilKit::IRawByteStreamListener* _streamListener{nullptr};
    std::vector<std::vector<VSilKit::ConstBuffer>> _writes;
    size_t _completedWrites{0};
    std::vector<std::vector<VSilKit::MutableBuffer>> _reads;

    std::unique_ptr<VAsioPeer> _peer;
};

// ================================================================================
//  Sending
// ================================================================================

TEST(Test_VAsioPeer, small_message_is_written_as_one_inline_buffer)
{
    PeerUnderTest p;
    const auto event = MakeEvent(3, 1);

    p.Peer().SendSilKitMsg(SerializedMessage{event, From, EndpointId{42}});

    ASSERT_EQ(p.WriteCount(), 1u);
    EXPECT_EQ(p.LastWrite().size(), 1u);
    EXPECT_EQ(p.DrainWrites(), Reference(event, 42));
}

TEST(Test_VAsioPeer, large_unshared_message_is_written_as_one_owned_buffer)
{
    PeerUnderTest p;
    const auto event = MakeEvent(1024, 2);

    p.Peer().SendSilKitMsg(SerializedMessage{event, From, EndpointId{42}});

    ASSERT_EQ(p.WriteCount(), 1u);
    EXPECT_EQ(p.LastWrite().size(), 1u);
    EXPECT_EQ(p.DrainWrites(), Reference(event, 42));
}

TEST(Test_VAsioPeer, small_shared_message_is_inlined_with_the_peers_remote_index)
{
    PeerUnderTest p;
    const auto event = MakeEvent(3, 3);
    const SharedSerializedMessage shared{event, From};

    p.Peer().SendSilKitMsg(shared, EndpointId{77});

    ASSERT_EQ(p.WriteCount(), 1u);
    EXPECT_EQ(p.LastWrite().size(), 1u);
    EXPECT_EQ(p.DrainWrites(), Reference(event, 77));
}

TEST(Test_VAsioPeer, large_shared_message_is_written_as_header_and_shared_body)
{
    PeerUnderTest p;
    const auto event = MakeEvent(1024, 4);
    const SharedSerializedMessage shared{event, From};

    p.Peer().SendSilKitMsg(shared, EndpointId{77});

    ASSERT_EQ(p.WriteCount(), 1u);
    ASSERT_EQ(p.LastWrite().size(), 2u);
    EXPECT_EQ(p.LastWrite()[0].GetSize(), shared.HeaderSize());
    // the body is written from the shared bytes, not from a copy
    EXPECT_EQ(p.LastWrite()[1].GetData(), shared.Body().AsSpan().data());
    EXPECT_EQ(p.DrainWrites(), Reference(event, 77));
}

TEST(Test_VAsioPeer, peers_share_the_body_but_write_their_own_remote_index)
{
    PeerUnderTest a;
    PeerUnderTest b;
    const auto event = MakeEvent(1024, 5);
    const SharedSerializedMessage shared{event, From};

    a.Peer().SendSilKitMsg(shared, EndpointId{5});
    b.Peer().SendSilKitMsg(shared, EndpointId{9});

    ASSERT_EQ(a.LastWrite().size(), 2u);
    ASSERT_EQ(b.LastWrite().size(), 2u);
    EXPECT_EQ(a.LastWrite()[1].GetData(), b.LastWrite()[1].GetData());
    EXPECT_EQ(a.DrainWrites(), Reference(event, 5));
    EXPECT_EQ(b.DrainWrites(), Reference(event, 9));
}

TEST(Test_VAsioPeer, partial_writes_resume_where_they_stopped)
{
    const auto event = MakeEvent(1024, 6);
    const SharedSerializedMessage shared{event, From};
    const auto expected = Reference(event, 77);
    const auto header = shared.HeaderSize();
    const auto total = shared.TotalSize();

    const std::vector<std::deque<size_t>> splits{
        {0},
        {1},
        {header - 1},
        {header},
        {header + 1},
        {total - 1},
        {1, 1, 1, header, 0, 7, 100},
    };

    for (const auto& split : splits)
    {
        PeerUnderTest p;
        p.Peer().SendSilKitMsg(shared, EndpointId{77});
        EXPECT_EQ(p.DrainWrites(split), expected) << "first transfer: " << split.front();
    }
}

TEST(Test_VAsioPeer, partial_writes_of_an_owned_buffer_resume_where_they_stopped)
{
    PeerUnderTest p;
    const auto event = MakeEvent(1024, 7);

    p.Peer().SendSilKitMsg(SerializedMessage{event, From, EndpointId{42}});

    EXPECT_EQ(p.DrainWrites({1, 500, 0, 3}), Reference(event, 42));
}

TEST(Test_VAsioPeer, queued_items_keep_valid_headers)
{
    PeerUnderTest p;
    const auto first = MakeEvent(1024, 8);
    const auto second = MakeEvent(3, 9);
    const auto third = MakeEvent(2048, 10);
    const SharedSerializedMessage sharedFirst{first, From};
    const SharedSerializedMessage sharedSecond{second, From};
    const SharedSerializedMessage sharedThird{third, From};

    p.Peer().SendSilKitMsg(sharedFirst, EndpointId{1});
    p.Peer().SendSilKitMsg(sharedSecond, EndpointId{2});
    p.Peer().SendSilKitMsg(sharedThird, EndpointId{3});

    // only the first item is in flight, the others wait in the queue
    ASSERT_EQ(p.WriteCount(), 1u);
    EXPECT_EQ(p.DrainWrites({10}), Concat({Reference(first, 1), Reference(second, 2), Reference(third, 3)}));
}

TEST(Test_VAsioPeer, body_outlives_the_shared_message)
{
    PeerUnderTest p;
    const auto event = MakeEvent(1024, 11);

    {
        const SharedSerializedMessage shared{event, From};
        p.Peer().SendSilKitMsg(shared, EndpointId{5});
    }

    EXPECT_EQ(p.DrainWrites(), Reference(event, 5));
}

TEST(Test_VAsioPeer, aggregation_concatenates_inline_owned_and_shared_items)
{
    PeerUnderTest p;
    p.Peer().EnableAggregation();

    const auto small = MakeEvent(3, 12);
    const auto owned = MakeEvent(1024, 13);
    const auto sharedEvent = MakeEvent(2048, 14);
    const SharedSerializedMessage shared{sharedEvent, From};
    const SilKit::Services::Orchestration::NextSimTask nextSimTask{std::chrono::nanoseconds{1},
                                                                   std::chrono::nanoseconds{2}};

    p.Peer().SendSilKitMsg(SerializedMessage{small, From, EndpointId{1}});
    p.Peer().SendSilKitMsg(SerializedMessage{owned, From, EndpointId{2}});
    p.Peer().SendSilKitMsg(shared, EndpointId{3});
    EXPECT_EQ(p.WriteCount(), 0u);

    p.Peer().SendSilKitMsg(SerializedMessage{nextSimTask, From, EndpointId{4}});

    ASSERT_EQ(p.WriteCount(), 1u);
    EXPECT_EQ(p.DrainWrites(), Concat({Reference(small, 1), Reference(owned, 2), Reference(sharedEvent, 3),
                                       Reference(nextSimTask, 4)}));
}

TEST(Test_VAsioPeer, shutdown_with_a_write_in_flight)
{
    PeerUnderTest p;
    const auto event = MakeEvent(1024, 15);
    const SharedSerializedMessage shared{event, From};

    p.Peer().SendSilKitMsg(shared, EndpointId{1});
    p.Peer().SendSilKitMsg(shared, EndpointId{2});
    p.Peer().Shutdown();

    // the in-flight write completes, the queued one was dropped
    EXPECT_EQ(p.DrainWrites(), Reference(event, 1));

    p.Peer().SendSilKitMsg(shared, EndpointId{3});
    EXPECT_EQ(p.WriteCount(), 1u);
}

// ================================================================================
//  Receiving, with and without the receive blob pool
// ================================================================================

class Test_VAsioPeerReceive : public testing::TestWithParam<bool>
{
protected:
    Test_VAsioPeerReceive()
        : p{GetParam()}
    {
        p.Listener().onData = [this](SerializedMessage& message) {
            storageSizes.push_back(message.GetStorageSize());
            auto event = message.Deserialize<WireDataMessageEvent>();
            payloads.push_back(ToStdVector(event.data.AsSpan()));
            if (retain)
            {
                retained.push_back(std::move(event));
            }
        };
        p.Peer().StartAsyncRead();
    }

    PeerUnderTest p;
    bool retain{false};
    std::vector<size_t> storageSizes;
    std::vector<std::vector<uint8_t>> payloads;
    std::vector<WireDataMessageEvent> retained;
};

TEST_P(Test_VAsioPeerReceive, payload_is_delivered_intact)
{
    const auto event = MakeEvent(1024, 1);

    p.Feed(Reference(event, 3));

    ASSERT_EQ(payloads.size(), 1u);
    EXPECT_EQ(payloads[0], ToStdVector(event.data.AsSpan()));
}

TEST_P(Test_VAsioPeerReceive, message_wrapping_the_ring_buffer_is_reassembled)
{
    // 41 bytes of headers, length prefix and timestamp around the payload
    const auto first = MakeEvent(4000 - 41, 2);
    const auto second = MakeEvent(500 - 41, 3);
    const auto firstBytes = Reference(first, 3);
    ASSERT_EQ(firstBytes.size(), 4000u);

    // NB: the first read leaves 10 bytes of the second message in the 4096 byte ring, so the
    //     rest of it is read into the wrapped free region
    p.Feed(Concat({firstBytes, Reference(second, 3)}), {4010});

    ASSERT_EQ(payloads.size(), 2u);
    EXPECT_EQ(payloads[0], ToStdVector(first.data.AsSpan()));
    EXPECT_EQ(payloads[1], ToStdVector(second.data.AsSpan()));
}

TEST_P(Test_VAsioPeerReceive, retained_payload_is_not_overwritten)
{
    const auto kept = MakeEvent(200, 4);

    retain = true;
    p.Feed(Reference(kept, 3));
    retain = false;

    // more messages than the pool holds, so every blob is up for reuse
    for (uint8_t seed = 5; seed < 5 + 2 * ReceiveBlobPool::MaxEntries; ++seed)
    {
        p.Feed(Reference(MakeEvent(200, seed), 3));
    }

    ASSERT_EQ(retained.size(), 1u);
    EXPECT_EQ(ToStdVector(retained[0].data.AsSpan()), ToStdVector(kept.data.AsSpan()));
}

TEST_P(Test_VAsioPeerReceive, message_after_a_larger_one_is_viewed_exactly)
{
    // NB: small enough that the pooled blob of the first message is reused for the second
    const auto large = MakeEvent(2 * 1024, 5);
    const auto small = MakeEvent(100, 6);
    const auto smallBytes = Reference(small, 3);

    p.Feed(Reference(large, 3));
    p.Feed(smallBytes);

    ASSERT_EQ(payloads.size(), 2u);
    EXPECT_EQ(storageSizes[1], smallBytes.size());
    EXPECT_EQ(payloads[1], ToStdVector(small.data.AsSpan()));
}

TEST_P(Test_VAsioPeerReceive, message_above_the_pool_limit)
{
    const auto large = MakeEvent(ReceiveBlobPool::MaxBlobSize + 6 * 1024, 7);
    const auto small = MakeEvent(100, 8);

    p.Feed(Reference(large, 3));
    p.Feed(Reference(small, 3));

    ASSERT_EQ(payloads.size(), 2u);
    EXPECT_EQ(payloads[0], ToStdVector(large.data.AsSpan()));
    EXPECT_EQ(payloads[1], ToStdVector(small.data.AsSpan()));
}

TEST_P(Test_VAsioPeerReceive, byte_by_byte_delivery)
{
    const auto event = MakeEvent(300, 9);
    const auto bytes = Reference(event, 3);

    p.Feed(bytes, std::deque<size_t>(bytes.size(), 1));

    ASSERT_EQ(payloads.size(), 1u);
    EXPECT_EQ(payloads[0], ToStdVector(event.data.AsSpan()));
}

INSTANTIATE_TEST_SUITE_P(ReceiveBlobPool, Test_VAsioPeerReceive, testing::Values(true, false),
                         [](const testing::TestParamInfo<bool>& info) {
    return std::string{info.param ? "Enabled" : "Disabled"};
});

} // namespace
