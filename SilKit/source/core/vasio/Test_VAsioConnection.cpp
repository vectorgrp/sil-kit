// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

// fowards required for TestDataTypes because of  SilKitMsgTraits
#include "silkit/services/can/fwd_decl.hpp"
#include "silkit/services/ethernet/fwd_decl.hpp"
#include "silkit/services/flexray/fwd_decl.hpp"
#include "silkit/services/lin/fwd_decl.hpp"
#include "silkit/services/pubsub/fwd_decl.hpp"
#include "silkit/services/rpc/fwd_decl.hpp"
#include "silkit/services/orchestration/fwd_decl.hpp"
#include "silkit/services/logging/fwd_decl.hpp"

// internal types required for TestDataTypes because of SilKitMsgTraits
#include "wire/can/WireCanMessages.hpp"
#include "wire/pubsub/WireDataMessages.hpp"
#include "wire/ethernet/WireEthernetMessages.hpp"
#include "wire/flexray/WireFlexrayMessages.hpp"
#include "wire/lin/WireLinMessages.hpp"
#include "wire/rpc/WireRpcMessages.hpp"

#include "core/service/ServiceDatatypes.hpp"         //concrete, no forwards
#include "core/requests/RequestReplyDatatypes.hpp"    //concrete, no forwards
#include "core/internal/LoggingDatatypesInternal.hpp" //concrete, no forwards
#include "core/internal/OrchestrationDatatypes.hpp"   //concrete, no forwards

#include "core/internal/ProtocolVersion.hpp"
#include "core/internal/TestDataTypes.hpp" // must be included before VAsioConnection

#include "core/vasio/IVAsioPeer.hpp"
#include "core/internal/IMessageReceiver.hpp"

#include "core/vasio/VAsioConnection.hpp"
#include "core/vasio/VAsioConstants.hpp"
#include "core/mock/participant/MockParticipant.hpp" // for DummyLogger
#include "core/vasio/VAsioSerdes.hpp"
#include "core/vasio/SerializedMessage.hpp"
#include "util/Uri.hpp"
#include "services/orchestration/TimeProvider.hpp"

#include "services/logging/LoggerMessage.hpp"

#include <chrono>
#include <filesystem>

#include "core/vasio/mock/MockVAsioPeer.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

using namespace std::chrono_literals;
using namespace SilKit::Core;

using testing::Return;
using testing::ReturnRef;
using testing::_;

namespace {
struct MockSilKitMessageReceiver
    : public IMessageReceiver<Tests::Version1::TestMessage>
    , public IMessageReceiver<Tests::Version2::TestMessage>
    , public IMessageReceiver<Tests::TestFrameEvent>
    , public IServiceEndpoint
{
    ServiceDescriptor _serviceDescriptor;

    MockSilKitMessageReceiver()
    {
        _serviceDescriptor.SetServiceId(1);
        _serviceDescriptor.SetParticipantNameAndComputeId("MockSilkitMessageReceiver");

        ON_CALL(*this, GetServiceDescriptor()).WillByDefault(ReturnRef(_serviceDescriptor));
    }
    // IMessageReceiver<T>
    MOCK_METHOD(void, ReceiveMsg, (const SilKit::Core::IServiceEndpoint*, const Tests::Version1::TestMessage&),
                (override));
    MOCK_METHOD(void, ReceiveMsg, (const SilKit::Core::IServiceEndpoint*, const Tests::Version2::TestMessage&),
                (override));
    MOCK_METHOD(void, ReceiveMsg, (const SilKit::Core::IServiceEndpoint*, const Tests::TestFrameEvent&), (override));

    // IServiceEndpoint
    MOCK_METHOD(void, SetServiceDescriptor, (const ServiceDescriptor& serviceDescriptor), (override));
    MOCK_METHOD(const ServiceDescriptor&, GetServiceDescriptor, (), (override, const));
};


// adds some more behavior to MockVasioPeer
struct MockVAsioPeer2 : public MockVAsioPeer
{
    VAsioPeerInfo _peerInfo;
    ServiceDescriptor _serviceDescriptor;
    ProtocolVersion _protocolVersion;
    std::string _simulationName;

