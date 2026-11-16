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

    static bool HasPlacement(const VAsioRegistry& registry, const std::string& simulationName)
    {
        return registry.FindPlacement(simulationName) != nullptr;
    }

    static auto UseMockLogger(VAsioRegistry& registry,
                              std::unique_ptr<SilKit::Services::Logging::ILoggerInternal>& originalLogger)
        -> SilKit::Services::Logging::MockLogger&
    {
        auto logger{std::make_unique<testing::NiceMock<SilKit::Services::Logging::MockLogger>>()};
        auto& result{*logger};
        EXPECT_CALL(result, ProcessLoggerMessage(testing::_)).Times(testing::AnyNumber());
        originalLogger = std::move(registry._logger);
        registry._logger = std::move(logger);
        return result;
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

    std::unique_ptr<SilKit::Services::Logging::ILoggerInternal> _originalLogger;
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
    FakeVAsioPeer a1{"A", "session/Sim"};
    FakeVAsioPeer a2{"A", "session/Sim"};

    VAsioRegistryTestAccess::Announce(_registry, &a1, "session/Sim");
    VAsioRegistryTestAccess::Announce(_registry, &a2, "session/Sim");

    EXPECT_EQ(a1.GetServiceDescriptor().GetGeneration(), 1u);
    EXPECT_EQ(a2.GetServiceDescriptor().GetGeneration(), 2u);
    EXPECT_TRUE(VAsioRegistryTestAccess::IsPeerRegistered(_registry, &a1));
    EXPECT_TRUE(VAsioRegistryTestAccess::IsPeerRegistered(_registry, &a2));
}

TEST_F(Test_VAsioRegistrySimulations, participant_with_same_name_but_different_simulation_name_gets_its_own_generation)
{
    FakeVAsioPeer a1{"A", "session/Sim1"};
    FakeVAsioPeer a2{"A", "session/Sim2"};

    VAsioRegistryTestAccess::Announce(_registry, &a1, "session/Sim1");
    EXPECT_EQ(a1.GetServiceDescriptor().GetGeneration(), 1u);

    VAsioRegistryTestAccess::Announce(_registry, &a2, "session/Sim2");
    EXPECT_EQ(a2.GetServiceDescriptor().GetGeneration(), 1u);

    VAsioRegistryTestAccess::Shutdown(_registry, &a1);
    EXPECT_FALSE(VAsioRegistryTestAccess::IsPeerRegistered(_registry, &a1));
    EXPECT_TRUE(VAsioRegistryTestAccess::IsPeerRegistered(_registry, &a2));

    VAsioRegistryTestAccess::Announce(_registry, &a1, "session/Sim1");
    EXPECT_EQ(a1.GetServiceDescriptor().GetGeneration(), 1u);
}

TEST_F(Test_VAsioRegistrySimulations, status_and_workflow_configuration_reach_the_placement_of_the_sender)
{
    using ParticipantState = SilKit::Services::Orchestration::ParticipantState;

    FakeVAsioPeer a{"A", "session/Sim"};
    FakeVAsioPeer b{"B", "session/Sim"};

    VAsioRegistryTestAccess::Announce(_registry, &a, "session/Sim");
    VAsioRegistryTestAccess::Announce(_registry, &b, "session/Sim");

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
    VAsioRegistryTestAccess::Announce(_registry, &a, "session/Sim");
    EXPECT_EQ(a.GetServiceDescriptor().GetGeneration(), 2u);
}

