// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "config/ConfigurationTestUtils.hpp"

#include "util/Uri.hpp"

#include "core/vasio/VAsioRegistry.hpp"
#include "core/vasio/io/MakeAsioIoContext.hpp"
#include "core/vasio/mock/MockVAsioPeer.hpp"

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
        registry.OnParticipantAnnouncement(peer, announcement);
    }

    static void Shutdown(VAsioRegistry& registry, IVAsioPeer* peer)
    {
        registry.OnPeerShutdown(peer);
    }

    static bool IsPeerRegistered(const VAsioRegistry& registry, const IVAsioPeer* peer)
    {
        return registry.IsPeerRegistered(peer);
    }
};

} // namespace Core
} // namespace SilKit

namespace {

using SilKit::Core::VAsioRegistry;
using SilKit::Core::VAsioRegistryTestAccess;

struct TestPeer : testing::NiceMock<SilKit::Core::MockVAsioPeer>
{
    SilKit::Core::VAsioPeerInfo _peerInfo;
    SilKit::Core::ServiceDescriptor _serviceDescriptor;
    std::string _simulationName;

    TestPeer(const std::string& participantName, const std::string& simulationName)
        : _simulationName{simulationName}
    {
        _peerInfo.participantName = participantName;
        _peerInfo.participantId = 1234;
        _peerInfo.acceptorUris.push_back("tcp://127.0.0.1:1234");

        _serviceDescriptor.SetParticipantNameAndComputeId(participantName);

        ON_CALL(*this, GetLocalAddress()).WillByDefault(testing::Return("tcp://127.0.0.1:1234"));
        ON_CALL(*this, GetRemoteAddress()).WillByDefault(testing::Return("tcp://127.0.0.1:1234"));
        ON_CALL(*this, GetSimulationName()).WillByDefault(testing::ReturnRef(_simulationName));
        ON_CALL(*this, GetInfo()).WillByDefault(testing::ReturnRef(_peerInfo));
        ON_CALL(*this, GetServiceDescriptor()).WillByDefault(testing::ReturnRef(_serviceDescriptor));
        ON_CALL(*this, SetServiceDescriptor(testing::_))
            .WillByDefault([this](const SilKit::Core::ServiceDescriptor& descriptor) { _serviceDescriptor = descriptor; });
        ON_CALL(*this, GetProtocolVersion()).WillByDefault(testing::Return(SilKit::Core::CurrentProtocolVersion()));
    }
};

class Test_VAsioRegistryGenerations : public testing::Test
{
protected:
    Test_VAsioRegistryGenerations()
        : _registry{SilKit::Config::MakeEmptyParticipantConfigurationImpl()}
    {
    }

