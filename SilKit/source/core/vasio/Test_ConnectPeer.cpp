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


#include "ConnectPeer.hpp"

#include "MockLogger.hpp"

#include "MockConnector.hpp"
#include "MockConnectPeer.hpp"
#include "MockIoContext.hpp"
#include "MockRawByteStream.hpp"

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
using ::testing::NiceMock;
using ::testing::Sequence;
using ::testing::Return;

using SilKit::Services::Logging::MockLogger;
using VSilKit::MockIoContextWithExecutionQueue;
using VSilKit::MockConnectorThatFails;
using VSilKit::MockConnectorThatSucceeds;
using VSilKit::MockRawByteStream;


struct Test_ConnectPeer : ::testing::Test
{
    MockIoContextWithExecutionQueue ioContext;
    NiceMock<MockLogger> logger;

    auto MakeConnectorThatFails(std::chrono::milliseconds timeout) -> std::unique_ptr<MockConnectorThatFails>
    {
        auto connector{std::make_unique<MockConnectorThatFails>(ioContext)};
        EXPECT_CALL(*connector, DoSetListener);
        EXPECT_CALL(*connector, DoAsyncConnect(timeout));
        return connector;
    }

    auto MakeConnectorThatSucceeds(std::chrono::milliseconds timeout) -> std::unique_ptr<MockConnectorThatSucceeds>
    {
        auto connector{std::make_unique<MockConnectorThatSucceeds>(ioContext)};
        EXPECT_CALL(*connector, DoSetListener);
        EXPECT_CALL(*connector, DoAsyncConnect(timeout));
        return connector;
    }
};


TEST_F(Test_ConnectPeer, tcp_hosts_are_resolved_and_tried_in_order_with_specified_timeout)
{
    static constexpr bool DOMAIN_SOCKETS_ENABLED{true};
    static constexpr auto TIMEOUT{4321ms};

    auto MakeConnector{[this] {
        return MakeConnectorThatFails(TIMEOUT);
    }};

    // Arrange

    Sequence s1;

    EXPECT_CALL(ioContext, Resolve("host"))
        .InSequence(s1)
        .WillOnce(Return(std::vector<std::string>{"1.2.3.4", "5.6.7.8"}));

    EXPECT_CALL(ioContext, MakeTcpConnector("1.2.3.4", 1234)).InSequence(s1).WillOnce(MakeConnector);
    EXPECT_CALL(ioContext, MakeTcpConnector("5.6.7.8", 1234)).InSequence(s1).WillOnce(MakeConnector);

    MockConnectPeerListener connectPeerListener;
    EXPECT_CALL(connectPeerListener, OnConnectPeerSuccess).Times(0);
    EXPECT_CALL(connectPeerListener, OnConnectPeerFailure).Times(1).InSequence(s1);

    // Act

    VAsioPeerInfo peerInfo;
    peerInfo.participantName = "A";
    peerInfo.participantId = SilKit::Util::Hash::Hash(peerInfo.participantName);
    peerInfo.acceptorUris.emplace_back("tcp://host:1234");
    peerInfo.capabilities = "";

    ConnectPeer connectPeer{&ioContext, &logger, peerInfo, DOMAIN_SOCKETS_ENABLED};
    connectPeer.SetListener(connectPeerListener);
    connectPeer.AsyncConnect(1, TIMEOUT);

    ioContext.Run();
}


