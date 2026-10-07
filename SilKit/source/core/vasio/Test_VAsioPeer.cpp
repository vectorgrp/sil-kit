// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "core/vasio/VAsioPeer.hpp"
#include "core/vasio/VAsioProxyPeer.hpp"

#include "services/logging/MockLogger.hpp"

#include "core/vasio/io/mock/MockIoContext.hpp"
#include "core/vasio/io/mock/MockRawByteStream.hpp"
#include "core/vasio/io/mock/MockTimer.hpp"
#include "core/vasio/mock/MockVAsioPeer.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace {

using namespace SilKit::Core;

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

using SilKit::Services::Ethernet::WireEthernetFrameEvent;
using SilKit::Services::Logging::MockLogger;
using VSilKit::MockIoContextWithExecutionQueue;
using VSilKit::MockRawByteStream;
using VSilKit::MockTimer;

auto MakeEthernetFrameMessage(std::shared_ptr<const void> reservation) -> SerializedMessage
{
    WireEthernetFrameEvent msg{};
    msg.frame.raw = std::vector<uint8_t>(60);
    msg.transmitReservation = std::move(reservation);
    return SerializedMessage{msg, EndpointAddress{}, EndpointId{1}};
}

struct Test_VAsioPeer : ::testing::Test
{
    MockIoContextWithExecutionQueue ioContext;
    NiceMock<MockLogger> logger;
    MockRawByteStream* stream{nullptr};
    VSilKit::IRawByteStreamListener* streamListener{nullptr};
    size_t pendingWriteSize{0};
    std::unique_ptr<VAsioPeer> peer;

    Test_VAsioPeer()
    {
        EXPECT_CALL(ioContext, MakeTimer).WillOnce([] { return std::make_unique<NiceMock<MockTimer>>(); });

        auto rawByteStream = std::make_unique<NiceMock<MockRawByteStream>>();
        stream = rawByteStream.get();
        ON_CALL(*stream, SetListener).WillByDefault([this](auto& listener) { streamListener = &listener; });
        ON_CALL(*stream, AsyncWriteSome).WillByDefault([this](VSilKit::ConstBufferSequence buffers) {
            pendingWriteSize = buffers[0].GetSize();
        });

        peer = std::make_unique<VAsioPeer>(nullptr, &ioContext, std::move(rawByteStream), &logger,
                                           std::make_unique<VSilKit::NoMetrics>());
    }

    void CompleteWrite(size_t bytes)
    {
        streamListener->OnAsyncWriteSomeDone(*stream, bytes);
        ioContext.Run();
    }
};

TEST_F(Test_VAsioPeer, transmit_reservation_is_released_after_write)
{
    auto reservation = std::make_shared<int>();
    std::weak_ptr<int> weak = reservation;

    peer->SendSilKitMsg(MakeEthernetFrameMessage(std::move(reservation)));
    ioContext.Run();
    ASSERT_GT(pendingWriteSize, 1u);
    EXPECT_FALSE(weak.expired());

    CompleteWrite(pendingWriteSize - 1);
    EXPECT_FALSE(weak.expired());

    CompleteWrite(1);
    EXPECT_TRUE(weak.expired());
}

TEST_F(Test_VAsioPeer, transmit_reservation_is_released_on_shutdown)
{
    auto reservation = std::make_shared<int>();
    std::weak_ptr<int> weak = reservation;

    peer->SendSilKitMsg(MakeEthernetFrameMessage(nullptr));
    ioContext.Run();
    peer->SendSilKitMsg(MakeEthernetFrameMessage(std::move(reservation)));
    EXPECT_FALSE(weak.expired());

    peer->Shutdown();
    EXPECT_TRUE(weak.expired());
}

TEST_F(Test_VAsioPeer, aggregated_transmit_reservation_is_released_after_write)
{
    auto reservation = std::make_shared<int>();
    std::weak_ptr<int> weak = reservation;

    peer->EnableAggregation();
    peer->SendSilKitMsg(MakeEthernetFrameMessage(std::move(reservation)));
    ioContext.Run();
    EXPECT_EQ(pendingWriteSize, 0u);
    EXPECT_FALSE(weak.expired());

    peer->SendSilKitMsg(
        SerializedMessage{SilKit::Services::Orchestration::NextSimTask{}, EndpointAddress{}, EndpointId{1}});
    ioContext.Run();
    ASSERT_GT(pendingWriteSize, 0u);
    EXPECT_FALSE(weak.expired());

    CompleteWrite(pendingWriteSize);
    EXPECT_TRUE(weak.expired());
}

TEST(Test_VAsioProxyPeer, forwards_transmit_reservation)
{
    NiceMock<MockLogger> logger;
    NiceMock<MockVAsioPeer> registryPeer;
    VAsioPeerInfo registryInfo{};
    ON_CALL(registryPeer, GetInfo()).WillByDefault(ReturnRef(registryInfo));

    VAsioProxyPeer proxyPeer{nullptr, "Participant", VAsioPeerInfo{}, &registryPeer, &logger};

    auto reservation = std::make_shared<int>();
    EXPECT_CALL(registryPeer, SendSilKitMsg(_)).WillOnce([&reservation](SerializedMessage buffer) {
        EXPECT_EQ(buffer.ReleaseTransmitReservation(), reservation);
    });

    proxyPeer.SendSilKitMsg(MakeEthernetFrameMessage(reservation));
}

} // namespace
