/* Copyright (c) 2023 Vector Informatik GmbH

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
#include <stdexcept>
#include <thread>
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "GetTestPid.hpp"
#include "silkit/SilKit.hpp"
#include "SimTestHarness.hpp"
using namespace std::chrono_literals;
using namespace SilKit::Tests;
namespace {

std::mutex mx;
const auto registryUri = MakeTestRegistryUri();
auto locParticipantConfig = R"(
Middleware:
  AcceptorUris: ["local://participant1.sock"]
  RegistryAsFallbackProxy: false
  ConnectAttempts: 1
Logging:
  Sinks:
    - Type: Stdout
      # Level: Trace
)";
auto tcpParticipantConfig = R"(
Middleware:
  EnableDomainSockets: false
  RegistryAsFallbackProxy: false
  ConnectAttempts: 1
Logging:
  Sinks:
    - Type: Stdout
      # Level: Trace
)";
std::vector<uint8_t> testData{0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7};

void TestDirectConnectionNotPossible(std::vector<std::string> participantConfigs, size_t pubParticipantIndex)
{
    // Idea: create participants which are unable to connect directly:
    //       one only accepts local domain connections, the other TCP
    ASSERT_EQ(participantConfigs.size(), 3u);
    ASSERT_LT(pubParticipantIndex, 3u);

    SimTestHarness testSetup{{"Participant1", "Participant2", "Participant3"}, registryUri, true};
    // Must not throw exceptions:
    std::vector<SimParticipant*> participants;
    for (size_t index = 0; index != 3; ++index)
    {
        auto&& participantName{"Participant" + std::to_string(index + 1)};
        auto&& participantConfig{participantConfigs[index]};
        participants.emplace_back(testSetup.GetParticipant(participantName, participantConfig));
    }
    
    SimParticipant* pubParticipant = participants[pubParticipantIndex];
    SimParticipant* subParticipant1 = participants[(pubParticipantIndex + 1) % 3];
    SimParticipant* subParticipant2 = participants[(pubParticipantIndex + 2) % 3];

    // check that we can exchange messages
    auto pubSubSpec = SilKit::Services::PubSub::PubSubSpec{};
    uint64_t numDataReceived{};

    subParticipant1->Participant()->CreateDataSubscriber("test", pubSubSpec, [&](auto&&, auto&& data) {
        ASSERT_EQ(testData, ToStdVector(data.data));
        std::unique_lock<decltype(mx)> lock{mx};
        numDataReceived++;
        std::cout << " ----> Data received!" << std::endl;
    });

    subParticipant2->Participant()->CreateDataSubscriber("test", pubSubSpec, [&](auto&&, auto&& data) {
        ASSERT_EQ(testData, ToStdVector(data.data));
        std::unique_lock<decltype(mx)> lock{mx};
        numDataReceived++;
        std::cout << " ----> Data received!" << std::endl;
    });

    auto publisher1 = pubParticipant->Participant()->CreateDataPublisher("test", pubSubSpec);
    pubParticipant->GetOrCreateTimeSyncService()->SetSimulationStepHandler(
        [&](auto&&, auto&&) {
            std::unique_lock<decltype(mx)> lock{mx};
            if (numDataReceived >= 2)
            {
                pubParticipant->Stop();
            }
            else
            {
                std::cout << "<---- published" << std::endl;
                publisher1->Publish(testData);
            }
        },
        1ms);

    ASSERT_TRUE(testSetup.Run(4s));
    ASSERT_GE(numDataReceived, 2);
}

TEST(ITest_RequestRemoteParticipantConnect, test_direct_connection_not_possible_a)
{
    TestDirectConnectionNotPossible({locParticipantConfig, tcpParticipantConfig, tcpParticipantConfig}, 0);
    TestDirectConnectionNotPossible({locParticipantConfig, tcpParticipantConfig, tcpParticipantConfig}, 1);
    TestDirectConnectionNotPossible({locParticipantConfig, tcpParticipantConfig, tcpParticipantConfig}, 2);
}

TEST(ITest_RequestRemoteParticipantConnect, test_direct_connection_not_possible_b)
{
    TestDirectConnectionNotPossible({tcpParticipantConfig, locParticipantConfig, tcpParticipantConfig}, 0);
    TestDirectConnectionNotPossible({tcpParticipantConfig, locParticipantConfig, tcpParticipantConfig}, 1);
    TestDirectConnectionNotPossible({tcpParticipantConfig, locParticipantConfig, tcpParticipantConfig}, 2);
}

TEST(ITest_RequestRemoteParticipantConnect, test_direct_connection_not_possible_c)
{
    TestDirectConnectionNotPossible({tcpParticipantConfig, tcpParticipantConfig, locParticipantConfig}, 0);
    TestDirectConnectionNotPossible({tcpParticipantConfig, tcpParticipantConfig, locParticipantConfig}, 1);
    TestDirectConnectionNotPossible({tcpParticipantConfig, tcpParticipantConfig, locParticipantConfig}, 2);
}

} /* anonymous namespace */