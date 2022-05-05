// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "IVAsioPeer.hpp"
#include "VAsioConnection.hpp"
#include "MockParticipant.hpp" // for DummyLogger

#include <chrono>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

using namespace std::chrono_literals;
using namespace ib::mw;

using testing::Return;
using testing::ReturnRef;
using testing::_;

namespace
{

struct MockVAsioPeer
    : public IVAsioPeer
    , public IIbServiceEndpoint
    , public IVasioProtocolPeer
{
    VAsioPeerUri _peerUri;
    VAsioPeerInfo _peerInfo;
    ServiceDescriptor _serviceDescriptor;
    MockVAsioPeer()
    {
        _peerUri.participantId = 1234;
        _peerUri.participantName = "MockVAsioPeer";
        _peerUri.acceptorUris.push_back("tcp://localhost:1234");
        _peerInfo.acceptorHost = "localhost";
        _peerInfo.acceptorPort = 1234;

        ON_CALL(*this, GetLocalAddress()).WillByDefault(Return("127.0.0.1"));
        ON_CALL(*this, GetRemoteAddress()).WillByDefault(Return("127.0.0.1"));
        ON_CALL(*this, GetUri()).WillByDefault(ReturnRef(_peerUri));
        ON_CALL(*this, GetInfo()).WillByDefault(ReturnRef(_peerInfo));
        ON_CALL(*this, GetServiceDescriptor()).WillByDefault(ReturnRef(_serviceDescriptor));
    }
    //IVasioPeer

    MOCK_METHOD(void, SendIbMsg, (MessageBuffer), (override));
    MOCK_METHOD(void, Subscribe, (VAsioMsgSubscriber), (override));
    MOCK_METHOD(const VAsioPeerInfo&, GetInfo, (), (const, override));
    MOCK_METHOD(void, SetInfo, (VAsioPeerInfo), (override));
    MOCK_METHOD(void, SetUri, (VAsioPeerUri), (override));
    MOCK_METHOD(const VAsioPeerUri&, GetUri, (), (override, const));
    MOCK_METHOD(std::string, GetRemoteAddress, (), (const, override));
    MOCK_METHOD(std::string, GetLocalAddress, (), (const, override));
    //IIbServiceEndpoint
    MOCK_METHOD(void, SetServiceDescriptor, (const ServiceDescriptor& serviceDescriptor), (override));
    MOCK_METHOD(const ServiceDescriptor&, GetServiceDescriptor, (), (override, const));

    //IVasioProtocolPeer
    MOCK_METHOD(void, VersionNotSupported, (), (override));
};

// Wire protocol, using the Serdes* methods
auto Serialize(const ParticipantAnnouncement& announcement) -> MessageBuffer
{
    MessageBuffer buffer;
    uint32_t msgSizePlaceholder{0u};

    buffer << msgSizePlaceholder
           << VAsioMsgKind::IbRegistryMessage
           << RegistryMessageKind::ParticipantAnnouncement
           << announcement;
    return buffer;
}

template<typename IbMessageT>
auto Deserialize(MessageBuffer) -> IbMessageT;

template<>
auto Deserialize(MessageBuffer buffer) -> ParticipantAnnouncementReply
{
    VAsioMsgKind msgkind;
    RegistryMessageKind kind;
    ParticipantAnnouncementReply reply;
    buffer
        >> msgkind
        >> kind
        >> reply
        ;
    return reply;
}

void DropMessageSize(MessageBuffer& buffer)
{
    //After receiving we have to strip the message size, modifying the read position in the buffer
    uint32_t messageSize{0u};
    buffer >> messageSize;
}

MATCHER_P(AnnouncementReplyMatcher, announcement,
    "Deserialize the MessageBuffer and check the announcement's reply")
{
    auto buffer = arg;
    DropMessageSize(buffer);
    auto reply  = Deserialize<ParticipantAnnouncementReply>(buffer);
    return true;
}

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
};


TEST_F(VAsioConnectionTest, unsupported_version_connect)
{
    ParticipantAnnouncement announcement{};
    announcement.peerUri = _from.GetUri();
    announcement.messageHeader.versionHigh = 1;

    auto buffer = Serialize(announcement);
    DropMessageSize(buffer);

    EXPECT_CALL(_from, VersionNotSupported()).Times(1);
    _connection.OnSocketData(&_from, std::move(buffer));

}

TEST_F(VAsioConnectionTest, current_version_connect)
{
    ParticipantAnnouncement announcement{}; //sets correct version in header
    announcement.peerUri = _from.GetUri();

    auto buffer = Serialize(announcement);
    DropMessageSize(buffer);

    EXPECT_CALL(_from, SendIbMsg(AnnouncementReplyMatcher(announcement))).Times(1);

    EXPECT_CALL(_from, VersionNotSupported()).Times(0);
    _connection.OnSocketData(&_from, std::move(buffer));
}

} //end namespace