    MockVAsioPeer2()
    {
        _peerInfo.participantId = 1234;
        _peerInfo.participantName = "MockVAsioPeer";
        _peerInfo.acceptorUris.push_back("tcp://localhost:1234");

        _serviceDescriptor.SetServiceId(1);
        _serviceDescriptor.SetParticipantNameAndComputeId(_peerInfo.participantName);

        _protocolVersion = CurrentProtocolVersion();

        ON_CALL(*this, GetLocalAddress()).WillByDefault(Return("127.0.0.1"));
        ON_CALL(*this, GetRemoteAddress()).WillByDefault(Return("127.0.0.1"));
        ON_CALL(*this, GetSimulationName()).WillByDefault(ReturnRef(_simulationName));
        ON_CALL(*this, GetInfo()).WillByDefault(ReturnRef(_peerInfo));
        ON_CALL(*this, GetServiceDescriptor()).WillByDefault(ReturnRef(_serviceDescriptor));
        ON_CALL(*this, GetProtocolVersion()).WillByDefault(Return(_protocolVersion));
    }
};

//////////////////////////////////////////////////////////////////////
// Matchers
//////////////////////////////////////////////////////////////////////
MATCHER_P(AnnouncementReplyMatcher, validator,
          "Deserialize the MessageBuffer from the SerializedMessage and check the announcement's reply")
{
    SerializedMessage message = arg;
    auto reply = message.Deserialize<ParticipantAnnouncementReply>();
    return validator(reply);
}

MATCHER_P(SubscriptionAcknowledgeMatcher, subscriber,
          "Deserialize the MessageBuffer from the SerializedMessage and check the subscriptions's ack")
{
    SerializedMessage message = arg;
    auto reply = message.Deserialize<SubscriptionAcknowledge>();
    return reply.status == SubscriptionAcknowledge::Status::Success && reply.subscriber == subscriber;
}

MATCHER_P(ConnectRequestMatcher, status, "Deserialize the RemoteParticipantConnectRequest and check its status")
{
    SerializedMessage message = arg;
    return message.Deserialize<RemoteParticipantConnectRequest>().status == status;
}

MATCHER_P(ProxyMessageMatcher, isShutdownNotification,
          "Deserialize the ProxyMessage and check whether its payload is empty (shutdown notification)")
{
    SerializedMessage message = arg;
    return message.Deserialize<ProxyMessage>().payload.empty() == isShutdownNotification;
}

} // namespace

//////////////////////////////////////////////////////////////////////
// Test Fixture
//////////////////////////////////////////////////////////////////////

namespace SilKit {
namespace Core {

class Test_VAsioConnection : public testing::Test
{
protected:
    Test_VAsioConnection()
        : _connection(nullptr, &_dummyMetricsManager, {}, "Test_VAsioConnection", 1, &_timeProvider)
    {
        _connection.SetLoggerInternal(&_dummyLogger);
    }

    Tests::MockLogger _dummyLogger;
    Tests::DummyMetricsManager _dummyMetricsManager;
    Services::Orchestration::TimeProvider _timeProvider;
    VAsioConnection _connection;
    MockVAsioPeer2 _from;

    //we are a friend class
    // - allow selected access to private member
    template <typename MessageT, typename ServiceT>
    void RegisterSilKitMsgReceiver(SilKit::Core::IMessageReceiver<MessageT>* receiver)
    {
        _connection.RegisterSilKitMsgReceiver<MessageT, ServiceT>(receiver);
    }

    static auto ReceiveBlobPoolOf(const VAsioConnection& connection) -> const ReceiveBlobPool&
    {
        return connection._receiveBlobPool;
    }

    static auto PeerInfoOf(VAsioConnection& connection) -> VAsioPeerInfo
    {
        return connection.MakePeerInfo();
    }

    static void AssociateParticipantNameAndPeer(VAsioConnection& connection, const std::string& simulationName,
                                                const std::string& participantName, IVAsioPeer* peer)
    {
        connection.AssociateParticipantNameAndPeer(simulationName, participantName, peer);
    }

    // the generation of all peers that are not peers of a registry
    static constexpr uint64_t NoGeneration{0};

