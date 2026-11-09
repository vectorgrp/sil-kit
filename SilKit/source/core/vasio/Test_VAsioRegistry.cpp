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

// the registry grants access to its internals for testing
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

    static auto GenerationOf(const VAsioRegistry& registry, const IVAsioPeer* peer) -> std::optional<uint64_t>
    {
        return registry.FindGenerationOfPeer(peer);
    }

    static auto GenerationOfSender(const VAsioRegistry& registry, const IServiceEndpoint* from)
        -> std::optional<uint64_t>
    {
        return registry.FindGenerationOfSender(from);
    }

    static auto FindPeerInGenerationOf(const VAsioRegistry& registry, const IVAsioPeer* requester,
                                       const std::string& participantName) -> IVAsioPeer*
    {
        return registry.FindPeerInGenerationOf(requester, participantName);
    }
};

} // namespace Core
} // namespace SilKit

namespace {

using SilKit::Core::IVAsioPeer;
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

TEST_F(Test_VAsioRegistryGenerations, peers_with_the_same_name_are_resolved_within_their_own_generation)
{
    TestPeer a1{"A", "Sim"};
    TestPeer b1{"B", "Sim"};
    TestPeer a2{"A", "Sim"};
    TestPeer b2{"B", "Sim"};

    VAsioRegistryTestAccess::Announce(_registry, &a1, "Sim");
    VAsioRegistryTestAccess::Announce(_registry, &b1, "Sim");
    VAsioRegistryTestAccess::Announce(_registry, &a2, "Sim");
    VAsioRegistryTestAccess::Announce(_registry, &b2, "Sim");

    EXPECT_EQ(VAsioRegistryTestAccess::GenerationOf(_registry, &a1), 1u);
    EXPECT_EQ(VAsioRegistryTestAccess::GenerationOf(_registry, &b1), 1u);
    EXPECT_EQ(VAsioRegistryTestAccess::GenerationOf(_registry, &a2), 2u);
    EXPECT_EQ(VAsioRegistryTestAccess::GenerationOf(_registry, &b2), 2u);

    EXPECT_EQ(VAsioRegistryTestAccess::FindPeerInGenerationOf(_registry, &a1, "B"), &b1);
    EXPECT_EQ(VAsioRegistryTestAccess::FindPeerInGenerationOf(_registry, &b1, "A"), &a1);
    EXPECT_EQ(VAsioRegistryTestAccess::FindPeerInGenerationOf(_registry, &a2, "B"), &b2);
    EXPECT_EQ(VAsioRegistryTestAccess::FindPeerInGenerationOf(_registry, &b2, "A"), &a2);
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

    EXPECT_EQ(VAsioRegistryTestAccess::GenerationOf(_registry, &a1), std::nullopt);
    EXPECT_EQ(VAsioRegistryTestAccess::FindPeerInGenerationOf(_registry, &b1, "A"), nullptr);

    // the other generation is unaffected
    EXPECT_EQ(VAsioRegistryTestAccess::GenerationOf(_registry, &a2), 2u);
    EXPECT_EQ(VAsioRegistryTestAccess::FindPeerInGenerationOf(_registry, &b2, "A"), &a2);
    EXPECT_EQ(VAsioRegistryTestAccess::FindPeerInGenerationOf(_registry, &a2, "B"), &b2);
}

TEST_F(Test_VAsioRegistryGenerations, unknown_requester_resolves_to_no_peer)
{
    TestPeer a{"A", "Sim"};
    TestPeer unknown{"B", "Sim"};

    VAsioRegistryTestAccess::Announce(_registry, &a, "Sim");

    EXPECT_EQ(VAsioRegistryTestAccess::FindPeerInGenerationOf(_registry, &unknown, "A"), nullptr);
}

TEST_F(Test_VAsioRegistryGenerations, generation_of_the_sender_is_found_via_the_remote_service_endpoint)
{
    TestPeer a1{"A", "Sim"};
    TestPeer a2{"A", "Sim"};

    VAsioRegistryTestAccess::Announce(_registry, &a1, "Sim");
    VAsioRegistryTestAccess::Announce(_registry, &a2, "Sim");

    // both peers carry an identical service descriptor, only the peer tells them apart
    SilKit::Core::RemoteServiceEndpoint fromA1{a1._serviceDescriptor, &a1};
    SilKit::Core::RemoteServiceEndpoint fromA2{a2._serviceDescriptor, &a2};
    EXPECT_EQ(VAsioRegistryTestAccess::GenerationOfSender(_registry, &fromA1), 1u);
    EXPECT_EQ(VAsioRegistryTestAccess::GenerationOfSender(_registry, &fromA2), 2u);

    // an endpoint without a peer, e.g. a local one, has no generation
    SilKit::Core::RemoteServiceEndpoint withoutPeer{a1._serviceDescriptor};
    EXPECT_EQ(VAsioRegistryTestAccess::GenerationOfSender(_registry, &withoutPeer), std::nullopt);
}


} // namespace
