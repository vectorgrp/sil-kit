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


#include "Uri.hpp"
#include "core/vasio/io/MakeAsioIoContext.hpp"
#include "Filesystem.hpp"
#include "Uuid.hpp"

#include "MockLogger.hpp"

#include "MockAcceptor.hpp"
#include "MockConnector.hpp"
#include "MockRawByteStream.hpp"
#include "MockTimer.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"


namespace {


using namespace SilKit::Core;

using namespace std::chrono_literals;


using ::testing::AnyOf;
using ::testing::Contains;
using ::testing::Sequence;

using SilKit::Services::Logging::MockLogger;
using VSilKit::MockAcceptorListener;
using VSilKit::MockConnectorListener;
using VSilKit::MockRawByteStreamListener;
using VSilKit::MockTimerListener;


struct MockCallbacks
{
    MOCK_METHOD(void, Handle, (int), ());
};


struct Test_IoContext : ::testing::Test
{
    std::string acceptorLocalDomainSocketPath;

    void SetUp() override
    {
        namespace fs = SilKit::Filesystem;

        acceptorLocalDomainSocketPath = fs::temp_directory_path().string() + fs::path::preferred_separator
                                        + to_string(SilKit::Util::Uuid::GenerateRandom()) + ".silkit";
    }

    void TearDown() override
    {
        namespace fs = SilKit::Filesystem;

        try
        {
            fs::remove(acceptorLocalDomainSocketPath);
        }
        catch (...)
        {
        }
    }
};


TEST_F(Test_IoContext, sequential_post_keeps_order)
{
    MockCallbacks callbacks;

    Sequence s1;
    EXPECT_CALL(callbacks, Handle(0)).Times(1).InSequence(s1);
    EXPECT_CALL(callbacks, Handle(1)).Times(1).InSequence(s1);

    auto ioContext = VSilKit::MakeAsioIoContext({});

    ioContext->Post([&callbacks]() {
        callbacks.Handle(0);
    });

    ioContext->Post([&callbacks]() {
        callbacks.Handle(1);
    });

    ioContext->Run();
}

TEST_F(Test_IoContext, nested_post_is_executed_last)
{
    MockCallbacks callbacks;

    Sequence s1;
    EXPECT_CALL(callbacks, Handle(0)).Times(1).InSequence(s1);
    EXPECT_CALL(callbacks, Handle(1)).Times(1).InSequence(s1);
    EXPECT_CALL(callbacks, Handle(2)).Times(1).InSequence(s1);
    EXPECT_CALL(callbacks, Handle(3)).Times(1).InSequence(s1);

    auto ioContext = VSilKit::MakeAsioIoContext({});

    ioContext->Post([&ioContext, &callbacks]() {
        callbacks.Handle(0);
        ioContext->Post([&callbacks]() {
            callbacks.Handle(3);
        });
        callbacks.Handle(1);
    });

    ioContext->Post([&callbacks]() {
        callbacks.Handle(2);
    });

    ioContext->Run();
}

TEST_F(Test_IoContext, sequential_dispatch_keeps_order)
{
    MockCallbacks callbacks;

    Sequence s1;
    EXPECT_CALL(callbacks, Handle(0)).Times(1).InSequence(s1);
    EXPECT_CALL(callbacks, Handle(1)).Times(1).InSequence(s1);

    auto ioContext = VSilKit::MakeAsioIoContext({});

    ioContext->Dispatch([&callbacks]() {
        callbacks.Handle(0);
    });

    ioContext->Dispatch([&callbacks]() {
        callbacks.Handle(1);
    });

    ioContext->Run();
}

TEST_F(Test_IoContext, nested_dispatch_is_executed_immediately)
{
    MockCallbacks callbacks;

    Sequence s1;
    EXPECT_CALL(callbacks, Handle(0)).Times(1).InSequence(s1);
    EXPECT_CALL(callbacks, Handle(1)).Times(1).InSequence(s1);
    EXPECT_CALL(callbacks, Handle(2)).Times(1).InSequence(s1);
    EXPECT_CALL(callbacks, Handle(3)).Times(1).InSequence(s1);

    auto ioContext = VSilKit::MakeAsioIoContext({});

    ioContext->Dispatch([&ioContext, &callbacks]() {
        callbacks.Handle(0);
        ioContext->Dispatch([&callbacks]() {
            callbacks.Handle(1);
        });
        callbacks.Handle(2);
    });

    ioContext->Dispatch([&callbacks]() {
        callbacks.Handle(3);
    });

    ioContext->Run();
}

TEST_F(Test_IoContext, resolve)
{
    auto ioContext = VSilKit::MakeAsioIoContext({});

    auto resolveStrings = ioContext->Resolve("localhost");

    // Some Linux distributions only map localhost to 127.0.0.1 (IPv4), not ::1 (IPv6)
    EXPECT_THAT(resolveStrings, AnyOf(Contains("127.0.0.1"), Contains("::1")));
}

TEST_F(Test_IoContext, timers_execute_and_wait)
{
    auto ioContext = VSilKit::MakeAsioIoContext({});

    auto timer1 = ioContext->MakeTimer();
    auto timer2 = ioContext->MakeTimer();

    MockTimerListener listener1;
    MockTimerListener listener2;

    std::chrono::steady_clock::time_point beforeNestedWait, nestedWaitExpired;

    Sequence s1;

    // keep the time when the second timer is triggered from the first
    EXPECT_CALL(listener1, OnTimerExpired).InSequence(s1).WillOnce([&beforeNestedWait, &timer2] {
        beforeNestedWait = std::chrono::steady_clock::now();
        timer2->AsyncWaitFor(20ms);
    });

    // keep the time when the second timer completes
    EXPECT_CALL(listener2, OnTimerExpired).InSequence(s1).WillOnce([&nestedWaitExpired] {
        nestedWaitExpired = std::chrono::steady_clock::now();
    });

    timer1->SetListener(listener1);
    timer2->SetListener(listener2);

    timer1->AsyncWaitFor(10ms);

    ioContext->Run();

    // check that the timer waited for at least the specified amount
    EXPECT_GE(nestedWaitExpired - beforeNestedWait, 20ms);
}

TEST_F(Test_IoContext, timer_expires_with_zero_wait_duration)
{
    MockTimerListener listener1;

    EXPECT_CALL(listener1, OnTimerExpired).Times(1);

    auto ioContext = VSilKit::MakeAsioIoContext({});

    auto timer1 = ioContext->MakeTimer();
    timer1->SetListener(listener1);
    timer1->AsyncWaitFor(0ms);

    ioContext->Run();
}

TEST_F(Test_IoContext, tcp_acceptor_timeout)
{
    MockAcceptorListener listener;
    EXPECT_CALL(listener, OnAsyncAcceptSuccess).Times(0);
    EXPECT_CALL(listener, OnAsyncAcceptFailure).Times(1);

    auto ioContext{VSilKit::MakeAsioIoContext({})};

    auto acceptor{ioContext->MakeTcpAcceptor("127.0.0.1", 0)};
    acceptor->SetListener(listener);
    acceptor->AsyncAccept(50ms);

    ioContext->Run();
}

TEST_F(Test_IoContext, local_domain_acceptor_timeout)
{
    MockAcceptorListener listener;
    EXPECT_CALL(listener, OnAsyncAcceptSuccess).Times(0);
    EXPECT_CALL(listener, OnAsyncAcceptFailure).Times(1);

    auto ioContext{VSilKit::MakeAsioIoContext({})};

    auto acceptor{ioContext->MakeLocalAcceptor(acceptorLocalDomainSocketPath)};
    acceptor->SetListener(listener);
    acceptor->AsyncAccept(50ms);

    ioContext->Run();
}

struct Test_IoContext_AcceptorConnector_PingPong : Test_IoContext
{
    Sequence sa, sc;