    static auto FindPeer(const VAsioConnection& connection, const std::string& simulationName, uint64_t generation,
                         const std::string& participantName) -> IVAsioPeer*
    {
        return connection.FindPeer(simulationName, generation, participantName);
    }

    static auto FindPeerForRequester(const VAsioConnection& connection, const IVAsioPeer* requester,
                                     const std::string& participantName) -> IVAsioPeer*
    {
        return connection.FindPeerForRequester(requester, participantName);
    }

    static void RemovePeerFromConnection(VAsioConnection& connection, IVAsioPeer* peer)
    {
        connection.RemovePeerFromConnection(peer);
    }

    auto MakeRegistryConnection() -> std::unique_ptr<VAsioConnection>
    {
        SilKit::Config::ParticipantConfiguration config;
        config.middleware.registryAsFallbackProxy = true;
        config.middleware.experimentalRemoteParticipantConnection = true;

        auto connection = std::make_unique<VAsioConnection>(nullptr, &_dummyMetricsManager, config, "Registry",
                                                            REGISTRY_PARTICIPANT_ID, &_timeProvider);
        connection->SetLoggerInternal(&_dummyLogger);
        return connection;
    }
};

} // namespace Core
} // namespace SilKit

//////////////////////////////////////////////////////////////////////
// Versioned initial handshake
//////////////////////////////////////////////////////////////////////

TEST_F(Test_VAsioConnection, unsupported_version_connect)
{
    ParticipantAnnouncement announcement{};
    announcement.peerInfo = _from.GetInfo();
    announcement.messageHeader.versionHigh = 1;

    SerializedMessage message(announcement);

    auto validator = [](const ParticipantAnnouncementReply& reply) {
        return reply.status == ParticipantAnnouncementReply::Status::Failed;
    };
    EXPECT_CALL(_from, SendSilKitMsg(AnnouncementReplyMatcher(validator))).Times(1);
    _connection.OnSocketData(&_from, message);
}

TEST_F(Test_VAsioConnection, unsupported_version_reply_from_registry_should_throw)
{
    ParticipantAnnouncementReply reply{};
    reply.remoteHeader.versionHigh = 1;
    reply.remoteHeader.versionLow = 1;
    reply.status = ParticipantAnnouncementReply::Status::Failed;

    // a failed connection to a registry is fatal
    _from._peerInfo.participantId = REGISTRY_PARTICIPANT_ID;

    SerializedMessage message(reply);
    EXPECT_THROW(_connection.OnSocketData(&_from, message), SilKit::ProtocolError);
}

TEST_F(Test_VAsioConnection, supported_version_reply_from_registry_must_not_throw)
{
    ParticipantAnnouncementReply reply{};
    reply.status = ParticipantAnnouncementReply::Status::Success;

    _from._peerInfo.participantId = REGISTRY_PARTICIPANT_ID;

    SerializedMessage message(reply);
    EXPECT_NO_THROW(_connection.OnSocketData(&_from, message));
}

TEST_F(Test_VAsioConnection, current_version_connect)
{
    ParticipantAnnouncement announcement{}; //sets correct version in header
    announcement.peerInfo = _from.GetInfo();

    SerializedMessage message(announcement);

    auto validator = [](const ParticipantAnnouncementReply& reply) {
        return reply.status == ParticipantAnnouncementReply::Status::Success;
    };
    EXPECT_CALL(_from, SendSilKitMsg(AnnouncementReplyMatcher(validator))).Times(1);

    _connection.OnSocketData(&_from, message);
}

//////////////////////////////////////////////////////////////////////
// Versioned subscriptions: test backward compatibility
//////////////////////////////////////////////////////////////////////

