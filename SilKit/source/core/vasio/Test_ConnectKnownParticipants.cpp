// Copyright (c) 2022 Vector Informatik GmbH
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


#include "ConnectKnownParticipants.hpp"

#include "MockLogger.hpp"

#include "MockConnectKnownParticipantsListener.hpp"
#include "MockConnectionMethods.hpp"
#include "MockConnectPeer.hpp"
#include "MockIoContext.hpp"
#include "MockRawByteStream.hpp"
#include "MockTimer.hpp"
#include "MockVAsioPeer.hpp"

#include "Hash.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"


namespace {


using namespace SilKit::Core;

using namespace std::chrono_literals;


using ::testing::_;
using ::testing::AnyOf;
using ::testing::Contains;
using ::testing::Eq;
using ::testing::ExplainMatchResult;
using ::testing::NiceMock;
using ::testing::Pointee;
using ::testing::Property;
using ::testing::Sequence;
using ::testing::StrictMock;
using ::testing::Return;

using SilKit::Services::Logging::MockLogger;
using VSilKit::MockIoContextWithExecutionQueue;
using VSilKit::MockRawByteStream;
using VSilKit::MockTimer;
using VSilKit::MockTimerThatExpiresImmediately;

using PeerEvent = SilKit::Core::ConnectKnownParticipants::PeerEvent;


MATCHER_P(WithParticipantName, participantName, "")
{
    return ExplainMatchResult(Eq(participantName), arg.participantName, result_listener);
}

MATCHER_P(WithRemoteEndpoint, remoteEndpoint, "")
{
    return ExplainMatchResult(Property(&IRawByteStream::GetRemoteEndpoint, Eq(remoteEndpoint)), *arg, result_listener);
}


struct Test_ConnectKnownParticipants : ::testing::Test
{
    MockIoContextWithExecutionQueue ioContext;
    NiceMock<MockLogger> logger;

    ConnectKnownParticipantsSettings settings;

    auto MakeConnectPeerThatSucceeds(const VAsioPeerInfo& peerInfo) -> std::unique_ptr<MockConnectPeerThatSucceeds>
    {
        auto connectPeer{std::make_unique<MockConnectPeerThatSucceeds>(ioContext, peerInfo)};
        EXPECT_CALL(*connectPeer, DoSetListener).Times(1);
        EXPECT_CALL(*connectPeer, DoAsyncConnect).Times(1);
        EXPECT_CALL(*connectPeer, MakeRawByteStream).WillOnce([](const std::string& acceptorUri) {
            auto stream{std::make_unique<MockRawByteStream>()};
            stream->remoteEndpoint = acceptorUri;
            return stream;
        });
        return connectPeer;
    }

