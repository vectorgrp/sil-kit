// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "config/ConfigurationTestUtils.hpp"

#include "util/Uri.hpp"

#include "core/vasio/VAsioRegistry.hpp"
#include "core/vasio/io/MakeAsioIoContext.hpp"
#include "core/vasio/mock/FakeVAsioPeer.hpp"

#include "services/logging/MockLogger.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"


namespace {


TEST(Test_VAsioRegistry, check_start_listening)
{
    using UriType = SilKit::Core::Uri::UriType;

    auto configuration{SilKit::Config::MakeEmptyParticipantConfigurationImpl()};

    {
        SilKit::Core::VAsioRegistry vAsioRegistry{configuration};
        SilKit::Core::Uri registryUri{vAsioRegistry.StartListening("silkit://localhost:0")};
        ASSERT_EQ(registryUri.Type(), UriType::SilKit);
        ASSERT_EQ(registryUri.Host(), "localhost");
        ASSERT_NE(registryUri.Port(), 0);
    }

    {
        SilKit::Core::VAsioRegistry vAsioRegistry{configuration};
        SilKit::Core::Uri registryUri{vAsioRegistry.StartListening("silkit://0.0.0.0:0")};
        ASSERT_EQ(registryUri.Type(), UriType::SilKit);
        ASSERT_EQ(registryUri.Host(), "0.0.0.0");
        ASSERT_NE(registryUri.Port(), 0);
    }

    {
        SilKit::Core::VAsioRegistry vAsioRegistry{configuration};
        SilKit::Core::Uri registryUri{vAsioRegistry.StartListening("silkit://127.0.0.1:0")};
        ASSERT_EQ(registryUri.Type(), UriType::SilKit);
        ASSERT_EQ(registryUri.Host(), "127.0.0.1");
        ASSERT_NE(registryUri.Port(), 0);
    }
}

TEST(Test_VAsioRegistry, start_listening_on_invalid_address_throws)
{
    auto configuration{SilKit::Config::MakeEmptyParticipantConfigurationImpl()};

    SilKit::Core::VAsioRegistry vAsioRegistry{configuration};
    EXPECT_THROW(vAsioRegistry.StartListening("silkit://333.0.0.0:8501"), SilKit::ConfigurationError);
}

auto HasIpV6Loopback() -> bool
{
    ::testing::NiceMock<SilKit::Services::Logging::MockLogger> logger;

    auto ioContext{VSilKit::MakeAsioIoContext({})};
    ioContext->SetLogger(logger);

    try
    {
        ioContext->MakeTcpAcceptor("::1", 0);
        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

TEST(Test_VAsioRegistry, start_listening_on_ipv6_literal_uses_tcp)
{
    if (!HasIpV6Loopback())
    {
        GTEST_SKIP() << "IPv6 loopback is not available";
    }

    auto configuration{SilKit::Config::MakeEmptyParticipantConfigurationImpl()};

    SilKit::Core::VAsioRegistry vAsioRegistry{configuration};
    SilKit::Core::Uri registryUri{vAsioRegistry.StartListening("silkit://[::1]:0")};
    ASSERT_EQ(registryUri.Host(), "[::1]");
    // the port stays 0 if the registry fell back to domain sockets only
    ASSERT_NE(registryUri.Port(), 0);
}


} // namespace

namespace SilKit {
namespace Core {

struct VAsioRegistryTestAccess
{
    static void Announce(VAsioRegistry& registry, IVAsioPeer* peer, const std::string& simulationName)
    {
        ParticipantAnnouncement announcement{};
        announcement.peerInfo = peer->GetInfo();
        announcement.simulationName = simulationName;
        SerializedMessage message{announcement};
        registry._connection.OnSocketData(peer, message);
    }

    static void Shutdown(VAsioRegistry& registry, IVAsioPeer* peer)
    {
        // The connection calls the registry before it removes the peer. The callback is registered on the io thread of
        // the connection, which does not run in the tests, so it is called here.
        registry.OnPeerShutdown(peer);
        registry._connection.OnPeerShutdown(peer);
    }

    static bool IsPeerRegistered(const VAsioRegistry& registry, const IVAsioPeer* peer)
    {
        return registry._connection.GetPeerRegistration(peer).registered;
    }

    static void SendStatus(VAsioRegistry& registry, IVAsioPeer* peer,
                           SilKit::Services::Orchestration::ParticipantState state)
    {
        SilKit::Services::Orchestration::ParticipantStatus status;
        status.participantName = peer->GetInfo().participantName;
        status.state = state;
        registry.ReceiveMsg(peer, status);
    }

    static void SendWorkflowConfiguration(VAsioRegistry& registry, IVAsioPeer* peer,
                                          std::vector<std::string> requiredParticipantNames)
    {
        SilKit::Services::Orchestration::WorkflowConfiguration configuration;
        configuration.requiredParticipantNames = std::move(requiredParticipantNames);
        registry.ReceiveMsg(peer, configuration);
    }
};

} // namespace Core
} // namespace SilKit

namespace {

using SilKit::Core::FakeVAsioPeer;
using SilKit::Core::VAsioRegistry;
using SilKit::Core::VAsioRegistryTestAccess;

// The placement of peers into generations is covered by Test_SessionPlacement, these tests cover that the registry
// keeps a placement per simulation name and reads the registered peers from the connection.
class Test_VAsioRegistrySimulations : public testing::Test
{
protected:
    Test_VAsioRegistrySimulations()
        : _registry{SilKit::Config::MakeEmptyParticipantConfigurationImpl()}
    {
    }

    VAsioRegistry _registry;
};

TEST_F(Test_VAsioRegistrySimulations, announcing_a_peer_sends_the_known_participants_and_registers_it)
{
    FakeVAsioPeer a{"A", "Sim"};
    FakeVAsioPeer b{"B", "Sim"};

    // the known participants and the announcement reply
    EXPECT_CALL(a, SendSilKitMsg(testing::An<SilKit::Core::SerializedMessage>())).Times(2);
    EXPECT_CALL(b, SendSilKitMsg(testing::An<SilKit::Core::SerializedMessage>())).Times(2);

    VAsioRegistryTestAccess::Announce(_registry, &a, "Sim");
    VAsioRegistryTestAccess::Announce(_registry, &b, "Sim");

    EXPECT_TRUE(VAsioRegistryTestAccess::IsPeerRegistered(_registry, &a));
    EXPECT_TRUE(VAsioRegistryTestAccess::IsPeerRegistered(_registry, &b));
}

TEST_F(Test_VAsioRegistrySimulations, peers_with_the_same_name_are_placed_into_different_generations)
{
    FakeVAsioPeer a1{"A", "Sim"};
    FakeVAsioPeer a2{"A", "Sim"};

    VAsioRegistryTestAccess::Announce(_registry, &a1, "Sim");
    VAsioRegistryTestAccess::Announce(_registry, &a2, "Sim");

    EXPECT_EQ(a1.GetServiceDescriptor().GetGeneration(), 1u);
    EXPECT_EQ(a2.GetServiceDescriptor().GetGeneration(), 2u);
    EXPECT_TRUE(VAsioRegistryTestAccess::IsPeerRegistered(_registry, &a1));
    EXPECT_TRUE(VAsioRegistryTestAccess::IsPeerRegistered(_registry, &a2));
}

TEST_F(Test_VAsioRegistrySimulations, participant_with_same_name_but_different_simulation_name_gets_its_own_generation)
{
    FakeVAsioPeer a1{"A", "Sim1"};
    FakeVAsioPeer a2{"A", "Sim2"};

    VAsioRegistryTestAccess::Announce(_registry, &a1, "Sim1");
    EXPECT_EQ(a1.GetServiceDescriptor().GetGeneration(), 1u);

    VAsioRegistryTestAccess::Announce(_registry, &a2, "Sim2");
    EXPECT_EQ(a2.GetServiceDescriptor().GetGeneration(), 1u);

    VAsioRegistryTestAccess::Shutdown(_registry, &a1);
    EXPECT_FALSE(VAsioRegistryTestAccess::IsPeerRegistered(_registry, &a1));
    EXPECT_TRUE(VAsioRegistryTestAccess::IsPeerRegistered(_registry, &a2));

    VAsioRegistryTestAccess::Announce(_registry, &a1, "Sim1");
    EXPECT_EQ(a1.GetServiceDescriptor().GetGeneration(), 1u);
}

TEST_F(Test_VAsioRegistrySimulations, status_and_workflow_configuration_reach_the_placement_of_the_sender)
{
    using ParticipantState = SilKit::Services::Orchestration::ParticipantState;

    FakeVAsioPeer a{"A", "Sim"};
    FakeVAsioPeer b{"B", "Sim"};

    VAsioRegistryTestAccess::Announce(_registry, &a, "Sim");
    VAsioRegistryTestAccess::Announce(_registry, &b, "Sim");

    VAsioRegistryTestAccess::SendWorkflowConfiguration(_registry, &a, {"A", "B"});
    for (auto* peer : {&a, &b})
    {
        for (const auto state : {ParticipantState::ServicesCreated, ParticipantState::CommunicationInitializing,
                                 ParticipantState::CommunicationInitialized, ParticipantState::ReadyToRun})
        {
            VAsioRegistryTestAccess::SendStatus(_registry, peer, state);
        }
    }

    // the simulation has started, so the coordinated participant is placed into a new generation when it rejoins
    VAsioRegistryTestAccess::Shutdown(_registry, &a);
    VAsioRegistryTestAccess::Announce(_registry, &a, "Sim");
    EXPECT_EQ(a.GetServiceDescriptor().GetGeneration(), 2u);
}

TEST_F(Test_VAsioRegistrySimulations, state_of_a_generation_is_dropped_when_its_last_peer_shuts_down)
{
    using ParticipantState = SilKit::Services::Orchestration::ParticipantState;

    FakeVAsioPeer a1{"A", "Sim"};
    FakeVAsioPeer b1{"B", "Sim"};
    FakeVAsioPeer a2{"A", "Sim"};
    FakeVAsioPeer b2{"B", "Sim"};

    // generation 1 keeps the placement of the simulation alive
    VAsioRegistryTestAccess::Announce(_registry, &a1, "Sim");
    VAsioRegistryTestAccess::Announce(_registry, &b1, "Sim");

    VAsioRegistryTestAccess::Announce(_registry, &a2, "Sim");
    VAsioRegistryTestAccess::Announce(_registry, &b2, "Sim");
    ASSERT_EQ(b2.GetServiceDescriptor().GetGeneration(), 2u);
    VAsioRegistryTestAccess::SendWorkflowConfiguration(_registry, &a2, {"A", "B"});
    for (auto* peer : {&a2, &b2})
    {
        for (const auto state : {ParticipantState::ServicesCreated, ParticipantState::CommunicationInitializing,
                                 ParticipantState::CommunicationInitialized, ParticipantState::ReadyToRun})
        {
            VAsioRegistryTestAccess::SendStatus(_registry, peer, state);
        }
    }

    VAsioRegistryTestAccess::Shutdown(_registry, &a2);
    VAsioRegistryTestAccess::Shutdown(_registry, &b2);

    // generation 2 is reused, and its started state must not keep the required participant B out
    VAsioRegistryTestAccess::Announce(_registry, &a2, "Sim");
    EXPECT_EQ(a2.GetServiceDescriptor().GetGeneration(), 2u);
    VAsioRegistryTestAccess::Announce(_registry, &b2, "Sim");
    EXPECT_EQ(b2.GetServiceDescriptor().GetGeneration(), 2u);
}

TEST_F(Test_VAsioRegistrySimulations, repeated_shutdown_does_not_remove_a_peer_that_took_the_name_over)
{
    FakeVAsioPeer a{"A", "Sim"};
    FakeVAsioPeer b{"B", "Sim"};
    FakeVAsioPeer aRejoined{"A", "Sim"};

    VAsioRegistryTestAccess::Announce(_registry, &a, "Sim");
    VAsioRegistryTestAccess::Announce(_registry, &b, "Sim");
    VAsioRegistryTestAccess::Shutdown(_registry, &a);

    VAsioRegistryTestAccess::Announce(_registry, &aRejoined, "Sim");
    ASSERT_TRUE(VAsioRegistryTestAccess::IsPeerRegistered(_registry, &aRejoined));

    // the stale peer still carries the generation, but is no longer registered
    EXPECT_FALSE(VAsioRegistryTestAccess::IsPeerRegistered(_registry, &a));
    VAsioRegistryTestAccess::Shutdown(_registry, &a);

    EXPECT_TRUE(VAsioRegistryTestAccess::IsPeerRegistered(_registry, &aRejoined));
}

TEST_F(Test_VAsioRegistrySimulations, all_disconnected_handler_is_called_when_the_last_simulation_is_empty)
{
    int calls{0};
    _registry.SetAllDisconnectedHandler([&calls] { ++calls; });

    FakeVAsioPeer a{"A", "Sim1"};
    FakeVAsioPeer b{"B", "Sim2"};

    VAsioRegistryTestAccess::Announce(_registry, &a, "Sim1");
    VAsioRegistryTestAccess::Announce(_registry, &b, "Sim2");

    VAsioRegistryTestAccess::Shutdown(_registry, &a);
    EXPECT_EQ(calls, 0);

    VAsioRegistryTestAccess::Shutdown(_registry, &b);
    EXPECT_EQ(calls, 1);
}

TEST_F(Test_VAsioRegistrySimulations, all_disconnected_handler_waits_for_the_peers_of_all_generations)
{
    int calls{0};
    _registry.SetAllDisconnectedHandler([&calls] { ++calls; });

    FakeVAsioPeer a1{"A", "Sim"};
    FakeVAsioPeer a2{"A", "Sim"};

    VAsioRegistryTestAccess::Announce(_registry, &a1, "Sim");
    VAsioRegistryTestAccess::Announce(_registry, &a2, "Sim");

    VAsioRegistryTestAccess::Shutdown(_registry, &a1);
    EXPECT_EQ(calls, 0);
    EXPECT_TRUE(VAsioRegistryTestAccess::IsPeerRegistered(_registry, &a2));

    VAsioRegistryTestAccess::Shutdown(_registry, &a2);
    EXPECT_EQ(calls, 1);
}

} // namespace