// Disabled because we do not have a versioned Ser/Des that detects
// the different version used for transmission, this is a work in progress.
TEST_F(Test_VAsioConnection, DISABLED_versioned_send_testmessage)
{
    // We send a 'version1', but expect to receive a 'version2'
    Tests::Version1::TestMessage message;
    message.integer = 1234;
    message.str = "1234";

    // Setup subscriptions for transmisison
    using MessageTrait = SilKit::Core::SilKitMsgTraits<decltype(message)>;
    VAsioMsgSubscriber subscriber;
    subscriber.msgTypeName = MessageTrait::SerdesName();
    subscriber.networkName = "unittest";
    subscriber.version = MessageTrait::Version();
    subscriber.receiverIdx = 0; //the first receiver

    EXPECT_TRUE(subscriber.version == 1);

    // ReceiveSubscriptionAnnouncement -> sets internal structures up
    auto subscriberBuffer = SerializedMessage(subscriber);
    EXPECT_CALL(_from, SendSilKitMsg(SubscriptionAcknowledgeMatcher(subscriber))).Times(1);
    _connection.OnSocketData(&_from, subscriberBuffer);

    // Create a receiver with index 0 and a different TestMessage _version_
    MockSilKitMessageReceiver mockReceiver;
    RegisterSilKitMsgReceiver<Tests::Version2::TestMessage, MockSilKitMessageReceiver>(&mockReceiver);

    // the actual message
    auto buffer = SerializedMessage(message, _from.GetServiceDescriptor().to_endpointAddress(), subscriber.receiverIdx);

    _connection.OnSocketData(&_from, buffer);
}

//////////////////////////////////////////////////////////////////////
// Receive buffer pool configuration
//////////////////////////////////////////////////////////////////////

TEST_F(Test_VAsioConnection, receive_buffer_pool_is_enabled_by_default)
{
    EXPECT_TRUE(ReceiveBlobPoolOf(_connection).IsEnabled());
}

TEST_F(Test_VAsioConnection, receive_buffer_pool_follows_the_configuration)
{
    SilKit::Config::ParticipantConfiguration config;
    config.experimental.useReceiveBufferPool = false;

    VAsioConnection connection{nullptr, &_dummyMetricsManager, config, "Test_VAsioConnection", 1, &_timeProvider};

    EXPECT_FALSE(ReceiveBlobPoolOf(connection).IsEnabled());
}

TEST_F(Test_VAsioConnection, generated_local_socket_collision_preserves_the_existing_listener)
{
    const auto uris = _connection.PrepareAcceptorEndpointUris("silkit://localhost:8500");
    _connection.OpenLocalAcceptors(uris);
    const auto firstInfo = PeerInfoOf(_connection);
    ASSERT_EQ(firstInfo.acceptorUris.size(), 1u);
    const auto firstPath = Uri::Parse(firstInfo.acceptorUris.front()).Path();

    {
        VAsioConnection other{nullptr, &_dummyMetricsManager, {}, "Test_VAsioConnection", 1, &_timeProvider};
        other.SetLoggerInternal(&_dummyLogger);
        // Reuse the already-bound URI to force a collision and retry with a fresh generated path.
        other.OpenLocalAcceptors(uris);

        const auto secondInfo = PeerInfoOf(other);
        ASSERT_EQ(secondInfo.acceptorUris.size(), 1u);
        EXPECT_NE(firstInfo.acceptorUris.front(), secondInfo.acceptorUris.front());
        EXPECT_TRUE(std::filesystem::exists(firstPath));
        EXPECT_TRUE(std::filesystem::exists(Uri::Parse(secondInfo.acceptorUris.front()).Path()));
    }

    // Destroying the second listener must not remove the first listener's socket path.
    EXPECT_TRUE(std::filesystem::exists(firstPath));
}

TEST_F(Test_VAsioConnection, generated_local_socket_does_not_retry_unrelated_bind_errors)
{
    const auto uris = _connection.PrepareAcceptorEndpointUris("silkit://localhost:8500");
    const auto missingParent = Uri::Parse(uris.back()).Path();
    ASSERT_FALSE(std::filesystem::exists(missingParent));

    // Use the uncreated socket path as a parent directory to cause "no such file or directory", not a collision.
    _connection.OpenLocalAcceptors({"local://" + missingParent + "/socket"});

    // Regenerating a valid temporary path for this unrelated error would incorrectly create a listener.
    EXPECT_TRUE(PeerInfoOf(_connection).acceptorUris.empty());
}

//////////////////////////////////////////////////////////////////////
// Peers with identical names (registry with several generations)
//////////////////////////////////////////////////////////////////////

