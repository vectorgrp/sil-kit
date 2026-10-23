// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

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

const std::string anyPCfgWithProxy = R"(
Middleware:
  RegistryAsFallbackProxy: true
  ConnectAttempts: 1

Logging:
  Sinks:
    - Type: Stdout
      #Level: Trace
)";

TEST(ITest_RequestRemoteParticipantConnect, test_publish_to_a_direct_and_a_proxied_subscriber)
{
    // P1 only accepts local-domain connections and P2 does not use them, so P2 reaches P1 through the
    // registry proxy. P3 connects to both directly. P1 then publishes to one direct and one proxied peer,
    // with payloads large enough that the message body is shared between both.
    SimTestHarnessArgs simTestHarnessArgs;
    simTestHarnessArgs.syncParticipantNames = {"P1", "P2", "P3"};
    simTestHarnessArgs.deferParticipantCreation = true;
    simTestHarnessArgs.registry.participantConfiguration = registryConfigWithProxyNoRconn;

    SimTestHarness testSetup{simTestHarnessArgs};

    auto* publisherParticipant = testSetup.GetParticipant("P1", locPCfgWithProxyAndRconn);
    auto* proxiedParticipant = testSetup.GetParticipant("P2", tcpPCfgWithProxyAndRconn);
    auto* directParticipant = testSetup.GetParticipant("P3", anyPCfgWithProxy);

    std::vector<uint8_t> payload(4096);
    for (size_t i = 0; i < payload.size(); ++i)
    {
        payload[i] = static_cast<uint8_t>(i * 31 + 7);
    }

    const auto pubSubSpec = SilKit::Services::PubSub::PubSubSpec{};
    uint64_t numReceivedProxied{};
    uint64_t numReceivedDirect{};

    proxiedParticipant->Participant()->CreateDataSubscriber("test", pubSubSpec, [&](auto&&, auto&& data) {
        EXPECT_EQ(payload, ToStdVector(data.data));
        std::unique_lock<decltype(mx)> lock{mx};
        numReceivedProxied++;
    });

    directParticipant->Participant()->CreateDataSubscriber("test", pubSubSpec, [&](auto&&, auto&& data) {
        EXPECT_EQ(payload, ToStdVector(data.data));
        std::unique_lock<decltype(mx)> lock{mx};
        numReceivedDirect++;
    });

    auto* publisher = publisherParticipant->Participant()->CreateDataPublisher("test", pubSubSpec);
    publisherParticipant->GetOrCreateTimeSyncService()->SetSimulationStepHandler([&](auto&&, auto&&) {
        std::unique_lock<decltype(mx)> lock{mx};
        if (numReceivedProxied >= 1 && numReceivedDirect >= 1)
        {
            publisherParticipant->Stop();
        }
        else
        {
            publisher->Publish(payload);
        }
    }, 1ms);

    ASSERT_TRUE(testSetup.Run(4s));
    EXPECT_GE(numReceivedProxied, 1u);
    EXPECT_GE(numReceivedDirect, 1u);
}


} // namespace