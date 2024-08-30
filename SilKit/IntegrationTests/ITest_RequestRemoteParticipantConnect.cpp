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
#include "silkit/SilKit.hpp"
#include "SimTestHarness.hpp"


using namespace std::chrono_literals;
using namespace SilKit::Tests;


namespace {


std::mutex mx;

const std::string registryParticipantConfig = R"(
Logging:
  Sinks:
    - Type: Stdout
      #Level: Trace
)";

const std::string locConfNoProxy = R"(
Middleware:
  AcceptorUris: ["local://participant1.sock"]
  RegistryAsFallbackProxy: false
  ConnectAttempts: 1

Logging:
  Sinks:
    - Type: Stdout
      #Level: Trace
)";

const std::string tcpConfNoProxy = R"(
Middleware:
  EnableDomainSockets: false
  RegistryAsFallbackProxy: false
  ConnectAttempts: 1

Logging:
  Sinks:
    - Type: Stdout
      #Level: Trace
)";

std::vector<uint8_t> testData{0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7};

void TestDirectConnectionNotPossible(std::vector<std::string> participantConfigs, size_t pubParticipantIndex)
{
    // Idea: create participants which are unable to connect directly:
    //       one only accepts local domain connections, the other TCP
    ASSERT_EQ(participantConfigs.size(), 3u);
    ASSERT_LT(pubParticipantIndex, 3u);

    SimTestHarnessArgs simTestHarnessArgs;
    simTestHarnessArgs.syncParticipantNames = {"Participant1", "Participant2", "Participant3"};
    simTestHarnessArgs.deferParticipantCreation = true;
    simTestHarnessArgs.registry.participantConfiguration = registryParticipantConfig;

    SimTestHarness testSetup{simTestHarnessArgs};
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
    pubParticipant->GetOrCreateTimeSyncService()->SetSimulationStepHandler([&](auto&&, auto&&) {
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
    }, 1ms);

    ASSERT_TRUE(testSetup.Run(4s));
    ASSERT_GE(numDataReceived, 2);
}

// The following tests are set up such that the participants using the tcpConfNoProxy configuration are not able to
// connect directly to the participant using locConfNoProxy. No participant is allowed to use the proxy connection,
// which means that they are forced to initiate the connection using the remote-connect-request feature.
//
// As the order in which the participants connect to the registry is determining which participant attempts to connect
// directly to which other participants, and therefore also which participant requests the remote connection, we try all
// combinations.

TEST(ITest_RequestRemoteParticipantConnect, test_direct_connection_not_possible_a)
{
    TestDirectConnectionNotPossible({locConfNoProxy, tcpConfNoProxy, tcpConfNoProxy}, 0);
    TestDirectConnectionNotPossible({locConfNoProxy, tcpConfNoProxy, tcpConfNoProxy}, 1);
    TestDirectConnectionNotPossible({locConfNoProxy, tcpConfNoProxy, tcpConfNoProxy}, 2);
}

TEST(ITest_RequestRemoteParticipantConnect, test_direct_connection_not_possible_b)
{
    TestDirectConnectionNotPossible({tcpConfNoProxy, locConfNoProxy, tcpConfNoProxy}, 0);
    TestDirectConnectionNotPossible({tcpConfNoProxy, locConfNoProxy, tcpConfNoProxy}, 1);
    TestDirectConnectionNotPossible({tcpConfNoProxy, locConfNoProxy, tcpConfNoProxy}, 2);
}

TEST(ITest_RequestRemoteParticipantConnect, test_direct_connection_not_possible_c)
{
    TestDirectConnectionNotPossible({tcpConfNoProxy, tcpConfNoProxy, locConfNoProxy}, 0);
    TestDirectConnectionNotPossible({tcpConfNoProxy, tcpConfNoProxy, locConfNoProxy}, 1);
    TestDirectConnectionNotPossible({tcpConfNoProxy, tcpConfNoProxy, locConfNoProxy}, 2);
}

const auto registryConfigNoProxyNoRconn = R"(
Middleware:
  RegistryAsFallbackProxy: false
  ExperimentalRemoteParticipantConnection: false

Logging:
  Sinks:
    - Type: Stdout
      Level: Trace
)";

TEST(ITest_RequestRemoteParticipantConnect, test_timeout_if_registry_does_not_support_proxy_and_remote_connect)
{
    SimTestHarnessArgs simTestHarnessArgs;
    simTestHarnessArgs.syncParticipantNames = {"P1", "P2"};
    simTestHarnessArgs.deferParticipantCreation = true;
    simTestHarnessArgs.deferSystemControllerCreation = true;
    simTestHarnessArgs.registry.participantConfiguration = registryConfigNoProxyNoRconn;

    SimTestHarness simTestHarness{simTestHarnessArgs};

    // Create the first participant:
    // - connects to the registry
    (void)simTestHarness.GetParticipant("P1", locConfNoProxy);

    // Create the second participant:
    // - connects to the registry
    // - should fail to connect to the first participant because:
    //   - P1 only has local-domain acceptors
    //   - P2 does not connect to local-domain acceptors
    EXPECT_THROW((simTestHarness.GetParticipant("P2", tcpConfNoProxy)), SilKit::SilKitError);
}

const auto registryConfigWithProxyNoRconn = R"(
Middleware:
  RegistryAsFallbackProxy: true
  ExperimentalRemoteParticipantConnection: false

Logging:
  Sinks:
    - Type: Stdout
      Level: Trace
)";

const std::string locPCfgWithProxyAndRconn = R"(
Middleware:
  AcceptorUris: ["local://participant1.sock"]
  RegistryAsFallbackProxy: true
  ExperimentalRemoteParticipantConnection: true
  ConnectAttempts: 1

Logging:
  Sinks:
    - Type: Stdout
      #Level: Trace
)";

const std::string tcpPCfgWithProxyAndRconn = R"(
Middleware:
  EnableDomainSockets: false
  RegistryAsFallbackProxy: true
  ExperimentalRemoteParticipantConnection: true
  ConnectAttempts: 1

Logging:
  Sinks:
    - Type: Stdout
      #Level: Trace
)";

TEST(ITest_RequestRemoteParticipantConnect, test_fallback_to_proxy_if_registry_does_not_support_remote_connect)
{
    SimTestHarnessArgs simTestHarnessArgs;
    simTestHarnessArgs.syncParticipantNames = {"P1", "P2"};
    simTestHarnessArgs.deferParticipantCreation = true;
    simTestHarnessArgs.deferSystemControllerCreation = true;
    simTestHarnessArgs.registry.participantConfiguration = registryConfigWithProxyNoRconn;

    SimTestHarness simTestHarness{simTestHarnessArgs};

    // Create the first participant:
    // - connects to the registry
    (void)simTestHarness.GetParticipant("P1", locPCfgWithProxyAndRconn);

    // Create the second participant:
    // - connects to the registry
    // - should connect to the first participant because:
    //   - P1 only has local-domain acceptors
    //   - P2 does not connect to local-domain acceptors
    //   - registry does not support remote-connects
    //   - P1, P2, and registry do support proxy
    (void)simTestHarness.GetParticipant("P2", tcpPCfgWithProxyAndRconn);
}


} // namespace