TEST_F(Test_VAsioConnection, removing_a_peer_keeps_the_entry_of_another_peer_with_the_same_name)
{
    MockVAsioPeer2 first;
    MockVAsioPeer2 second;
    first._simulationName = "Sim";
    second._simulationName = "Sim";

    AssociateParticipantNameAndPeer(_connection, "Sim", "MockVAsioPeer", &first);
    AssociateParticipantNameAndPeer(_connection, "Sim", "MockVAsioPeer", &second);
    ASSERT_EQ(FindPeer(_connection, "Sim", NoGeneration, "MockVAsioPeer"), &first);

    RemovePeerFromConnection(_connection, &second);
    EXPECT_EQ(FindPeer(_connection, "Sim", NoGeneration, "MockVAsioPeer"), &first);

    RemovePeerFromConnection(_connection, &first);
    EXPECT_EQ(FindPeer(_connection, "Sim", NoGeneration, "MockVAsioPeer"), nullptr);
}

TEST_F(Test_VAsioConnection, peers_with_the_same_name_in_different_generations_are_registered_separately)
{
    MockVAsioPeer2 requesterOfFirst;
    MockVAsioPeer2 requesterOfSecond;
    MockVAsioPeer2 first;
    MockVAsioPeer2 second;
    requesterOfFirst._serviceDescriptor.SetGeneration(1);
    first._serviceDescriptor.SetGeneration(1);
    requesterOfSecond._serviceDescriptor.SetGeneration(2);
    second._serviceDescriptor.SetGeneration(2);

    AssociateParticipantNameAndPeer(_connection, "", "MockVAsioPeer", &first);
    AssociateParticipantNameAndPeer(_connection, "", "MockVAsioPeer", &second);

    EXPECT_EQ(FindPeerForRequester(_connection, &requesterOfFirst, "MockVAsioPeer"), &first);
    EXPECT_EQ(FindPeerForRequester(_connection, &requesterOfSecond, "MockVAsioPeer"), &second);

    // peers without a generation, i.e. everything but the peers of a registry, are not found by generation
    EXPECT_EQ(FindPeer(_connection, "", NoGeneration, "MockVAsioPeer"), nullptr);

    RemovePeerFromConnection(_connection, &first);
    EXPECT_EQ(FindPeerForRequester(_connection, &requesterOfFirst, "MockVAsioPeer"), nullptr);
    EXPECT_EQ(FindPeerForRequester(_connection, &requesterOfSecond, "MockVAsioPeer"), &second);
}

TEST_F(Test_VAsioConnection, connected_participant_names_contain_each_name_once)
{
    MockVAsioPeer2 firstGeneration;
    MockVAsioPeer2 secondGeneration;
    MockVAsioPeer2 other;
    other._peerInfo.participantName = "Other";
    firstGeneration._serviceDescriptor.SetGeneration(1);
    secondGeneration._serviceDescriptor.SetGeneration(2);

    AssociateParticipantNameAndPeer(_connection, "Sim", "MockVAsioPeer", &firstGeneration);
    AssociateParticipantNameAndPeer(_connection, "Sim", "MockVAsioPeer", &secondGeneration);
    AssociateParticipantNameAndPeer(_connection, "Sim", "Other", &other);

    EXPECT_THAT(_connection.GetConnectedParticipantsNames(), testing::ElementsAre("MockVAsioPeer", "Other"));
}