TEST_F(Test_VAsioRegistrySimulations, multiple_running_generations_coexist_and_are_removed_after_shutdown)
{
    using ParticipantState = SilKit::Services::Orchestration::ParticipantState;
    using SilKit::Services::Logging::ALoggerMessageWith;
    using SilKit::Services::Logging::Level;

    auto& logger{VAsioRegistryTestAccess::UseMockLogger(_registry, _originalLogger)};
    for (const auto* message : {"The simulation 'session/Sim' in generation 1 has started",
                                "The simulation 'session/Sim' in generation 2 has started"})
    {
        EXPECT_CALL(logger, ProcessLoggerMessage(ALoggerMessageWith(Level::Info, message))).Times(1);
    }
    int disconnectedCalls{0};
    _registry.SetAllDisconnectedHandler([&disconnectedCalls] { ++disconnectedCalls; });
    FakeVAsioPeer a1{"A", "session/Sim"};
    FakeVAsioPeer a2{"A", "session/Sim"};
    FakeVAsioPeer b1{"B", "session/Sim"};
    FakeVAsioPeer b2{"B", "session/Sim"};
    FakeVAsioPeer controller1{"Controller", "session/Sim"};
    FakeVAsioPeer controller2{"Controller", "session/Sim"};

    for (auto* peer : {&a1, &a2, &b1, &b2, &controller1, &controller2})
    {
        VAsioRegistryTestAccess::Announce(_registry, peer, "session/Sim");
        ASSERT_TRUE(VAsioRegistryTestAccess::IsPeerRegistered(_registry, peer));
    }
    for (auto* peer : {&a1, &b1, &controller1})
    {
        EXPECT_EQ(peer->GetServiceDescriptor().GetGeneration(), 1u);
    }
    for (auto* peer : {&a2, &b2, &controller2})
    {
        EXPECT_EQ(peer->GetServiceDescriptor().GetGeneration(), 2u);
    }

    for (auto* controller : {&controller1, &controller2})
    {
        VAsioRegistryTestAccess::SendWorkflowConfiguration(_registry, controller, {"A", "B"});
    }
    for (const auto state : {ParticipantState::ServicesCreated, ParticipantState::CommunicationInitializing,
                             ParticipantState::CommunicationInitialized, ParticipantState::ReadyToRun,
                             ParticipantState::Running})
    {
        for (auto* peer : {&a1, &b1, &a2, &b2})
        {
            VAsioRegistryTestAccess::SendStatus(_registry, peer, state);
        }
    }
    testing::Mock::VerifyAndClearExpectations(&logger);

    for (auto* peer : {&a1, &b1, &controller1})
    {
        VAsioRegistryTestAccess::Shutdown(_registry, peer);
        EXPECT_FALSE(VAsioRegistryTestAccess::IsPeerRegistered(_registry, peer));
        EXPECT_EQ(disconnectedCalls, 0);
    }
    EXPECT_TRUE(VAsioRegistryTestAccess::HasPlacement(_registry, "session/Sim"));
    for (auto* peer : {&a2, &b2, &controller2})
    {
        EXPECT_TRUE(VAsioRegistryTestAccess::IsPeerRegistered(_registry, peer));
    }

    for (auto* peer : {&a2, &b2})
    {
        VAsioRegistryTestAccess::Shutdown(_registry, peer);
        EXPECT_FALSE(VAsioRegistryTestAccess::IsPeerRegistered(_registry, peer));
        EXPECT_EQ(disconnectedCalls, 0);
        EXPECT_TRUE(VAsioRegistryTestAccess::HasPlacement(_registry, "session/Sim"));
    }
    VAsioRegistryTestAccess::Shutdown(_registry, &controller2);
    EXPECT_FALSE(VAsioRegistryTestAccess::IsPeerRegistered(_registry, &controller2));
    EXPECT_FALSE(VAsioRegistryTestAccess::HasPlacement(_registry, "session/Sim"));
    EXPECT_EQ(disconnectedCalls, 1);

    VAsioRegistryTestAccess::Announce(_registry, &a1, "session/Sim");
    EXPECT_EQ(a1.GetServiceDescriptor().GetGeneration(), 1u);
    VAsioRegistryTestAccess::Shutdown(_registry, &a1);
}