    VAsioRegistry _registry;
};

TEST_F(Test_VAsioRegistryGenerations, peers_with_the_same_name_are_placed_into_different_generations)
{
    TestPeer a1{"A", "Sim"};
    TestPeer b1{"B", "Sim"};
    TestPeer a2{"A", "Sim"};
    TestPeer b2{"B", "Sim"};

    VAsioRegistryTestAccess::Announce(_registry, &a1, "Sim");
    VAsioRegistryTestAccess::Announce(_registry, &b1, "Sim");
    VAsioRegistryTestAccess::Announce(_registry, &a2, "Sim");
    VAsioRegistryTestAccess::Announce(_registry, &b2, "Sim");

    EXPECT_EQ(a1.GetServiceDescriptor().GetGeneration(), 1u);
    EXPECT_EQ(b1.GetServiceDescriptor().GetGeneration(), 1u);
    EXPECT_EQ(a2.GetServiceDescriptor().GetGeneration(), 2u);
    EXPECT_EQ(b2.GetServiceDescriptor().GetGeneration(), 2u);

    for (const auto* peer : {&a1, &b1, &a2, &b2})
    {
        EXPECT_TRUE(VAsioRegistryTestAccess::IsPeerRegistered(_registry, peer));
    }
}

TEST_F(Test_VAsioRegistryGenerations, generation_gets_reset_when_non_exists)
{
    TestPeer a{"A", "Sim"};

    VAsioRegistryTestAccess::Announce(_registry, &a, "Sim");
    EXPECT_EQ(a.GetServiceDescriptor().GetGeneration(), 1u);
    EXPECT_TRUE(VAsioRegistryTestAccess::IsPeerRegistered(_registry, &a));

    VAsioRegistryTestAccess::Shutdown(_registry, &a);
    EXPECT_FALSE(VAsioRegistryTestAccess::IsPeerRegistered(_registry, &a));

    VAsioRegistryTestAccess::Announce(_registry, &a, "Sim");
    EXPECT_EQ(a.GetServiceDescriptor().GetGeneration(), 1u);
    EXPECT_TRUE(VAsioRegistryTestAccess::IsPeerRegistered(_registry, &a));
}

TEST_F(Test_VAsioRegistryGenerations, DISABLED_rejoining_bumps_the_generation_while_previous_generation_exists)
{
    TestPeer a{"A", "Sim"};
    TestPeer b{"B", "Sim"};

    VAsioRegistryTestAccess::Announce(_registry, &a, "Sim");
    VAsioRegistryTestAccess::Announce(_registry, &b, "Sim");
    EXPECT_EQ(a.GetServiceDescriptor().GetGeneration(), 1u);
    EXPECT_EQ(b.GetServiceDescriptor().GetGeneration(), 1u);

    VAsioRegistryTestAccess::Shutdown(_registry, &a);
    EXPECT_FALSE(VAsioRegistryTestAccess::IsPeerRegistered(_registry, &a));

    VAsioRegistryTestAccess::Announce(_registry, &a, "Sim");
    EXPECT_EQ(a.GetServiceDescriptor().GetGeneration(), 2u);
}

TEST_F(Test_VAsioRegistryGenerations, rejoining_bumps_the_generation_while_participant_with_same_name_exists)
{
    TestPeer a1{"A", "Sim"};
    TestPeer a2{"A", "Sim"};

    VAsioRegistryTestAccess::Announce(_registry, &a1, "Sim");
    EXPECT_EQ(a1.GetServiceDescriptor().GetGeneration(), 1u);

    VAsioRegistryTestAccess::Announce(_registry, &a2, "Sim");
    EXPECT_EQ(a2.GetServiceDescriptor().GetGeneration(), 2u);

    VAsioRegistryTestAccess::Shutdown(_registry, &a1);
    EXPECT_FALSE(VAsioRegistryTestAccess::IsPeerRegistered(_registry, &a1));

    VAsioRegistryTestAccess::Announce(_registry, &a1, "Sim");
    EXPECT_EQ(a1.GetServiceDescriptor().GetGeneration(), 3u);
}

TEST_F(Test_VAsioRegistryGenerations, participant_with_same_name_but_different_simulation_name_gets_its_own_generation)
{
    TestPeer a1{"A", "Sim1"};
    TestPeer a2{"A", "Sim2"};

    VAsioRegistryTestAccess::Announce(_registry, &a1, "Sim1");
    EXPECT_EQ(a1.GetServiceDescriptor().GetGeneration(), 1u);

    VAsioRegistryTestAccess::Announce(_registry, &a2, "Sim2");
    EXPECT_EQ(a2.GetServiceDescriptor().GetGeneration(), 1u);

    VAsioRegistryTestAccess::Shutdown(_registry, &a1);
    EXPECT_FALSE(VAsioRegistryTestAccess::IsPeerRegistered(_registry, &a1));

    VAsioRegistryTestAccess::Announce(_registry, &a1, "Sim1");
    EXPECT_EQ(a1.GetServiceDescriptor().GetGeneration(), 1u);
}

TEST_F(Test_VAsioRegistryGenerations, shutdown_of_a_peer_does_not_affect_a_peer_with_the_same_name)
{
    TestPeer a1{"A", "Sim"};
    TestPeer b1{"B", "Sim"};
    TestPeer a2{"A", "Sim"};
    TestPeer b2{"B", "Sim"};

    VAsioRegistryTestAccess::Announce(_registry, &a1, "Sim");
    VAsioRegistryTestAccess::Announce(_registry, &b1, "Sim");
    VAsioRegistryTestAccess::Announce(_registry, &a2, "Sim");
    VAsioRegistryTestAccess::Announce(_registry, &b2, "Sim");

    VAsioRegistryTestAccess::Shutdown(_registry, &a1);

    EXPECT_FALSE(VAsioRegistryTestAccess::IsPeerRegistered(_registry, &a1));

    // the other peers, in particular the one with the same name, are unaffected
    EXPECT_TRUE(VAsioRegistryTestAccess::IsPeerRegistered(_registry, &b1));
    EXPECT_TRUE(VAsioRegistryTestAccess::IsPeerRegistered(_registry, &a2));
    EXPECT_TRUE(VAsioRegistryTestAccess::IsPeerRegistered(_registry, &b2));
}

TEST_F(Test_VAsioRegistryGenerations, repeated_shutdown_does_not_remove_a_peer_that_took_the_name_over)
{
    TestPeer a{"A", "Sim"};
    TestPeer b{"B", "Sim"};
    TestPeer aRejoined{"A", "Sim"};

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

TEST_F(Test_VAsioRegistryGenerations, unknown_peer_is_not_registered)
{
    TestPeer a{"A", "Sim"};
    TestPeer unknown{"B", "Sim"};

    VAsioRegistryTestAccess::Announce(_registry, &a, "Sim");

    EXPECT_FALSE(VAsioRegistryTestAccess::IsPeerRegistered(_registry, &unknown));
}

TEST_F(Test_VAsioRegistryGenerations, generation_is_visible_to_receivers_via_the_descriptor_of_the_sender)
{
    TestPeer a1{"A", "Sim"};
    TestPeer a2{"A", "Sim"};
    TestPeer unannounced{"A", "Sim"};

    VAsioRegistryTestAccess::Announce(_registry, &a1, "Sim");
    VAsioRegistryTestAccess::Announce(_registry, &a2, "Sim");

    // The connection builds the endpoint passed to ReceiveMsg from a copy of the descriptor of the sending peer.
    // Name and participant id are identical for both peers, only the generation tells them apart.
    const SilKit::Core::ServiceDescriptor descriptorOfA1{a1.GetServiceDescriptor()};
    const SilKit::Core::ServiceDescriptor descriptorOfA2{a2.GetServiceDescriptor()};
    const SilKit::Core::RemoteServiceEndpoint fromA1{descriptorOfA1};
    const SilKit::Core::RemoteServiceEndpoint fromA2{descriptorOfA2};

    EXPECT_EQ(fromA1.GetServiceDescriptor().GetParticipantId(), fromA2.GetServiceDescriptor().GetParticipantId());
    EXPECT_EQ(fromA1.GetServiceDescriptor().GetGeneration(), 1u);
    EXPECT_EQ(fromA2.GetServiceDescriptor().GetGeneration(), 2u);

    // a peer the registry has not seen has no generation
    EXPECT_EQ(unannounced.GetServiceDescriptor().GetGeneration(), std::nullopt);
}

} // namespace