TEST_F(Test_VAsioConnection, participants_are_listed_by_simulation_and_generation)
{
    MockVAsioPeer2 first;
    MockVAsioPeer2 second;
    MockVAsioPeer2 other;
    MockVAsioPeer2 unstamped;
    MockVAsioPeer2 otherSimulation;
    for (auto* peer : {&first, &second, &other, &unstamped})
    {
        peer->_simulationName = "Sim";
    }
    otherSimulation._simulationName = "Else";
    other._peerInfo.participantName = "Other";
    unstamped._peerInfo.participantName = "Unstamped";
    first._serviceDescriptor.SetGeneration(1);
    second._serviceDescriptor.SetGeneration(2);
    other._serviceDescriptor.SetGeneration(2);

    AssociateParticipantNameAndPeer(_connection, "Sim", "MockVAsioPeer", &first);
    AssociateParticipantNameAndPeer(_connection, "Sim", "MockVAsioPeer", &second);
    AssociateParticipantNameAndPeer(_connection, "Sim", "Other", &other);
    AssociateParticipantNameAndPeer(_connection, "Sim", "Unstamped", &unstamped);
    AssociateParticipantNameAndPeer(_connection, "Else", "MockVAsioPeer", &otherSimulation);

    const auto registered{_connection.ParticipantsByGeneration("Sim")};
    EXPECT_THAT(registered, testing::ElementsAre(testing::Pair(NoGeneration, std::set<std::string>{"Unstamped"}),
                                                 testing::Pair(1, std::set<std::string>{"MockVAsioPeer"}),
                                                 testing::Pair(2, std::set<std::string>{"MockVAsioPeer", "Other"})));
    EXPECT_TRUE(_connection.ParticipantsByGeneration("Unknown").empty());

    RemovePeerFromConnection(_connection, &first);
    EXPECT_EQ(_connection.ParticipantsByGeneration("Sim").count(1), 0u);
}

TEST_F(Test_VAsioConnection, peers_in_generation_of_a_peer_are_the_other_peers_of_its_simulation_and_generation)
{
    MockVAsioPeer2 a1;
    MockVAsioPeer2 b1;
    MockVAsioPeer2 a2;
    MockVAsioPeer2 otherSimulation;
    for (auto* peer : {&a1, &b1, &a2, &otherSimulation})
    {
        peer->_simulationName = "Sim";
    }
    otherSimulation._simulationName = "Else";
    b1._peerInfo.participantName = "B";
    a1._serviceDescriptor.SetGeneration(1);
    b1._serviceDescriptor.SetGeneration(1);
    a2._serviceDescriptor.SetGeneration(2);
    otherSimulation._serviceDescriptor.SetGeneration(1);

    AssociateParticipantNameAndPeer(_connection, "Sim", "MockVAsioPeer", &a1);
    AssociateParticipantNameAndPeer(_connection, "Sim", "B", &b1);
    AssociateParticipantNameAndPeer(_connection, "Sim", "MockVAsioPeer", &a2);
    AssociateParticipantNameAndPeer(_connection, "Else", "MockVAsioPeer", &otherSimulation);

    EXPECT_THAT(_connection.PeersInGenerationOf(&a1), testing::ElementsAre(&b1));
    EXPECT_THAT(_connection.PeersInGenerationOf(&b1), testing::ElementsAre(&a1));
    EXPECT_THAT(_connection.PeersInGenerationOf(&a2), testing::IsEmpty());

    // a peer that is stamped but not registered yet gets the registered peers of its generation
    MockVAsioPeer2 joining;
    joining._simulationName = "Sim";
    joining._peerInfo.participantName = "C";
    joining._serviceDescriptor.SetGeneration(1);
    EXPECT_THAT(_connection.PeersInGenerationOf(&joining), testing::UnorderedElementsAre(&a1, &b1));
}

TEST_F(Test_VAsioConnection, only_the_registered_peer_is_registered_and_not_one_with_the_same_name)
{
    MockVAsioPeer2 registered;
    MockVAsioPeer2 stale;
    registered._simulationName = "Sim";
    stale._simulationName = "Sim";

    EXPECT_FALSE(_connection.GetPeerRegistration(&registered).registered);
    EXPECT_FALSE(_connection.GetPeerRegistration(&stale).nameTakenByOther);

    AssociateParticipantNameAndPeer(_connection, "Sim", "MockVAsioPeer", &registered);
    EXPECT_TRUE(_connection.GetPeerRegistration(&registered).registered);
    EXPECT_FALSE(_connection.GetPeerRegistration(&registered).nameTakenByOther);
    EXPECT_FALSE(_connection.GetPeerRegistration(&stale).registered);
    EXPECT_TRUE(_connection.GetPeerRegistration(&stale).nameTakenByOther);

    RemovePeerFromConnection(_connection, &registered);
    EXPECT_FALSE(_connection.GetPeerRegistration(&registered).registered);
    EXPECT_FALSE(_connection.GetPeerRegistration(&stale).nameTakenByOther);
}