TEST_F(Test_ConnectPeer, local_is_tried_before_tcp_but_order_is_stable)
{
    static constexpr bool DOMAIN_SOCKETS_ENABLED{true};
    static constexpr auto TIMEOUT{4321ms};

    auto MakeConnector{[this] {
        return MakeConnectorThatFails(TIMEOUT);
    }};

    // Arrange

    Sequence s1;

    EXPECT_CALL(ioContext, Resolve("host"))
        .InSequence(s1)
        .WillOnce(Return(std::vector<std::string>{"1.2.3.4", "5.6.7.8"}))
        .WillOnce(Return(std::vector<std::string>{"9.0.1.2", "3.4.5.6"}));

    EXPECT_CALL(ioContext, MakeLocalConnector("/some/path")).InSequence(s1).WillOnce(MakeConnector);
    EXPECT_CALL(ioContext, MakeLocalConnector("/some/other/path")).InSequence(s1).WillOnce(MakeConnector);
    EXPECT_CALL(ioContext, MakeTcpConnector("1.2.3.4", 1234)).InSequence(s1).WillOnce(MakeConnector);
    EXPECT_CALL(ioContext, MakeTcpConnector("5.6.7.8", 1234)).InSequence(s1).WillOnce(MakeConnector);
    EXPECT_CALL(ioContext, MakeTcpConnector("9.0.1.2", 5678)).InSequence(s1).WillOnce(MakeConnector);
    EXPECT_CALL(ioContext, MakeTcpConnector("3.4.5.6", 5678)).InSequence(s1).WillOnce(MakeConnector);

    MockConnectPeerListener connectPeerListener;
    EXPECT_CALL(connectPeerListener, OnConnectPeerSuccess).Times(0);
    EXPECT_CALL(connectPeerListener, OnConnectPeerFailure).Times(1).InSequence(s1);

    // Act

    VAsioPeerInfo peerInfo;
    peerInfo.participantName = "A";
    peerInfo.participantId = SilKit::Util::Hash::Hash(peerInfo.participantName);
    peerInfo.acceptorUris.emplace_back("local:///some/path");
    peerInfo.acceptorUris.emplace_back("tcp://host:1234");
    peerInfo.acceptorUris.emplace_back("local:///some/other/path");
    peerInfo.acceptorUris.emplace_back("tcp://host:5678");
    peerInfo.capabilities = "";

    ConnectPeer connectPeer{&ioContext, &logger, peerInfo, DOMAIN_SOCKETS_ENABLED};
    connectPeer.SetListener(connectPeerListener);
    connectPeer.AsyncConnect(1, TIMEOUT);

    ioContext.Run();
}


TEST_F(Test_ConnectPeer, retry_count_is_honored)
{
    static constexpr bool DOMAIN_SOCKETS_ENABLED{true};
    static constexpr size_t RETRY_COUNT{3};
    static constexpr auto TIMEOUT{4321ms};

    auto MakeConnector{[this] {
        return MakeConnectorThatFails(TIMEOUT);
    }};

    // Arrange

    Sequence s1;

    EXPECT_CALL(ioContext, Resolve("host")).InSequence(s1).WillOnce(Return(std::vector<std::string>{"1.2.3.4"}));

    EXPECT_CALL(ioContext, MakeTcpConnector("1.2.3.4", 1234))
        .Times(RETRY_COUNT)
        .InSequence(s1)
        .WillRepeatedly(MakeConnector);

    MockConnectPeerListener connectPeerListener;
    EXPECT_CALL(connectPeerListener, OnConnectPeerSuccess).Times(0);
    EXPECT_CALL(connectPeerListener, OnConnectPeerFailure).Times(1).InSequence(s1);

    // Act

    VAsioPeerInfo peerInfo;
    peerInfo.participantName = "A";
    peerInfo.participantId = SilKit::Util::Hash::Hash(peerInfo.participantName);
    peerInfo.acceptorUris.emplace_back("tcp://host:1234");
    peerInfo.capabilities = "";

    ConnectPeer connectPeer{&ioContext, &logger, peerInfo, DOMAIN_SOCKETS_ENABLED};
    connectPeer.SetListener(connectPeerListener);
    connectPeer.AsyncConnect(RETRY_COUNT, TIMEOUT);

    ioContext.Run();
}


TEST_F(Test_ConnectPeer, each_retry_tries_each_uri)
{
    static constexpr bool DOMAIN_SOCKETS_ENABLED{true};
    static constexpr size_t RETRY_COUNT{2};
    static constexpr auto TIMEOUT{4321ms};

    auto MakeConnector{[this] {
        return MakeConnectorThatFails(TIMEOUT);
    }};

    // Arrange

    Sequence s1;

    EXPECT_CALL(ioContext, MakeLocalConnector("/one")).InSequence(s1).WillOnce(MakeConnector);
    EXPECT_CALL(ioContext, MakeLocalConnector("/two")).InSequence(s1).WillOnce(MakeConnector);
    EXPECT_CALL(ioContext, MakeLocalConnector("/one")).InSequence(s1).WillOnce(MakeConnector);
    EXPECT_CALL(ioContext, MakeLocalConnector("/two")).InSequence(s1).WillOnce(MakeConnector);

    MockConnectPeerListener connectPeerListener;
    EXPECT_CALL(connectPeerListener, OnConnectPeerSuccess).Times(0);
    EXPECT_CALL(connectPeerListener, OnConnectPeerFailure).Times(1).InSequence(s1);

    // Act

    VAsioPeerInfo peerInfo;
    peerInfo.participantName = "A";
    peerInfo.participantId = SilKit::Util::Hash::Hash(peerInfo.participantName);
    peerInfo.acceptorUris.emplace_back("local:///one");
    peerInfo.acceptorUris.emplace_back("local:///two");
    peerInfo.capabilities = "";

    ConnectPeer connectPeer{&ioContext, &logger, peerInfo, DOMAIN_SOCKETS_ENABLED};
    connectPeer.SetListener(connectPeerListener);
    connectPeer.AsyncConnect(RETRY_COUNT, TIMEOUT);

    ioContext.Run();
}