    auto MakeConnectPeerThatFails(const VAsioPeerInfo& peerInfo) -> std::unique_ptr<MockConnectPeerThatFails>
    {
        auto connectPeer{std::make_unique<MockConnectPeerThatFails>(ioContext, peerInfo)};
        EXPECT_CALL(*connectPeer, DoSetListener).Times(1);
        EXPECT_CALL(*connectPeer, DoAsyncConnect).Times(1);
        return connectPeer;
    }
};


TEST_F(Test_ConnectKnownParticipants, empty_known_participants_calls_success_handlers_in_order)
{
    // Arrange

    Sequence s1;

    StrictMock<MockConnectionMethods> connectionMethods;

    MockConnectKnownParticipantsListener listener;
    EXPECT_CALL(listener, OnConnectKnownParticipantsWaitingForAllReplies).Times(1).InSequence(s1);
    EXPECT_CALL(listener, OnConnectKnownParticipantsAllRepliesReceived).Times(1).InSequence(s1);

    // Act

    ConnectKnownParticipants connectKnownParticipants{ioContext, connectionMethods, listener, settings};
    connectKnownParticipants.SetLogger(logger);

    connectKnownParticipants.SetKnownParticipants({});
    connectKnownParticipants.StartConnecting();

    ioContext.Run();
}


TEST_F(Test_ConnectKnownParticipants, successful_connection_initiates_waiting_for_replies)
{
    auto MakeSucceedingConnectPeer{[this](const VAsioPeerInfo& peerInfo) {
        return MakeConnectPeerThatSucceeds(peerInfo);
    }};

    VAsioPeerInfo peerInfo;
    peerInfo.participantName = "A";
    peerInfo.participantId = SilKit::Util::Hash::Hash(peerInfo.participantName);
    peerInfo.acceptorUris.emplace_back("local:///one");
    peerInfo.capabilities = "";

    // Arrange

    Sequence s1;

    StrictMock<MockConnectionMethods> connectionMethods;
    {
        EXPECT_CALL(connectionMethods, MakeConnectPeer(WithParticipantName(peerInfo.participantName)))
            .InSequence(s1)
            .WillOnce(MakeSucceedingConnectPeer);

        EXPECT_CALL(connectionMethods, MakeVAsioPeer(WithRemoteEndpoint(peerInfo.acceptorUris.front())))
            .InSequence(s1)
            .WillOnce([](std::unique_ptr<IRawByteStream>) {
                auto vAsioPeer{std::make_unique<NiceMock<MockVAsioPeer>>()};
                return vAsioPeer;
            });

        EXPECT_CALL(connectionMethods, HandleConnectedPeer).InSequence(s1);
        EXPECT_CALL(connectionMethods, AddPeer).InSequence(s1);
    }

    MockConnectKnownParticipantsListener listener;
    EXPECT_CALL(listener, OnConnectKnownParticipantsWaitingForAllReplies).Times(1).InSequence(s1);

    // Act

    ConnectKnownParticipants connectKnownParticipants{ioContext, connectionMethods, listener, settings};
    connectKnownParticipants.SetLogger(logger);

    connectKnownParticipants.SetKnownParticipants({peerInfo});
    connectKnownParticipants.StartConnecting();

    ioContext.Run();
}


TEST_F(Test_ConnectKnownParticipants, direct_connect_fallback_to_remote_connect_fallback_to_proxy_triggers_failure)
{
    auto MakeFailingConnectPeer{[this](const VAsioPeerInfo& peerInfo) {
        return MakeConnectPeerThatFails(peerInfo);
    }};

    VAsioPeerInfo peerInfo;
    peerInfo.participantName = "A";
    peerInfo.participantId = SilKit::Util::Hash::Hash(peerInfo.participantName);
    peerInfo.acceptorUris.emplace_back("local:///one");
    peerInfo.capabilities = "";

    // Arrange

    Sequence s1;

    StrictMock<MockConnectionMethods> connectionMethods;
    MockConnectKnownParticipantsListener listener;

    EXPECT_CALL(connectionMethods, MakeConnectPeer(WithParticipantName(peerInfo.participantName)))
        .InSequence(s1)
        .WillOnce(MakeFailingConnectPeer);

    EXPECT_CALL(connectionMethods, TryRemoteConnectRequest(WithParticipantName(peerInfo.participantName)))
        .InSequence(s1)
        .WillOnce(Return(false));

    EXPECT_CALL(connectionMethods, TryProxyConnect(WithParticipantName(peerInfo.participantName)))
        .InSequence(s1)
        .WillOnce(Return(false));

    EXPECT_CALL(listener, OnConnectKnownParticipantsFailure).Times(1).InSequence(s1);

    // Act

    ConnectKnownParticipants connectKnownParticipants{ioContext, connectionMethods, listener, settings};
    connectKnownParticipants.SetLogger(logger);

    connectKnownParticipants.SetKnownParticipants({peerInfo});
    connectKnownParticipants.StartConnecting();

    ioContext.Run();
}


TEST_F(Test_ConnectKnownParticipants, remote_connect_timer_expiry_fallback_to_proxy)
{
    auto MakeFailingConnectPeer{[this](const VAsioPeerInfo& peerInfo) {
        return MakeConnectPeerThatFails(peerInfo);
    }};

    VAsioPeerInfo peerInfo;
    peerInfo.participantName = "A";
    peerInfo.participantId = SilKit::Util::Hash::Hash(peerInfo.participantName);
    peerInfo.acceptorUris.emplace_back("local:///one");
    peerInfo.capabilities = "";

    StrictMock<MockConnectionMethods> connectionMethods;
    MockConnectKnownParticipantsListener listener;

    ConnectKnownParticipants connectKnownParticipants{ioContext, connectionMethods, listener, settings};
    connectKnownParticipants.SetLogger(logger);

    // Arrange

    Sequence s1;

    EXPECT_CALL(connectionMethods, MakeConnectPeer(WithParticipantName(peerInfo.participantName)))
        .InSequence(s1)
        .WillOnce(MakeFailingConnectPeer);

    EXPECT_CALL(connectionMethods, TryRemoteConnectRequest(WithParticipantName(peerInfo.participantName)))
        .InSequence(s1)
        .WillOnce(Return(true));

    EXPECT_CALL(ioContext, MakeTimer).InSequence(s1).WillOnce([this] {
        const auto timeout{static_cast<std::chrono::nanoseconds>(settings.remoteConnectRequestTimeout)};
        auto timer{std::make_unique<MockTimerThatExpiresImmediately>(ioContext)};
        EXPECT_CALL(*timer, DoSetListener);
        EXPECT_CALL(*timer, DoAsyncWaitFor(timeout));
        return timer;
    });

    EXPECT_CALL(connectionMethods, TryProxyConnect(WithParticipantName(peerInfo.participantName)))
        .InSequence(s1)
        .WillOnce(Return(false));

    EXPECT_CALL(listener, OnConnectKnownParticipantsFailure).Times(1).InSequence(s1);

    // Act

    connectKnownParticipants.SetKnownParticipants({peerInfo});
    connectKnownParticipants.StartConnecting();

    ioContext.Run();
}


TEST_F(Test_ConnectKnownParticipants, remote_connect_waits_for_replies_after_connecting_and_announcement_notifications)
{
    // The REMOTE_PARTICIPANT_IS_CONNECTING notification is generally sent via the registry before the remote
    // connection is actually attempted. The first message over the remote connection triggers the
    // REMOTE_PARTICIPANT_ANNOUNCEMENT notification.

    auto MakeFailingConnectPeer{[this](const VAsioPeerInfo& peerInfo) {
        return MakeConnectPeerThatFails(peerInfo);
    }};

    VAsioPeerInfo peerInfo;
    peerInfo.participantName = "A";
    peerInfo.participantId = SilKit::Util::Hash::Hash(peerInfo.participantName);
    peerInfo.acceptorUris.emplace_back("local:///one");
    peerInfo.capabilities = "";

    StrictMock<MockConnectionMethods> connectionMethods;
    MockConnectKnownParticipantsListener listener;

    ConnectKnownParticipants connectKnownParticipants{ioContext, connectionMethods, listener, settings};
    connectKnownParticipants.SetLogger(logger);

    // Arrange

    Sequence s1;

    EXPECT_CALL(connectionMethods, MakeConnectPeer(WithParticipantName(peerInfo.participantName)))
        .InSequence(s1)
        .WillOnce(MakeFailingConnectPeer);

    EXPECT_CALL(connectionMethods, TryRemoteConnectRequest(WithParticipantName(peerInfo.participantName)))
        .InSequence(s1)
        .WillOnce(Return(true));

    EXPECT_CALL(ioContext, MakeTimer).InSequence(s1).WillOnce([this, &peerInfo, &connectKnownParticipants] {
        ioContext.Post([&peerInfo, &connectKnownParticipants] {
            connectKnownParticipants.HandlePeerEvent(peerInfo.participantName,
                                                     PeerEvent::REMOTE_PARTICIPANT_IS_CONNECTING);
            connectKnownParticipants.HandlePeerEvent(peerInfo.participantName,
                                                     PeerEvent::REMOTE_PARTICIPANT_ANNOUNCEMENT);
        });
        return std::make_unique<NiceMock<MockTimer>>();
    });

    EXPECT_CALL(listener, OnConnectKnownParticipantsWaitingForAllReplies).Times(1).InSequence(s1);

    // Act

    connectKnownParticipants.SetKnownParticipants({peerInfo});
    connectKnownParticipants.StartConnecting();

    ioContext.Run();
}


TEST_F(Test_ConnectKnownParticipants, remote_connect_waits_for_replies_after_announcement_and_connecting_notifications)
{
    // Also see the remote_connect_waits_for_replies_after_connecting_and_announcement_notifications test.

    // It is possible that the REMOTE_PARTICIPANT_ANNOUNCEMENT is received before the REMOTE_PARTICIPANT_IS_CONNECTING.
    // This might happen if connection establishment is really fast, and the registry is really slow for some reason.

    auto MakeFailingConnectPeer{[this](const VAsioPeerInfo& peerInfo) {
        return MakeConnectPeerThatFails(peerInfo);
    }};

    VAsioPeerInfo peerInfo;
    peerInfo.participantName = "A";
    peerInfo.participantId = SilKit::Util::Hash::Hash(peerInfo.participantName);
    peerInfo.acceptorUris.emplace_back("local:///one");
    peerInfo.capabilities = "";

    StrictMock<MockConnectionMethods> connectionMethods;
    MockConnectKnownParticipantsListener listener;

    ConnectKnownParticipants connectKnownParticipants{ioContext, connectionMethods, listener, settings};
    connectKnownParticipants.SetLogger(logger);

    // Arrange

    Sequence s1;

    EXPECT_CALL(connectionMethods, MakeConnectPeer(WithParticipantName(peerInfo.participantName)))
        .InSequence(s1)
        .WillOnce(MakeFailingConnectPeer);

    EXPECT_CALL(connectionMethods, TryRemoteConnectRequest(WithParticipantName(peerInfo.participantName)))
        .InSequence(s1)
        .WillOnce(Return(true));

    EXPECT_CALL(ioContext, MakeTimer).InSequence(s1).WillOnce([this, &peerInfo, &connectKnownParticipants] {
        ioContext.Post([&peerInfo, &connectKnownParticipants] {
            connectKnownParticipants.HandlePeerEvent(peerInfo.participantName,
                                                     PeerEvent::REMOTE_PARTICIPANT_ANNOUNCEMENT);
            connectKnownParticipants.HandlePeerEvent(peerInfo.participantName,
                                                     PeerEvent::REMOTE_PARTICIPANT_IS_CONNECTING);
        });
        return std::make_unique<NiceMock<MockTimer>>();
    });

    EXPECT_CALL(listener, OnConnectKnownParticipantsWaitingForAllReplies).Times(1).InSequence(s1);

    // Act

    connectKnownParticipants.SetKnownParticipants({peerInfo});
    connectKnownParticipants.StartConnecting();

    ioContext.Run();
}


TEST_F(Test_ConnectKnownParticipants, remote_connect_fallback_to_proxy_after_failed_to_connect_notification)
{
    // Also see the remote_connect_waits_for_replies_after_connecting_and_announcement_notifications test.

    // It is possible that the REMOTE_PARTICIPANT_ANNOUNCEMENT is received before the REMOTE_PARTICIPANT_IS_CONNECTING.
    // This might happen if connection establishment is really fast, and the registry is really slow for some reason.

    auto MakeFailingConnectPeer{[this](const VAsioPeerInfo& peerInfo) {
        return MakeConnectPeerThatFails(peerInfo);
    }};

    VAsioPeerInfo peerInfo;
    peerInfo.participantName = "A";
    peerInfo.participantId = SilKit::Util::Hash::Hash(peerInfo.participantName);
    peerInfo.acceptorUris.emplace_back("local:///one");
    peerInfo.capabilities = "";

    StrictMock<MockConnectionMethods> connectionMethods;
    MockConnectKnownParticipantsListener listener;

    ConnectKnownParticipants connectKnownParticipants{ioContext, connectionMethods, listener, settings};
    connectKnownParticipants.SetLogger(logger);

    // Arrange

    Sequence s1;

    EXPECT_CALL(connectionMethods, MakeConnectPeer(WithParticipantName(peerInfo.participantName)))
        .InSequence(s1)
        .WillOnce(MakeFailingConnectPeer);

    EXPECT_CALL(connectionMethods, TryRemoteConnectRequest(WithParticipantName(peerInfo.participantName)))
        .InSequence(s1)
        .WillOnce(Return(true));

    EXPECT_CALL(ioContext, MakeTimer).InSequence(s1).WillOnce([this, &peerInfo, &connectKnownParticipants] {
        ioContext.Post([&peerInfo, &connectKnownParticipants] {
            connectKnownParticipants.HandlePeerEvent(peerInfo.participantName,
                                                     PeerEvent::REMOTE_PARTICIPANT_FAILED_TO_CONNECT);
        });
        return std::make_unique<NiceMock<MockTimer>>();
    });

    EXPECT_CALL(connectionMethods, TryProxyConnect(WithParticipantName(peerInfo.participantName)))
        .InSequence(s1)
        .WillOnce(Return(false));

    EXPECT_CALL(listener, OnConnectKnownParticipantsFailure).Times(1).InSequence(s1);

    // Act

    connectKnownParticipants.SetKnownParticipants({peerInfo});
    connectKnownParticipants.StartConnecting();

    ioContext.Run();
}


} // namespace