TEST_F(Test_VAsioConnection, registration_counts_the_other_peers_by_generation_simulation_and_in_total)
{
    MockVAsioPeer2 a1;
    MockVAsioPeer2 b1;
    MockVAsioPeer2 a2;
    MockVAsioPeer2 otherSimulation;
    for (auto* peer : {&a1, &b1, &a2})
    {
        peer->_simulationName = "Sim";
    }
    otherSimulation._simulationName = "Else";
    b1._peerInfo.participantName = "B";
    a1._serviceDescriptor.SetGeneration(1);
    b1._serviceDescriptor.SetGeneration(1);
    a2._serviceDescriptor.SetGeneration(2);
    otherSimulation._serviceDescriptor.SetGeneration(1);

    AssociateParticipantNameAndPeer(_connection, "Sim", "MockVAsioPeer", &a1);
    AssociateParticipantNameAndPeer(_connection, "Sim", "B", &b1);
    AssociateParticipantNameAndPeer(_connection, "Sim", "MockVAsioPeer", &a2);
    AssociateParticipantNameAndPeer(_connection, "Else", "MockVAsioPeer", &otherSimulation);

    const auto ofA1{_connection.GetPeerRegistration(&a1)};
    EXPECT_TRUE(ofA1.registered);
    EXPECT_FALSE(ofA1.nameTakenByOther);
    EXPECT_EQ(ofA1.othersInGeneration, 1u);
    EXPECT_EQ(ofA1.othersInSimulation, 2u);
    EXPECT_EQ(ofA1.othersTotal, 3u);

    const auto ofA2{_connection.GetPeerRegistration(&a2)};
    EXPECT_EQ(ofA2.othersInGeneration, 0u);
    EXPECT_EQ(ofA2.othersInSimulation, 2u);

    const auto ofOtherSimulation{_connection.GetPeerRegistration(&otherSimulation)};
    EXPECT_EQ(ofOtherSimulation.othersInGeneration, 0u);
    EXPECT_EQ(ofOtherSimulation.othersInSimulation, 0u);
    EXPECT_EQ(ofOtherSimulation.othersTotal, 3u);

    // a peer that is not registered counts all registered peers as others
    MockVAsioPeer2 stale;
    stale._simulationName = "Sim";
    stale._serviceDescriptor.SetGeneration(1);
    const auto ofStale{_connection.GetPeerRegistration(&stale)};
    EXPECT_FALSE(ofStale.registered);
    EXPECT_TRUE(ofStale.nameTakenByOther);
    EXPECT_EQ(ofStale.othersInGeneration, 2u);
    EXPECT_EQ(ofStale.othersTotal, 4u);

    stale._serviceDescriptor.SetGeneration(3);
    EXPECT_FALSE(_connection.GetPeerRegistration(&stale).nameTakenByOther);
    stale._simulationName = "Unknown";
    stale._serviceDescriptor.SetGeneration(1);
    EXPECT_FALSE(_connection.GetPeerRegistration(&stale).nameTakenByOther);
}

TEST_F(Test_VAsioConnection, generation_assigned_while_handling_the_announcement_is_used_for_the_name_lookup)
{
    // The registry assigns the generation in a handler of the announcement, which runs before the peer is
    // associated with its name.
    uint64_t generation{0};
    _connection.RegisterMessageReceiver([&generation](IVAsioPeer* peer, ParticipantAnnouncement) {
        dynamic_cast<MockVAsioPeer2*>(peer)->_serviceDescriptor.SetGeneration(generation);
    });

    ParticipantAnnouncement announcement{};
    announcement.peerInfo = _from.GetInfo();
    SerializedMessage message(announcement);

    MockVAsioPeer2 second;
    MockVAsioPeer2 requesterOfFirst;
    MockVAsioPeer2 requesterOfSecond;
    requesterOfFirst._serviceDescriptor.SetGeneration(1);
    requesterOfSecond._serviceDescriptor.SetGeneration(2);

    generation = 1;
    _connection.OnSocketData(&_from, message);
    generation = 2;
    SerializedMessage secondMessage(announcement);
    _connection.OnSocketData(&second, secondMessage);

    EXPECT_EQ(FindPeerForRequester(_connection, &requesterOfFirst, "MockVAsioPeer"), &_from);
    EXPECT_EQ(FindPeerForRequester(_connection, &requesterOfSecond, "MockVAsioPeer"), &second);
}

