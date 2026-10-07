// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <chrono>
#include <future>
#include <map>
#include <thread>
#include <vector>

#include "silkit/SilKit.hpp"
#include "silkit/services/ethernet/all.hpp"
#include "silkit/vendor/CreateSilKitRegistry.hpp"

#include "gtest/gtest.h"

namespace {

using namespace std::chrono_literals;
using namespace SilKit::Services::Ethernet;

TEST(ITest_EthernetTransmitQueue, frames_are_dropped_while_the_transmit_queue_is_full)
{
    auto registry =
        SilKit::Vendor::Vector::CreateSilKitRegistry(SilKit::Config::ParticipantConfigurationFromString(""));
    const auto registryUri = registry->StartListening("silkit://localhost:0");

    std::promise<void> receiverBlocked;
    std::promise<void> unblockReceiver;
    auto receiverUnblocked = unblockReceiver.get_future().share();
    bool isFirstFrame{true};

    auto receiver =
        SilKit::CreateParticipant(SilKit::Config::ParticipantConfigurationFromString(""), "Receiver", registryUri);
    auto* receiverController = receiver->CreateEthernetController("ETH1", "ETH1");
    // NB: blocking the IO thread of the receiver stalls the connection, so the sender's queue fills up
    receiverController->AddFrameHandler([&](IEthernetController*, const EthernetFrameEvent&) {
        if (isFirstFrame)
        {
            isFirstFrame = false;
            receiverBlocked.set_value();
            receiverUnblocked.wait();
        }
    });
    receiverController->Activate();

    auto sender = SilKit::CreateParticipant(SilKit::Config::ParticipantConfigurationFromString(R"(
Middleware:
  EnableDomainSockets: false
  TcpNotSentLowWatermark: 16384
Experimental:
  TransmitQueueSize: 16384
)"),
                                            "Sender", registryUri);
    auto* senderController = sender->CreateEthernetController("ETH1", "ETH1");
    std::map<EthernetTransmitStatus, size_t> numAcks;
    EthernetTransmitStatus lastStatus{};
    senderController->AddFrameTransmitHandler([&](IEthernetController*, const EthernetFrameTransmitEvent& ack) {
        ++numAcks[ack.status];
        lastStatus = ack.status;
    });
    senderController->Activate();

    const std::vector<uint8_t> frame(1500);
    size_t numSent{0};
    const auto sendFrame = [&] {
        senderController->SendFrame(EthernetFrame{frame});
        ++numSent;
    };

    auto blocked = receiverBlocked.get_future();
    auto deadline = std::chrono::steady_clock::now() + 10s;
    while (blocked.wait_for(10ms) != std::future_status::ready && std::chrono::steady_clock::now() < deadline)
    {
        sendFrame();
    }
    if (blocked.wait_for(0s) != std::future_status::ready)
    {
        unblockReceiver.set_value();
        FAIL() << "receiver did not receive a frame";
    }

    for (size_t i = 0; i < 200'000 && lastStatus != EthernetTransmitStatus::Dropped; ++i)
    {
        sendFrame();
    }
    const auto numDroppedWhileBlocked = numAcks[EthernetTransmitStatus::Dropped];

    unblockReceiver.set_value();

    deadline = std::chrono::steady_clock::now() + 10s;
    do
    {
        std::this_thread::sleep_for(10ms);
        sendFrame();
    } while (lastStatus != EthernetTransmitStatus::Transmitted && std::chrono::steady_clock::now() < deadline);

    EXPECT_GT(numDroppedWhileBlocked, 0u);
    EXPECT_EQ(lastStatus, EthernetTransmitStatus::Transmitted);
    EXPECT_EQ(numAcks[EthernetTransmitStatus::Transmitted] + numAcks[EthernetTransmitStatus::Dropped], numSent);
}

} // anonymous namespace
