// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <iostream>

#include "ib/sim/all.hpp"
#include "ib/util/functional.hpp"

#include "SimTestHarness.hpp"
#include "GetTestPid.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

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
    return Field(&EthTransmitAcknowledge::transmitId, transmitId);
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

        syncParticipantNames = { "EthWriter" ,"EthReader1" ,"EthReader2" };
    }

    void SetupSender(ib::test::SimParticipant* participant)
    {
        std::cout << " Sender init " << participant->Name() << std::endl;

        auto* controller = participant->ComAdapter()->CreateEthController("ETH1", "LINK1");
        controller->RegisterMessageAckHandler(
            [this, participant](IEthController*, const EthTransmitAcknowledge& ack) {
            std::cout << participant->Name() << " <- EthTransmitAck" << std::endl;
            callbacks.AckHandler(ack);
            numAcked++;
        });
        auto* participantController = 
            participant->ComAdapter()->GetParticipantController();
        participantController->SetSimulationTask(
            [this, participant, controller](std::chrono::nanoseconds now, std::chrono::nanoseconds) {
                if (numSent < testMessages.size())
                {
                    const auto& message = testMessages.at(numSent);
                    EthFrame frame;
                    frame.SetDestinationMac(EthMac{ 0x12,0x23,0x45,0x67,0x89,0x9a });
                    frame.SetSourceMac(EthMac{ 0x9a, 0x89, 0x67, 0x45, 0x23, 0x12});
                    frame.SetEtherType(0x8100);
                    frame.SetPayload(reinterpret_cast<const uint8_t*>(message.expectedData.c_str()), message.expectedData.size());

                    std::cout << participant->Name() << " -> SendMesg @" << now.count() << "ns" << std::endl;

                    controller->SendFrame(std::move(frame));
                    numSent++;
                    std::this_thread::sleep_for(100ms);
                }
        });
    }

    void SetupReceiver(ib::test::SimParticipant* participant)
    {
        std::cout << " Receiver init " << participant->Name() << std::endl;
        auto* controller = participant->ComAdapter()->CreateEthController("ETH1", "LINK1");
        controller->RegisterMessageAckHandler(
            [this](IEthController* , const EthTransmitAcknowledge& ack) {
            callbacks.AckHandler(ack);
        });

        controller->RegisterReceiveMessageHandler(
            [this, participant](IEthController*, const EthMessage& msg) {
                const auto& pl = msg.ethFrame.GetPayload();
                std::string message(pl.begin(), pl.end());
                std::cout << participant->Name() 
                    <<" <- received message"
                    << ": ethframe size=" << pl.size()
                    << ": testMessages size=" << testMessages.size()
                    << ": receiveCount=" << numReceived
                    << ": " << message
                    << std::endl;
                testMessages[numReceived].receivedData = message;
                numReceived++;
                if (numReceived >= testMessages.size())
                {
                    participant->Stop();
                }
            });
    }

    void ExecuteTest()
    {
        ib::test::SimTestHarness testHarness(syncParticipantNames, domainId);

        //participant setup
        auto* ethWriter  = testHarness.GetParticipant("EthWriter");
        auto* ethReader1 = testHarness.GetParticipant("EthReader1");
        auto* ethReader2 = testHarness.GetParticipant("EthReader2");

        SetupSender(ethWriter);
        SetupReceiver(ethReader1);

        // reader 2 simply counts the number of messages
        auto* controller = ethReader2->ComAdapter()->CreateEthController("ETH1", "LINK1");
        controller->RegisterReceiveMessageHandler(
            [this](auto, auto) {
                numReceived2++;
            }
        );
    
        // run the simulation and check invariants
        for (auto index = 1u; index <= testMessages.size(); index++)
        {
            EXPECT_CALL(callbacks, AckHandler(MatchTxId(index))).Times(1);
        }
        EXPECT_CALL(callbacks, AckHandler(MatchTxId(0))).Times(0);


        EXPECT_TRUE(testHarness.Run(30s))
            << "TestHarness Timeout occurred!"
            << " numSent=" << numSent
            << " numAcked=" << numAcked
            << " numReceived=" << numReceived
            << " numReceived2=" << numReceived2;


        EXPECT_EQ(numAcked, numSent);
        EXPECT_EQ(numSent, numReceived);
        EXPECT_EQ(numSent, numReceived2);
        for (auto&& message : testMessages)
        {
            EXPECT_EQ(message.expectedData, message.receivedData);
        }

    }


protected:
    uint32_t domainId;
    //ib::cfg::Config ibConfig;
    std::vector<std::string> syncParticipantNames;

    struct TestMessage
    {
        std::string expectedData;
        std::string receivedData;
    };
    std::vector<TestMessage> testMessages;
    unsigned numSent{0},
        numReceived{0},
        numReceived2{0},
        numAcked{0};

    Callbacks callbacks;
};

TEST_F(ThreeEthControllerITest, test_eth_ack_callbacks)
{
    ExecuteTest();
}

//AFTMAGT-252: debug messages caused a segfault when the LogMsg RTPS topic
//             was already destroyed during teardown.
//   We are adding debug loggers here to verify that the logging
//   mechanism isn't affected by the ComAdapter' connection lifecycle and its
//   internal debugging/tracing calls.

auto makeLoggingConfig() -> ib::cfg::Config
{
    ib::cfg::ConfigBuilder builder{"DebugLogTestConfig"};
    auto &&setup = builder.SimulationSetup();
    auto& writer = setup.AddParticipant("EthWriter");
    writer->AddEthernet("ETH1").WithLink("LINK1");
    writer->ConfigureLogger()
        ->EnableLogFromRemotes()
        ->WithFlushLevel(ib::mw::logging::Level::Debug)
        ->AddSink(ib::cfg::Sink::Type::Stdout)
        .WithLogLevel(ib::mw::logging::Level::Debug);

    auto& reader1 = setup.AddParticipant("EthReader1");
    reader1->AddEthernet("ETH1").WithLink("LINK1");
    reader1->ConfigureLogger()
        ->EnableLogFromRemotes()
        ->WithFlushLevel(ib::mw::logging::Level::Debug)
        ->AddSink(ib::cfg::Sink::Type::Remote)
        .WithLogLevel(ib::mw::logging::Level::Debug);

    auto& reader2 = setup.AddParticipant("EthReader2");
    reader2->AddEthernet("ETH1").WithLink("LINK1");
    reader2->ConfigureLogger()
        ->EnableLogFromRemotes()
        ->WithFlushLevel(ib::mw::logging::Level::Debug)
        ->AddSink(ib::cfg::Sink::Type::Remote)
        .WithLogLevel(ib::mw::logging::Level::Debug);

    return builder.Build();
}


// TODO Reactivate after logging can be configured
//TEST_F(ThreeEthControllerITest, DISABLED_test_vasio_logging_orthogonal)
//{
//    ibConfig = makeLoggingConfig();
//    ibConfig.middlewareConfig.activeMiddleware = ib::cfg::Middleware::VAsio;
//
//    ExecuteTest();
//}

} // anonymous namespace