TEST_F(Test_VAsioConnection, registry_relays_connect_request_within_the_generation_of_the_requester)
{
    auto registryConnection = MakeRegistryConnection();

    MockVAsioPeer2 requester;
    MockVAsioPeer2 target;
    MockVAsioPeer2 targetOfOtherGeneration;
    requester._serviceDescriptor.SetGeneration(1);
    target._peerInfo.participantName = "Target";
    target._serviceDescriptor.SetGeneration(1);
    targetOfOtherGeneration._peerInfo.participantName = "Target";
    targetOfOtherGeneration._serviceDescriptor.SetGeneration(2);

    // the peer of the other generation is registered first, so that it would win if the generation was ignored
    AssociateParticipantNameAndPeer(*registryConnection, "", "Target", &targetOfOtherGeneration);
    AssociateParticipantNameAndPeer(*registryConnection, "", "Target", &target);

    RemoteParticipantConnectRequest request{};
    request.status = RemoteParticipantConnectRequest::CONNECTING;
    request.requestOrigin = target.GetInfo();
    request.requestTarget = requester.GetInfo();
    SerializedMessage message(request);

    EXPECT_CALL(target, SendSilKitMsg(ConnectRequestMatcher(RemoteParticipantConnectRequest::CONNECTING))).Times(1);
    EXPECT_CALL(targetOfOtherGeneration, SendSilKitMsg(_)).Times(0);

    registryConnection->OnSocketData(&requester, message);
}

TEST_F(Test_VAsioConnection, registry_proxy_shutdown_notifies_only_destinations_of_the_shutdown_peer)
{
    auto registryConnection = MakeRegistryConnection();

    // peers with the same names in two generations
    MockVAsioPeer2 sourceA;
    MockVAsioPeer2 sourceB;
    MockVAsioPeer2 destinationOfA;
    MockVAsioPeer2 destinationOfB;
    sourceA._peerInfo.participantName = "Source";
    sourceB._peerInfo.participantName = "Source";
    destinationOfA._peerInfo.participantName = "Destination";
    destinationOfB._peerInfo.participantName = "Destination";
    sourceA._serviceDescriptor.SetGeneration(1);
    destinationOfA._serviceDescriptor.SetGeneration(1);
    sourceB._serviceDescriptor.SetGeneration(2);
    destinationOfB._serviceDescriptor.SetGeneration(2);

    // the peers of the second generation are registered first, so that they would win if the generation was ignored
    AssociateParticipantNameAndPeer(*registryConnection, "", "Source", &sourceB);
    AssociateParticipantNameAndPeer(*registryConnection, "", "Source", &sourceA);
    AssociateParticipantNameAndPeer(*registryConnection, "", "Destination", &destinationOfB);
    AssociateParticipantNameAndPeer(*registryConnection, "", "Destination", &destinationOfA);

    ProxyMessage proxyMessage{};
    proxyMessage.source = "Source";
    proxyMessage.destination = "Destination";
    proxyMessage.payload = {1, 2, 3};

    // only sourceA proxies a message, and it is delivered to the destination of its own generation
    EXPECT_CALL(destinationOfA, SendSilKitMsg(ProxyMessageMatcher(false))).Times(1);
    EXPECT_CALL(destinationOfB, SendSilKitMsg(_)).Times(0);
    SerializedMessage message(proxyMessage);
    registryConnection->OnSocketData(&sourceA, message);

    // sourceB never proxied anything, so its shutdown must not be reported to anyone
    registryConnection->OnPeerShutdown(&sourceB);

    EXPECT_CALL(destinationOfA, SendSilKitMsg(ProxyMessageMatcher(true))).Times(1);
    registryConnection->OnPeerShutdown(&sourceA);
}