TEST_F(Test_VAsioRegistrySimulations, workflow_contract_survives_generation_removal_and_resets_with_simulation)
{
    using SilKit::Services::Logging::ALoggerMessageWith;
    using SilKit::Services::Logging::Level;

    auto& logger{VAsioRegistryTestAccess::UseMockLogger(_registry, _originalLogger)};
    FakeVAsioPeer controller1{"Controller", "session/Sim"};
    FakeVAsioPeer controller2{"Controller", "session/Sim"};
    FakeVAsioPeer otherController{"OtherController", "session/Sim"};
    VAsioRegistryTestAccess::Announce(_registry, &controller1, "session/Sim");
    VAsioRegistryTestAccess::Announce(_registry, &controller2, "session/Sim");
    VAsioRegistryTestAccess::SendWorkflowConfiguration(_registry, &controller1, {"A", "B"});
    VAsioRegistryTestAccess::Shutdown(_registry, &controller1);
    ASSERT_TRUE(VAsioRegistryTestAccess::HasPlacement(_registry, "session/Sim"));

    // Removing the original generation must not allow a different required-name set.
    EXPECT_CALL(logger, ProcessLoggerMessage(ALoggerMessageWith(Level::Error))).Times(1);
    VAsioRegistryTestAccess::SendWorkflowConfiguration(_registry, &controller2, {"A"});
    testing::Mock::VerifyAndClearExpectations(&logger);
    EXPECT_CALL(logger, ProcessLoggerMessage(testing::_)).Times(testing::AnyNumber());
    // Reordered names are equivalent. Keep this expectation through the fresh simulation below.
    EXPECT_CALL(logger, ProcessLoggerMessage(ALoggerMessageWith(Level::Error))).Times(0);
    VAsioRegistryTestAccess::SendWorkflowConfiguration(_registry, &controller2, {"B", "A"});
    VAsioRegistryTestAccess::Shutdown(_registry, &controller2);
    ASSERT_FALSE(VAsioRegistryTestAccess::HasPlacement(_registry, "session/Sim"));

    // With no peers left, both the generation counter and workflow contract reset.
    VAsioRegistryTestAccess::Announce(_registry, &otherController, "session/Sim");
    EXPECT_EQ(otherController.GetServiceDescriptor().GetGeneration(), 1u);
    VAsioRegistryTestAccess::SendWorkflowConfiguration(_registry, &otherController, {"C"});
    VAsioRegistryTestAccess::Shutdown(_registry, &otherController);
}

TEST_F(Test_VAsioRegistrySimulations, state_of_a_generation_is_dropped_when_its_last_peer_shuts_down)
{
    using ParticipantState = SilKit::Services::Orchestration::ParticipantState;

    FakeVAsioPeer a1{"A", "session/Sim"};
    FakeVAsioPeer b1{"B", "session/Sim"};
    FakeVAsioPeer a2{"A", "session/Sim"};
    FakeVAsioPeer b2{"B", "session/Sim"};

    // generation 1 keeps the placement of the simulation alive
    VAsioRegistryTestAccess::Announce(_registry, &a1, "session/Sim");
    VAsioRegistryTestAccess::Announce(_registry, &b1, "session/Sim");

    VAsioRegistryTestAccess::Announce(_registry, &a2, "session/Sim");
    VAsioRegistryTestAccess::Announce(_registry, &b2, "session/Sim");
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
    VAsioRegistryTestAccess::Announce(_registry, &a2, "session/Sim");
    EXPECT_EQ(a2.GetServiceDescriptor().GetGeneration(), 2u);
    VAsioRegistryTestAccess::Announce(_registry, &b2, "session/Sim");
    EXPECT_EQ(b2.GetServiceDescriptor().GetGeneration(), 2u);
}

TEST_F(Test_VAsioRegistrySimulations, repeated_shutdown_does_not_remove_a_peer_that_took_the_name_over)
{
    using SilKit::Services::Logging::ALoggerMessageWith;
    using SilKit::Services::Logging::Level;

    auto& logger{VAsioRegistryTestAccess::UseMockLogger(_registry, _originalLogger)};
    EXPECT_CALL(logger, ProcessLoggerMessage(ALoggerMessageWith(
                            Level::Debug, "Duplicate peer 'A' has shut down, which had no participant information")))
        .Times(2);
    for (const auto& simulationName : {"Sim", "session/x"})
    {
        FakeVAsioPeer a{"A", simulationName};
        FakeVAsioPeer b{"B", simulationName};
        FakeVAsioPeer aRejoined{"A", simulationName};

        VAsioRegistryTestAccess::Announce(_registry, &a, simulationName);
        VAsioRegistryTestAccess::Announce(_registry, &b, simulationName);
        VAsioRegistryTestAccess::Shutdown(_registry, &a);

        VAsioRegistryTestAccess::Announce(_registry, &aRejoined, simulationName);
        ASSERT_TRUE(VAsioRegistryTestAccess::IsPeerRegistered(_registry, &aRejoined));

        // The stale peer still carries the generation in session mode, but is no longer registered.
        EXPECT_FALSE(VAsioRegistryTestAccess::IsPeerRegistered(_registry, &a));
        VAsioRegistryTestAccess::Shutdown(_registry, &a);

        EXPECT_TRUE(VAsioRegistryTestAccess::IsPeerRegistered(_registry, &aRejoined));
        VAsioRegistryTestAccess::Shutdown(_registry, &aRejoined);
        VAsioRegistryTestAccess::Shutdown(_registry, &b);
    }
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

    FakeVAsioPeer a1{"A", "session/Sim"};
    FakeVAsioPeer a2{"A", "session/Sim"};

    VAsioRegistryTestAccess::Announce(_registry, &a1, "session/Sim");
    VAsioRegistryTestAccess::Announce(_registry, &a2, "session/Sim");

    VAsioRegistryTestAccess::Shutdown(_registry, &a1);
    EXPECT_EQ(calls, 0);
    EXPECT_TRUE(VAsioRegistryTestAccess::IsPeerRegistered(_registry, &a2));

    VAsioRegistryTestAccess::Shutdown(_registry, &a2);
    EXPECT_EQ(calls, 1);
}