TEST_F(Test_ConnectPeer, disabling_local_domain_ignores_local_uris)
{
    static constexpr bool DOMAIN_SOCKETS_ENABLED{false};
    static constexpr auto TIMEOUT{4321ms};

    auto MakeConnector{[this] {
        return MakeConnectorThatFails(TIMEOUT);
    }};

    // Arrange

    Sequence s1;

    EXPECT_CALL(ioContext, Resolve("host")).InSequence(s1).WillOnce(Return(std::vector<std::string>{"1.2.3.4"}));

    EXPECT_CALL(ioContext, MakeTcpConnector("1.2.3.4", 1234)).InSequence(s1).WillOnce(MakeConnector);

    MockConnectPeerListener connectPeerListener;
    EXPECT_CALL(connectPeerListener, OnConnectPeerSuccess).Times(0);
    EXPECT_CALL(connectPeerListener, OnConnectPeerFailure).Times(1).InSequence(s1);

    // Act

    VAsioPeerInfo peerInfo;
    peerInfo.participantName = "A";
    peerInfo.participantId = SilKit::Util::Hash::Hash(peerInfo.participantName);
    peerInfo.acceptorUris.emplace_back("tcp://host:1234");
    peerInfo.acceptorUris.emplace_back("local:///one");
    peerInfo.capabilities = "";

    ConnectPeer connectPeer{&ioContext, &logger, peerInfo, DOMAIN_SOCKETS_ENABLED};
    connectPeer.SetListener(connectPeerListener);
    connectPeer.AsyncConnect(1, TIMEOUT);

    ioContext.Run();
}


TEST_F(Test_ConnectPeer, successful_connection_skips_remainder)
{
    static constexpr bool DOMAIN_SOCKETS_ENABLED{true};
    static constexpr auto TIMEOUT{4321ms};

    auto MakeFailingConnector{[this] {
        return MakeConnectorThatFails(TIMEOUT);
    }};

    auto MakeSucceedingConnector{[this] {
        auto connector{MakeConnectorThatSucceeds(TIMEOUT)};
        EXPECT_CALL(*connector, MakeRawByteStream).WillOnce([] {
            return std::make_unique<MockRawByteStream>();
        });
        return connector;
    }};

    // Arrange

    Sequence s1;

    EXPECT_CALL(ioContext, MakeLocalConnector("/one")).InSequence(s1).WillOnce(MakeFailingConnector);
    EXPECT_CALL(ioContext, MakeLocalConnector("/two")).InSequence(s1).WillOnce(MakeFailingConnector);
    EXPECT_CALL(ioContext, MakeLocalConnector("/one")).InSequence(s1).WillOnce(MakeSucceedingConnector);

    MockConnectPeerListener connectPeerListener;
    EXPECT_CALL(connectPeerListener, OnConnectPeerSuccess).Times(1).InSequence(s1);
    EXPECT_CALL(connectPeerListener, OnConnectPeerFailure).Times(0);

    // Act

    VAsioPeerInfo peerInfo;
    peerInfo.participantName = "A";
    peerInfo.participantId = SilKit::Util::Hash::Hash(peerInfo.participantName);
    peerInfo.acceptorUris.emplace_back("local:///one");
    peerInfo.acceptorUris.emplace_back("local:///two");
    peerInfo.capabilities = "";

    ConnectPeer connectPeer{&ioContext, &logger, peerInfo, DOMAIN_SOCKETS_ENABLED};
    connectPeer.SetListener(connectPeerListener);
    connectPeer.AsyncConnect(2, TIMEOUT);

    ioContext.Run();
}


} // namespace
