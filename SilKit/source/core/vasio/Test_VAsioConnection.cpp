/* Copyright (c) 2022 Vector Informatik GmbH

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
#include "WireCanMessages.hpp"
#include "WireDataMessages.hpp"
#include "WireEthernetMessages.hpp"
#include "WireFlexrayMessages.hpp"
#include "WireLinMessages.hpp"
#include "WireRpcMessages.hpp"

#include "ServiceDatatypes.hpp" //concrete, no forwards
#include "RequestReplyDatatypes.hpp" //concrete, no forwards
#include "LoggingDatatypesInternal.hpp" //concrete, no forwards
#include "OrchestrationDatatypes.hpp" //concrete, no forwards

#include "ProtocolVersion.hpp"
#include "TestDataTypes.hpp" // must be included before VAsioConnection

#include "IVAsioPeer.hpp"
#include "IMessageReceiver.hpp"

#include "VAsioConnection.hpp"
#include "VAsioConstants.hpp"
#include "MockParticipant.hpp" // for DummyLogger
#include "VAsioSerdes.hpp"
#include "SerializedMessage.hpp"
#include "TimeProvider.hpp"

#include "ILoggerInternal.hpp"

#include <chrono>

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


struct MockVAsioPeer : public IVAsioPeer
{
    VAsioPeerInfo _peerInfo;
    ServiceDescriptor _serviceDescriptor;
    ProtocolVersion _protocolVersion;
    std::string _simulationName;

    MockVAsioPeer()
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

    // IVAsioPeer
    MOCK_METHOD(void, SendSilKitMsg, (SerializedMessage), (override));
    MOCK_METHOD(void, Subscribe, (VAsioMsgSubscriber), (override));
    MOCK_METHOD(const VAsioPeerInfo&, GetInfo, (), (const, override));
    MOCK_METHOD(void, SetInfo, (VAsioPeerInfo), (override));
    MOCK_METHOD(std::string, GetRemoteAddress, (), (const, override));
    MOCK_METHOD(std::string, GetLocalAddress, (), (const, override));
    MOCK_METHOD(void, SetSimulationName, (const std::string&), (override));
    MOCK_METHOD(const std::string&, GetSimulationName, (), (const, override));
    MOCK_METHOD(void, StartAsyncRead, (), (override));
    MOCK_METHOD(void, SetProtocolVersion, (ProtocolVersion), (override));
    MOCK_METHOD(ProtocolVersion, GetProtocolVersion, (), (const, override));
    MOCK_METHOD(void, Shutdown, (), (override));
    MOCK_METHOD(void, EnableAggregation, (), (override));

    // IServiceEndpoint (via IVAsioPeer)
    MOCK_METHOD(void, SetServiceDescriptor, (const ServiceDescriptor& serviceDescriptor), (override));
    MOCK_METHOD(const ServiceDescriptor&, GetServiceDescriptor, (), (override, const));
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
        _connection.SetLogger(&_dummyLogger);
    }

    Tests::MockLogger _dummyLogger;
    Tests::DummyMetricsManager _dummyMetricsManager;
    Services::Orchestration::TimeProvider _timeProvider;
    VAsioConnection _connection;
    MockVAsioPeer _from;

    //we are a friend class
    // - allow selected access to private member
    template <typename MessageT, typename ServiceT>
    void RegisterSilKitMsgReceiver(SilKit::Core::IMessageReceiver<MessageT>* receiver)
    {
        _connection.RegisterSilKitMsgReceiver<MessageT, ServiceT>(receiver);
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
    _connection.OnSocketData(&_from, std::move(message));
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
    EXPECT_THROW(_connection.OnSocketData(&_from, std::move(message)), SilKit::ProtocolError);
}

TEST_F(Test_VAsioConnection, supported_version_reply_from_registry_must_not_throw)
{
    ParticipantAnnouncementReply reply{};
    reply.status = ParticipantAnnouncementReply::Status::Success;

    _from._peerInfo.participantId = REGISTRY_PARTICIPANT_ID;

    SerializedMessage message(reply);
    EXPECT_NO_THROW(_connection.OnSocketData(&_from, std::move(message)));
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

    _connection.OnSocketData(&_from, std::move(message));
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
    _connection.OnSocketData(&_from, std::move(subscriberBuffer));

    // Create a receiver with index 0 and a different TestMessage _version_
    MockSilKitMessageReceiver mockReceiver;
    RegisterSilKitMsgReceiver<Tests::Version2::TestMessage, MockSilKitMessageReceiver>(&mockReceiver);

    // the actual message
    auto buffer = SerializedMessage(message, _from.GetServiceDescriptor().to_endpointAddress(), subscriber.receiverIdx);

    _connection.OnSocketData(&_from, std::move(buffer));
}