class Test_VAsioRegistryLegacy
    : public Test_VAsioRegistrySimulations
    , public testing::WithParamInterface<std::string>
{
};

TEST_P(Test_VAsioRegistryLegacy, duplicate_is_rejected_without_disturbing_the_original_and_can_rejoin_after_shutdown)
{
    using SilKit::Services::Logging::ALoggerMessageWith;
    using SilKit::Services::Logging::Level;

    const auto& simulationName{GetParam()};
    FakeVAsioPeer original{"A", simulationName};
    FakeVAsioPeer duplicate{"A", simulationName};
    auto& logger{VAsioRegistryTestAccess::UseMockLogger(_registry, _originalLogger)};

    VAsioRegistryTestAccess::Announce(_registry, &original, simulationName);
    const auto diagnostic{"A participant with the same name 'A' already exists in the simulation " + simulationName};
    EXPECT_CALL(logger, ProcessLoggerMessage(ALoggerMessageWith(Level::Warn, diagnostic))).Times(1);
    EXPECT_CALL(duplicate, SendSilKitMsg(testing::An<SilKit::Core::SerializedMessage>()))
        .WillOnce([&diagnostic](SilKit::Core::SerializedMessage message) {
            const auto reply{message.Deserialize<SilKit::Core::ParticipantAnnouncementReply>()};
            EXPECT_EQ(reply.status, SilKit::Core::ParticipantAnnouncementReply::Status::Failed);
            EXPECT_EQ(reply.diagnostic, diagnostic);
        });

    VAsioRegistryTestAccess::Announce(_registry, &duplicate, simulationName);
    EXPECT_FALSE(original.GetServiceDescriptor().GetGeneration().has_value());
    EXPECT_FALSE(duplicate.GetServiceDescriptor().GetGeneration().has_value());
    EXPECT_TRUE(VAsioRegistryTestAccess::IsPeerRegistered(_registry, &original));
    EXPECT_FALSE(VAsioRegistryTestAccess::IsPeerRegistered(_registry, &duplicate));
    EXPECT_FALSE(VAsioRegistryTestAccess::HasPlacement(_registry, simulationName));

    EXPECT_CALL(logger, ProcessLoggerMessage(ALoggerMessageWith(
                            Level::Debug, "Duplicate peer 'A' has shut down, which had no participant information")))
        .Times(1);
    VAsioRegistryTestAccess::Shutdown(_registry, &duplicate);
    EXPECT_TRUE(VAsioRegistryTestAccess::IsPeerRegistered(_registry, &original));

    VAsioRegistryTestAccess::Shutdown(_registry, &original);
    EXPECT_FALSE(VAsioRegistryTestAccess::IsPeerRegistered(_registry, &original));
    testing::Mock::VerifyAndClearExpectations(&duplicate);
    EXPECT_CALL(duplicate, SendSilKitMsg(testing::An<SilKit::Core::SerializedMessage>())).Times(2);
    VAsioRegistryTestAccess::Announce(_registry, &duplicate, simulationName);
    EXPECT_TRUE(VAsioRegistryTestAccess::IsPeerRegistered(_registry, &duplicate));
    EXPECT_FALSE(duplicate.GetServiceDescriptor().GetGeneration().has_value());
}

