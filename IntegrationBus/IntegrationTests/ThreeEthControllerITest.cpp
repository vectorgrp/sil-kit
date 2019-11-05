// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <cstdlib>
#include <thread>
#include <future>

#include <iostream>

#include "CreateComAdapter.hpp"
#include "VAsioRegistry.hpp"

#include "ib/cfg/ConfigBuilder.hpp"
#include "ib/sim/all.hpp"
#include "ib/util/functional.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "GetTestPid.hpp"

namespace {

using namespace std::chrono_literals;
using namespace ib::mw;
using namespace ib::cfg;
using namespace ib::sim::eth;

using testing::_;
using testing::A;
using testing::An;
using testing::AnyNumber;
using testing::AtLeast;
using testing::InSequence;
using testing::NiceMock;
using testing::Return;

auto MatchTxId(EthTxId transmitId) -> testing::Matcher<const EthTransmitAcknowledge&>
{
    using namespace testing;
    return
        Field(&EthTransmitAcknowledge::transmitId, transmitId)
        ;
}

class ThreeEthControllerITest : public testing::Test
{
protected:
    struct Callbacks
    {
        MOCK_METHOD1(AckHandler, void(const EthTransmitAcknowledge&));
    };

protected:
    ThreeEthControllerITest()
    {
        domainId = static_cast<uint32_t>(GetTestPid());

        testMessages.resize(5);
        for (auto index = 0u; index < testMessages.size(); index++)
        {
            std::stringstream messageBuilder;
            messageBuilder << "Test Message " << index;
            testMessages[index].expectedData = messageBuilder.str();
        }

        ib::cfg::ConfigBuilder builder{"TestConfig"};
        auto &&setup = builder.SimulationSetup();
        setup.AddParticipant("EthWriter")
            ->AddEthernet("ETH1").WithLink("LINK1");
        setup.AddParticipant("EthReader1")
            ->AddEthernet("ETH1").WithLink("LINK1");
        setup.AddParticipant("EthReader2")
            ->AddEthernet("ETH1").WithLink("LINK1");

        ibConfig = builder.Build();
    }

    void Sender()
    {
        std::cout << " Sender init" << std::endl;
        auto comAdapter = CreateComAdapterImpl(ibConfig, "EthWriter");
        comAdapter->joinIbDomain(domainId);

        std::promise<void> threadFinishedPromise{};
        unsigned int receiveCount{ 0 };

        auto* controller = comAdapter->CreateEthController("ETH1");
        controller->RegisterMessageAckHandler(
            [this, &threadFinishedPromise, &receiveCount](IEthController*, const EthTransmitAcknowledge& ack) {
            std::cout << "<- EthTransmitAck" << std::endl;
            callbacks.AckHandler(ack);
            ++receiveCount;
            if (testMessages.size() == receiveCount)
            {
                allReceivedPromise.set_value();
                threadFinishedPromise.set_value();
            }
        });

        for (auto&& message : testMessages)
        {
            EthMessage msg;
            msg.ethFrame.SetDestinationMac(EthMac{ 0x12,0x23,0x45,0x67,0x89,0x9a });
            msg.ethFrame.SetSourceMac(EthMac{ 0x9a, 0x89, 0x67, 0x45, 0x23, 0x12});
            msg.ethFrame.SetEtherType(0x8100);
            msg.ethFrame.SetPayload(reinterpret_cast<const uint8_t*>(message.expectedData.c_str()), message.expectedData.size());

            std::cout << "<- SendMesg" << std::endl;

            std::this_thread::sleep_for(100ms);
            controller->SendMessage(std::move(msg));
        }

        auto threadFinished = threadFinishedPromise.get_future();
        ASSERT_EQ(threadFinished.wait_for(15s), std::future_status::ready);
    }

    void Receiver(const std::string& participantName, const std::shared_future<void>& finishedFuture)
    {
        std::cout << " Receiver init " << participantName << std::endl;
        auto comAdapter = CreateComAdapterImpl(ibConfig, participantName);
        comAdapter->joinIbDomain(domainId);

        unsigned int receiveCount{0};

        auto* controller = comAdapter->CreateEthController("ETH1");
        controller->RegisterMessageAckHandler(
            [this](IEthController* , const EthTransmitAcknowledge& ack) {
            callbacks.AckHandler(ack);
        });
        controller->RegisterReceiveMessageHandler(
            [this, &receiveCount, &participantName](IEthController* , const EthMessage& msg) {
                if (participantName == "EthReader1")
                {
                    const auto &pl = msg.ethFrame.GetPayload();
                    std::string message(pl.begin(), pl.end());
                    std::cout << " <- received message on " << participantName
                        << " ethframe size=" << pl.size()
                        << ": testMessages size=" << testMessages.size()
                        << ": receiveCount=" << receiveCount
                        << ": " << message
                        << std::endl;
                    testMessages[receiveCount].receivedData = message;
                    receiveCount++;
                }
        });
        //blocking
        ASSERT_EQ(finishedFuture.wait_for(15s), std::future_status::ready);
        if (participantName == "EthReader1")
        {
            ASSERT_EQ(receiveCount, testMessages.size());
        }
    }

    void ExecuteTest()
    {
        std::shared_future<void> finishedFuture(allReceivedPromise.get_future());
        std::thread receiver1Thread{[this, &finishedFuture] { Receiver("EthReader1", finishedFuture); }};
        std::thread receiver2Thread{[this, &finishedFuture] { Receiver("EthReader2", finishedFuture); }};
        for (auto index = 1u; index <= testMessages.size(); index++)
        {
            EXPECT_CALL(callbacks, AckHandler(MatchTxId(index))).Times(1);
        }
        EXPECT_CALL(callbacks, AckHandler(MatchTxId(0))).Times(0);

        std::thread senderThread{[this] { Sender(); }};

        senderThread.join();
        receiver1Thread.join();
        receiver2Thread.join();

        for (auto&& message : testMessages)
        {
            EXPECT_EQ(message.expectedData, message.receivedData);
        }
    }

    struct TestMessage
    {
        std::string expectedData;
        std::string receivedData;
    };

protected:
    uint32_t domainId;
    ib::cfg::Config ibConfig;

    std::vector<TestMessage> testMessages;

    std::promise<void> allReceivedPromise;
    Callbacks callbacks;
};

TEST_F(ThreeEthControllerITest, test_eth_ack_callbacks_fastrtps)
{
    ibConfig.middlewareConfig.activeMiddleware = ib::cfg::Middleware::FastRTPS;

    ExecuteTest();
}

TEST_F(ThreeEthControllerITest, test_eth_ack_callbacks_vasio)
{
    ibConfig.middlewareConfig.activeMiddleware = ib::cfg::Middleware::VAsio;

    auto registry = std::make_unique<VAsioRegistry>(ibConfig);
    registry->ProvideDomain(domainId);

    ExecuteTest();
}

} // anonymous namespace
