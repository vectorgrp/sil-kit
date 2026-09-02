// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

/*! Shutdown behaviour of the fully assembled dashboard REST client.
 *
 *  These exercise the real production wiring - DashboardRestClient over DashboardSystemServiceClient
 *  over RetryingHttpClient over AsioHttpClient - against a loopback server, rather than mocks.
 *
 *  The case that matters is a dashboard server that accepts the connection and then never answers.
 *  Before this rework such a request blocked forever: oatpp set no socket timeouts, and the abort
 *  hook the destructor called ran only after the worker thread had already been joined, so the
 *  registry hung on shutdown.
 *
 *  Nothing here measures wall-clock time or sleeps to synchronise. The read deadline is set far
 *  higher than these tests could ever legitimately take, so a request that returns at all can only
 *  have been ended by Abort(); if Abort() stops working the test blocks and the harness timeout
 *  reports it, which is a verdict that does not change with how loaded the machine is. Threads hand
 *  over through a Latch tied to the server actually receiving the request, so the client is
 *  guaranteed to be blocked in the read before the abort fires.
 */

#include <chrono>
#include <string>
#include <thread>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "core/mock/participant/MockParticipant.hpp"

#include "dashboard/http/FakeHttpServer.hpp"
#include "dashboard/service/DashboardRestClient.hpp"

using namespace testing;
using VSilKit::Tests::FakeHttpServer;
using VSilKit::Tests::Latch;

namespace SilKit {
namespace Dashboard {
namespace {

class Test_DashboardShutdown : public Test
{
public:
    void SetUp() override
    {
        EXPECT_CALL(_dummyLogger, GetLogLevel).WillRepeatedly(Return(Services::Logging::Level::Off));
    }

    /*! A client whose deadlines cannot plausibly fire during a test.
     *
     *  An hour-long read deadline means "the request came back" is only ever attributable to
     *  Abort(), never to a timeout that happened to elapse on a slow worker. The zero backoff keeps
     *  the retry path from spending real time.
     */
    auto CreateClient(uint16_t port, std::size_t maxAttempts = 3) -> std::shared_ptr<DashboardRestClient>
    {
        VSilKit::AsioHttpClientTimeouts timeouts{};
        timeouts.connect = std::chrono::hours{1};
        timeouts.write = std::chrono::hours{1};
        timeouts.read = std::chrono::hours{1};

        VSilKit::HttpRetryPolicy retryPolicy{};
        retryPolicy.maxAttempts = maxAttempts;
        retryPolicy.backoff = std::chrono::milliseconds{0};

        return std::make_shared<DashboardRestClient>(&_dummyLogger, "http://127.0.0.1:" + std::to_string(port),
                                                     timeouts, retryPolicy);
    }

    /*! A client pointed at a port nothing listens on, with a connect deadline that will not fire.
     *
     *  One attempt only: this is about a refused connection being reported, not about retrying, and
     *  some platforms take seconds to refuse a loopback connection.
     */
    auto CreateClientWithNoServer() -> std::shared_ptr<DashboardRestClient>
    {
        return CreateClient(1, 1); // port 1 is reserved and never has a listener
    }

    NiceMock<Core::Tests::MockLogger> _dummyLogger;
};

/*! A silent server plus an abort is the shutdown hang, reproduced.
 *
 *  The handler signals once the request has arrived, which is the moment the client is committed to
 *  reading a response that will never come. Only Abort() can end that read.
 */
TEST_F(Test_DashboardShutdown, OnSimulationStart_AgainstASilentServer_IsEndedByAbort)
{
    Latch requestReceived;
    FakeHttpServer server{[&requestReceived](const std::string&) {
        requestReceived.Signal();
        return std::string{}; // go silent
    }};

    const auto client = CreateClient(server.Port());

    std::thread aborter{[&] {
        requestReceived.Wait();
        client->Abort();
    }};

    const auto simulationId = client->OnSimulationStart("silkit://localhost:8500", 0);
    aborter.join();

    EXPECT_EQ(simulationId, 0u);
    EXPECT_EQ(server.Requests().size(), 1u) << "the abort must not have triggered a retry";
}

//! A refused connection is reported, rather than retried until the connect deadline expires.
TEST_F(Test_DashboardShutdown, OnSimulationStart_WithNothingListening_ReportsFailure)
{
    const auto client = CreateClientWithNoServer();

    EXPECT_EQ(client->OnSimulationStart("silkit://localhost:8500", 0), 0u);
}

TEST_F(Test_DashboardShutdown, Abort_IsIdempotentAndSafeBeforeAnyRequest)
{
    FakeHttpServer server{FakeHttpServer::Always(FakeHttpServer::MakeReply(201, R"({"id":1})"))};

    const auto client = CreateClient(server.Port());

    client->Abort();
    client->Abort();

    // An aborted client stays aborted, so this fails fast instead of reaching a server that would
    // have answered - which is what makes the assertion on AcceptCount meaningful.
    EXPECT_EQ(client->OnSimulationStart("silkit://localhost:8500", 0), 0u);
    EXPECT_EQ(server.AcceptCount(), 0);
}

//! Destroying a client whose request was aborted must not block on the dead request.
TEST_F(Test_DashboardShutdown, Destruction_AfterAnAbortedRequest_Completes)
{
    Latch requestReceived;
    FakeHttpServer server{[&requestReceived](const std::string&) {
        requestReceived.Signal();
        return std::string{};
    }};

    {
        const auto client = CreateClient(server.Port());

        std::thread aborter{[&] {
            requestReceived.Wait();
            client->Abort();
        }};

        client->OnSimulationStart("silkit://localhost:8500", 0);
        aborter.join();
    }

    SUCCEED() << "the client was destroyed without blocking";
}

// The happy path over the real stack, so the shutdown cases above are not the only coverage of the
// assembled client.
TEST_F(Test_DashboardShutdown, OnSimulationStart_AgainstARespondingServer_ReturnsTheSimulationId)
{
    FakeHttpServer server{FakeHttpServer::Always(FakeHttpServer::MakeReply(201, R"({"id":4711})"))};

    const auto client = CreateClient(server.Port());
    const auto simulationId = client->OnSimulationStart("silkit://localhost:8500", 12345);

    EXPECT_EQ(simulationId, 4711u);

    const auto requests = server.Requests();
    ASSERT_EQ(requests.size(), 1u);
    EXPECT_NE(requests[0].find("POST /system-service/v1.0/simulations HTTP/1.1"), std::string::npos);
    EXPECT_NE(requests[0].find(R"({"started": 12345,"configuration": {"connectUri": "silkit://localhost:8500"}})"),
              std::string::npos);
}

} // namespace
} // namespace Dashboard
} // namespace SilKit
