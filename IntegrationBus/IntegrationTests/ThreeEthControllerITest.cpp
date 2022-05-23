// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <iostream>

#include "ib/sim/all.hpp"
#include "ib/util/functional.hpp"

#include "SimTestHarness.hpp"
#include "GetTestPid.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "EthDatatypeUtils.hpp"

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

auto MatchTxId(EthernetTxId transmitId) -> testing::Matcher<const EthernetFrameTransmitEvent&>
{
    using namespace testing;
    return Field(&EthernetFrameTransmitEvent::transmitId, transmitId);
}

class ThreeEthControllerITest : public testing::Test
{
protected:
    struct Callbacks
    {
        MOCK_METHOD1(AckHandler, void(const EthernetFrameTransmitEvent&));
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

        auto* controller = participant->Participant()->CreateEthernetController("ETH1", "LINK1");
        controller->AddFrameTransmitHandler(
            [this, participant](IEthernetController*, const EthernetFrameTransmitEvent& ack) {
            std::cout << participant->Name() << " <- EthTransmitAck" << std::endl;
            callbacks.AckHandler(ack);
            numAcked++;
        });
        auto* participantController = 
            participant->Participant()->GetParticipantController();
        participantController->SetSimulationTask(
            [this, participant, controller](std::chrono::nanoseconds now, std::chrono::nanoseconds) {
                if (numSent < testMessages.size())
                {
                    const auto& message = testMessages.at(numSent);

                    std::cout << participant->Name() << " -> sent frame @" << now.count() << "ns" << std::endl;

                    EthernetMac destinationMac{ 0x12, 0x23, 0x45, 0x67, 0x89, 0x9a };
                    EthernetMac sourceMac{ 0x9a, 0x89, 0x67, 0x45, 0x23, 0x12 };
                    EthernetEtherType etherType{ 0x0800 };
                    EthernetVlanTagControlIdentifier tci{ 0x0000 };

                    auto ethernetFrame = CreateEthernetFrameWithVlanTag(destinationMac, sourceMac, etherType, message.expectedData, tci);
                    controller->SendFrame(ethernetFrame);
                    numSent++;
                    std::this_thread::sleep_for(100ms);
                }
        });
    }

    void SetupReceiver(ib::test::SimParticipant* participant)
    {
        std::cout << " Receiver init " << participant->Name() << std::endl;
        auto* controller = participant->Participant()->CreateEthernetController("ETH1", "LINK1");
        controller->AddFrameTransmitHandler(
            [this](IEthernetController* , const EthernetFrameTransmitEvent& ack) {
            callbacks.AckHandler(ack);
        });

        controller->AddFrameHandler(
            [this, participant](IEthernetController*, const EthernetFrameEvent& event) {
                const auto& frame = event.frame;
                std::string message(frame.raw.begin() + EthernetFrameHeaderSize + EthernetFrameVlanTagSize, frame.raw.end());
                std::cout << participant->Name() 
                    <<" <- received frame"
                    << ": frame size=" << frame.raw.size()
                    << ": testMessages size=" << testMessages.size()
                    << ": receiveCount=" << numReceived
                    << ": message='" << message << "'"
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
        auto* controller = ethReader2->Participant()->CreateEthernetController("ETH1", "LINK1");
        controller->AddFrameHandler(
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
//   mechanism isn't affected by the Participant' connection lifecycle and its
//   internal debugging/tracing calls.

// TODO Reactivate after logging can be configured
//TEST_F(ThreeEthControllerITest, DISABLED_test_vasio_logging_orthogonal)
//{
//    ibConfig = makeLoggingConfig();
//    ibConfig.middlewareConfig.activeMiddleware = ib::cfg::Middleware::VAsio;
//
//    ExecuteTest();
//}

} // anonymous namespace