TEST_P(Test_VAsioRegistryLegacy, status_and_workflow_do_not_create_a_placement)
{
    const auto& simulationName{GetParam()};
    FakeVAsioPeer peer{"A", simulationName};
    VAsioRegistryTestAccess::Announce(_registry, &peer, simulationName);
    VAsioRegistryTestAccess::SendWorkflowConfiguration(_registry, &peer, {"A"});
    VAsioRegistryTestAccess::SendStatus(_registry, &peer,
                                      SilKit::Services::Orchestration::ParticipantState::ReadyToRun);
    EXPECT_FALSE(VAsioRegistryTestAccess::HasPlacement(_registry, simulationName));
}

INSTANTIATE_TEST_SUITE_P(DisabledNames, Test_VAsioRegistryLegacy,
                         testing::Values("Sim", "session", "Session/x", "x/session/"));

TEST_F(Test_VAsioRegistrySimulations, prefix_alone_enables_generations)
{
    FakeVAsioPeer a1{"A", "session/"};
    FakeVAsioPeer a2{"A", "session/"};
    VAsioRegistryTestAccess::Announce(_registry, &a1, "session/");
    VAsioRegistryTestAccess::Announce(_registry, &a2, "session/");
    EXPECT_EQ(a1.GetServiceDescriptor().GetGeneration(), 1u);
    EXPECT_EQ(a2.GetServiceDescriptor().GetGeneration(), 2u);
    EXPECT_TRUE(VAsioRegistryTestAccess::IsPeerRegistered(_registry, &a1));
    EXPECT_TRUE(VAsioRegistryTestAccess::IsPeerRegistered(_registry, &a2));
}

TEST_F(Test_VAsioRegistrySimulations, session_and_legacy_simulations_are_independent)
{
    FakeVAsioPeer session1{"A", "session/x"};
    FakeVAsioPeer session2{"A", "session/x"};
    FakeVAsioPeer legacy{"A", "y"};
    VAsioRegistryTestAccess::Announce(_registry, &session1, "session/x");
    VAsioRegistryTestAccess::Announce(_registry, &legacy, "y");
    VAsioRegistryTestAccess::Announce(_registry, &session2, "session/x");
    EXPECT_EQ(session1.GetServiceDescriptor().GetGeneration(), 1u);
    EXPECT_EQ(session2.GetServiceDescriptor().GetGeneration(), 2u);
    EXPECT_FALSE(legacy.GetServiceDescriptor().GetGeneration().has_value());
    EXPECT_FALSE(VAsioRegistryTestAccess::HasPlacement(_registry, "y"));
    EXPECT_TRUE(VAsioRegistryTestAccess::HasPlacement(_registry, "session/x"));

    VAsioRegistryTestAccess::Shutdown(_registry, &legacy);
    EXPECT_TRUE(VAsioRegistryTestAccess::IsPeerRegistered(_registry, &session1));
    EXPECT_TRUE(VAsioRegistryTestAccess::IsPeerRegistered(_registry, &session2));
    VAsioRegistryTestAccess::Announce(_registry, &legacy, "y");
    VAsioRegistryTestAccess::Shutdown(_registry, &session1);
    VAsioRegistryTestAccess::Shutdown(_registry, &session2);
    EXPECT_FALSE(VAsioRegistryTestAccess::HasPlacement(_registry, "session/x"));
    EXPECT_TRUE(VAsioRegistryTestAccess::IsPeerRegistered(_registry, &legacy));
}

TEST_F(Test_VAsioRegistrySimulations, shutdown_without_announcement_logs_at_debug_in_both_modes)
{
    using SilKit::Services::Logging::ALoggerMessageWith;
    using SilKit::Services::Logging::Level;

    auto& logger{VAsioRegistryTestAccess::UseMockLogger(_registry, _originalLogger)};
    EXPECT_CALL(logger, ProcessLoggerMessage(ALoggerMessageWith(
                            Level::Debug, "Peer 'Unknown' has shut down, which had no participant information")))
        .Times(2);
    EXPECT_CALL(logger, ProcessLoggerMessage(ALoggerMessageWith(Level::Error))).Times(0);
    for (const auto& simulationName : {"Sim", "session/x"})
    {
        FakeVAsioPeer peer{"Unknown", simulationName};
        VAsioRegistryTestAccess::Shutdown(_registry, &peer);
        EXPECT_FALSE(VAsioRegistryTestAccess::HasPlacement(_registry, simulationName));
    }
}

} // namespace
