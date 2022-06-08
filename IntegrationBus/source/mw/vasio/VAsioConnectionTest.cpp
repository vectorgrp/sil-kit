// Copyright (c) Vector Informatik GmbH. All rights reserved.
//XXX fowards required for TestDataTypes because of  IbMsgTraits
#include "ib/sim/can/fwd_decl.hpp"
#include "ib/sim/eth/fwd_decl.hpp"
#include "ib/sim/fr/fwd_decl.hpp"
#include "ib/sim/lin/fwd_decl.hpp"
#include "ib/sim/data/fwd_decl.hpp"
#include "ib/sim/rpc/fwd_decl.hpp"
#include "ib/mw/sync/fwd_decl.hpp"
#include "ib/mw/logging/fwd_decl.hpp"
#include "ServiceDatatypes.hpp" //concrete, no forwards

#include "ProtocolVersion.hpp"
#include "TestDataTypes.hpp" // must be included before VAsioConnection

#include "IVAsioPeer.hpp"
#include "IIbMessageReceiver.hpp"

#include "VAsioConnection.hpp"
#include "MockParticipant.hpp" // for DummyLogger
#include "VAsioSerdes.hpp"
#include "SerializedMessage.hpp"

#include <chrono>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

using namespace std::chrono_literals;
using namespace ib::mw;

using testing::Return;
using testing::ReturnRef;
using testing::_;
//Printing helpers, required for implicit template usage in VAsioConnection
namespace ib {
namespace mw {
namespace test {
namespace  version1 {
std::ostream& operator<<(std::ostream& out , const TestMessage& msg)
{
    out <<"version1:TestMessage{" << msg.integer <<",\"" << msg.str << "\"}";
    return out;
}
} // version1
inline namespace version2 {
std::ostream& operator<<(std::ostream& out , const TestMessage& msg)
{
    out <<"version2:TestMessage{" << msg.integer <<",\"" << msg.str << "\"}";
    return out;
}
}
std::ostream& operator<<(std::ostream& out , const TestFrameEvent& msg)
{
    out <<"TestFrameEvent{" << msg.integer <<",\"" << msg.str << "\"}";
    return out;
}

} //test
} //mw
} //ib

namespace {
struct MockIbMessageReceiver
    : public IIbMessageReceiver<test::version1::TestMessage>
    , public IIbMessageReceiver<test::version2::TestMessage>
    , public IIbMessageReceiver<test::TestFrameEvent>
    , public IIbServiceEndpoint
{
    ServiceDescriptor _serviceDescriptor;

    MockIbMessageReceiver()
    {
        _serviceDescriptor._serviceId = 1;
        _serviceDescriptor._participantId = 1;

        ON_CALL(*this, GetServiceDescriptor()).WillByDefault(ReturnRef(_serviceDescriptor));
    }
    //IIbMessageReceiver<T>
    MOCK_METHOD(void, ReceiveIbMessage, (const ib::mw::IIbServiceEndpoint*, const test::version1::TestMessage&), (override));
    MOCK_METHOD(void, ReceiveIbMessage, (const ib::mw::IIbServiceEndpoint*, const test::version2::TestMessage&), (override));
    MOCK_METHOD(void, ReceiveIbMessage, (const ib::mw::IIbServiceEndpoint*, const test::TestFrameEvent&), (override));

    //IIbServiceEndpoint
    MOCK_METHOD(void, SetServiceDescriptor, (const ServiceDescriptor& serviceDescriptor), (override));
    MOCK_METHOD(const ServiceDescriptor&, GetServiceDescriptor, (), (override, const));
};


struct MockVAsioPeer
    : public IVAsioPeer
    , public IIbServiceEndpoint
{
    VAsioPeerInfo _peerInfo;
    ServiceDescriptor _serviceDescriptor;
    ProtocolVersion _protocolVersion;

    MockVAsioPeer()
    {
        _peerInfo.participantId = 1234;
        _peerInfo.participantName = "MockVAsioPeer";
        _peerInfo.acceptorUris.push_back("tcp://localhost:1234");

        _serviceDescriptor._serviceId = 1;
        _serviceDescriptor._participantId = 1;

        _serviceDescriptor._serviceId = 1;
        _serviceDescriptor._participantId = 1;
        
        _protocolVersion = CurrentProtocolVersion();

        ON_CALL(*this, GetLocalAddress()).WillByDefault(Return("127.0.0.1"));
        ON_CALL(*this, GetRemoteAddress()).WillByDefault(Return("127.0.0.1"));
        ON_CALL(*this, GetInfo()).WillByDefault(ReturnRef(_peerInfo));
        ON_CALL(*this, GetServiceDescriptor()).WillByDefault(ReturnRef(_serviceDescriptor));
        ON_CALL(*this, GetProtocolVersion()).WillByDefault(Return(_protocolVersion));
    }

    //IVasioPeer
    MOCK_METHOD(void, SendIbMsg, (SerializedMessage), (override));
    MOCK_METHOD(void, Subscribe, (VAsioMsgSubscriber), (override));
    MOCK_METHOD(const VAsioPeerInfo&, GetInfo, (), (const, override));
    MOCK_METHOD(void, SetInfo, (VAsioPeerInfo), (override));
    MOCK_METHOD(std::string, GetRemoteAddress, (), (const, override));
    MOCK_METHOD(std::string, GetLocalAddress, (), (const, override));
    MOCK_METHOD(void, StartAsyncRead, (), (override));
    MOCK_METHOD(void, SetProtocolVersion, (ProtocolVersion), (override));
    MOCK_METHOD(ProtocolVersion, GetProtocolVersion, (), (const, override));

    //IIbServiceEndpoint
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
    return reply.status == SubscriptionAcknowledge::Status::Success
        && reply.subscriber == subscriber
        ;
}

} //namespace 