    MockCallbacks callbacks;
    MockLogger logger;
    MockAcceptorListener acceptorListener;
    MockConnectorListener connectorListener;

    struct
    {
        std::unique_ptr<VSilKit::IRawByteStream> stream;
        MockRawByteStreamListener listener;
        uint8_t readByte{0xFF}; // will be overwritten in async read handler
        uint8_t writeByte{22};
    } accepted;

    struct
    {
        std::unique_ptr<VSilKit::IRawByteStream> stream;
        MockRawByteStreamListener listener;
        uint8_t readByte{0xFF};
        uint8_t writeByte{11};
    } connected;

    void SetupExpectations()
    {
        // expectations on sequencing callbacks

        EXPECT_CALL(callbacks, Handle(10)).InSequence(sa); // from accept handler on acceptor
        EXPECT_CALL(callbacks, Handle(11)).InSequence(sa); // from read handler on accepted stream
        EXPECT_CALL(callbacks, Handle(12)).InSequence(sa); // from write handler on accepted stream

        EXPECT_CALL(callbacks, Handle(20)).InSequence(sc); // from connect handler on connector
        EXPECT_CALL(callbacks, Handle(21)).InSequence(sc); // from write handler of connected stream
        EXPECT_CALL(callbacks, Handle(22)).InSequence(sc); // from read handler of connected stream

        // expectations and behavior of the acceptor

        EXPECT_CALL(acceptorListener, OnAsyncAcceptSuccess).WillOnce([this](auto&, auto stream) {
            callbacks.Handle(10);

            accepted.stream = std::move(stream);
            accepted.stream->SetListener(accepted.listener);

            MutableBuffer readBuffer{&accepted.readByte, 1};
            accepted.stream->AsyncReadSome(MutableBufferSequence{&readBuffer, 1});
        });

        EXPECT_CALL(acceptorListener, OnAsyncAcceptFailure).Times(0);

        // expectations and behavior of the connector

        EXPECT_CALL(connectorListener, OnAsyncConnectSuccess).WillOnce([this](auto&, auto stream) {
            callbacks.Handle(20);
            connected.stream = std::move(stream);
            connected.stream->SetListener(connected.listener);

            ConstBuffer writeBuffer{&connected.writeByte, 1};
            connected.stream->AsyncWriteSome(ConstBufferSequence{&writeBuffer, 1});
        });

        EXPECT_CALL(connectorListener, OnAsyncConnectFailure).Times(0);

        // expectations and behavior of the accepted stream

        EXPECT_CALL(accepted.listener, OnAsyncReadSomeDone).WillOnce([this] {
            callbacks.Handle(static_cast<int>(accepted.readByte));

            ConstBuffer writeBuffer{&accepted.writeByte, 1};
            accepted.stream->AsyncWriteSome(ConstBufferSequence{&writeBuffer, 1});
        });

        EXPECT_CALL(accepted.listener, OnAsyncWriteSomeDone).WillOnce([this] {
            callbacks.Handle(12);
        });

        // expectations and behavior of the connected stream

        EXPECT_CALL(connected.listener, OnAsyncReadSomeDone).WillOnce([this] {
            callbacks.Handle(static_cast<int>(connected.readByte));
        });

        EXPECT_CALL(connected.listener, OnAsyncWriteSomeDone).WillOnce([this] {
            callbacks.Handle(21);

            MutableBuffer readBuffer{&connected.readByte, 1};
            connected.stream->AsyncReadSome(MutableBufferSequence{&readBuffer, 1});
        });
    }
};

TEST_F(Test_IoContext_AcceptorConnector_PingPong, tcp)
{
    SetupExpectations();

    auto ioContext = VSilKit::MakeAsioIoContext({});
    ioContext->SetLogger(logger);

    auto acceptor = ioContext->MakeTcpAcceptor("127.0.0.1", 0);
    acceptor->SetListener(acceptorListener);
    acceptor->AsyncAccept(5000ms);

    auto endpoint = acceptor->GetLocalEndpoint();
    auto uri = Uri::Parse(endpoint);

    ASSERT_EQ(uri.Type(), Uri::UriType::Tcp);

    auto connector = ioContext->MakeTcpConnector(uri.Host(), uri.Port());
    connector->SetListener(connectorListener);
    connector->AsyncConnect(0ms);

    ioContext->Run();
}

TEST_F(Test_IoContext_AcceptorConnector_PingPong, tcp_connect_send_buffer_size)
{
    SetupExpectations();

    AsioSocketOptions asioSocketOptions{};
    asioSocketOptions.tcp.sendBufferSize = 1024;

    auto ioContext = VSilKit::MakeAsioIoContext(asioSocketOptions);
    ioContext->SetLogger(logger);

    auto acceptor = ioContext->MakeTcpAcceptor("127.0.0.1", 0);
    acceptor->SetListener(acceptorListener);
    acceptor->AsyncAccept(5000ms);

    auto endpoint = acceptor->GetLocalEndpoint();
    auto uri = Uri::Parse(endpoint);

    ASSERT_EQ(uri.Type(), Uri::UriType::Tcp);

    auto connector = ioContext->MakeTcpConnector(uri.Host(), uri.Port());
    connector->SetListener(connectorListener);
    connector->AsyncConnect(0ms);

    ioContext->Run();
}

TEST_F(Test_IoContext_AcceptorConnector_PingPong, local_domain)
{
    SetupExpectations();

    auto ioContext = VSilKit::MakeAsioIoContext({});
    ioContext->SetLogger(logger);

    auto acceptor = ioContext->MakeLocalAcceptor(acceptorLocalDomainSocketPath);
    acceptor->SetListener(acceptorListener);
    acceptor->AsyncAccept(5000ms);

    auto endpoint = acceptor->GetLocalEndpoint();
    auto uri = Uri::Parse(endpoint);

    ASSERT_EQ(uri.Type(), Uri::UriType::Local);

    auto connector = ioContext->MakeLocalConnector(uri.Path());
    connector->SetListener(connectorListener);
    connector->AsyncConnect(0ms);

    ioContext->Run();
}

} // namespace