//////////////////////////////////////////////////////////////////////
// Test Fixture
//////////////////////////////////////////////////////////////////////

namespace ib {
namespace mw {
class VAsioConnectionTest : public testing::Test
{
protected:
    VAsioConnectionTest()
        : _connection({}, "VAsioConnectionTest", 1)
    {
        _connection.SetLogger(&_dummyLogger);
    }
    VAsioConnection _connection;
    MockVAsioPeer _from;
    test::DummyLogger _dummyLogger;

    //we are a friend class
    // - allow selected access to private member
    template<typename MessageT, typename ServiceT>
    void RegisterIbMsgReceiver(const std::string& networkName, ib::mw::IIbMessageReceiver<MessageT>* receiver)
    {
        _connection.RegisterIbMsgReceiver<MessageT, ServiceT>(networkName, receiver);
    }
};
} //mw
} //ib
//////////////////////////////////////////////////////////////////////
// Versioned initial handshake
//////////////////////////////////////////////////////////////////////

TEST_F(VAsioConnectionTest, unsupported_version_connect)
{
    ParticipantAnnouncement announcement{};
    announcement.peerInfo = _from.GetInfo();
    announcement.messageHeader.versionHigh = 1;

    SerializedMessage message(announcement);

    auto validator = [](const ParticipantAnnouncementReply& reply)
    {
        return reply.status == ParticipantAnnouncementReply::Status::Failed;
    };
    EXPECT_CALL(_from, SendIbMsg(AnnouncementReplyMatcher(validator))).Times(1);
    _connection.OnSocketData(&_from, std::move(message));

}

TEST_F(VAsioConnectionTest, unsupported_version_reply_from_registry_should_throw)
{
    ParticipantAnnouncementReply reply{};
    reply.remoteHeader.versionHigh = 1;
    reply.remoteHeader.versionLow = 1;
    reply.status = ParticipantAnnouncementReply::Status::Failed;

    // a failed connection to a registry is fatal
    _from._peerInfo.participantId = VAsioConnection::RegistryParticipantId;

    SerializedMessage message(reply);
    EXPECT_THROW(
        _connection.OnSocketData(&_from, std::move(message)),
        ib::ProtocolError);
}

TEST_F(VAsioConnectionTest, supported_version_reply_from_registry_must_not_throw)
{
    ParticipantAnnouncementReply reply{};
    reply.status = ParticipantAnnouncementReply::Status::Success;

    _from._peerInfo.participantId = VAsioConnection::RegistryParticipantId;

    SerializedMessage message(reply);
    EXPECT_NO_THROW(
        _connection.OnSocketData(&_from, std::move(message))
    );
}

TEST_F(VAsioConnectionTest, current_version_connect)
{
    ParticipantAnnouncement announcement{}; //sets correct version in header
    announcement.peerInfo = _from.GetInfo();

    SerializedMessage message(announcement);

    auto validator = [](const ParticipantAnnouncementReply& reply)
    {
        return reply.status == ParticipantAnnouncementReply::Status::Success;
    };
    EXPECT_CALL(_from, SendIbMsg(AnnouncementReplyMatcher(validator))).Times(1);

    _connection.OnSocketData(&_from, std::move(message));
}

//////////////////////////////////////////////////////////////////////
// Versioned subscriptions: test backward compatibility
//////////////////////////////////////////////////////////////////////

//Disabled because we do not have a versioned Ser/Des that detects
// the different version used for transmission, this is a work in progress.
    TEST_F(VAsioConnectionTest, DISABLED_versioned_send_testmessage)
{
    // We send a 'version1', but expect to receive a 'version2' 
    test::version1::TestMessage message;
    message.integer = 1234;
    message.str ="1234";

    //Setup subscriptions for transmisison
    using MessageTrait = ib::mw::IbMsgTraits<decltype(message)>;
    VAsioMsgSubscriber subscriber;
    subscriber.msgTypeName = MessageTrait::SerdesName();
    subscriber.networkName = "unittest";
    subscriber.version = MessageTrait::Version();
    subscriber.receiverIdx = 0; //the first receiver

    EXPECT_TRUE(subscriber.version == 1);

    // ReceiveSubscriptionAnnouncement -> sets internal structures up
    auto subscriberBuffer = SerializedMessage(subscriber);
    EXPECT_CALL(_from, SendIbMsg(SubscriptionAcknowledgeMatcher(subscriber))).Times(1);
    _connection.OnSocketData(&_from, std::move(subscriberBuffer));

    // Create a receiver with index 0 and a different TestMessage _version_
    MockIbMessageReceiver mockReceiver;
    RegisterIbMsgReceiver<test::version2::TestMessage,MockIbMessageReceiver>(subscriber.networkName, &mockReceiver);
    //the actual message

    auto buffer = SerializedMessage(message, _from.GetServiceDescriptor().to_endpointAddress(),
        subscriber.receiverIdx);

    _connection.OnSocketData(&_from, std::move(buffer));
}
