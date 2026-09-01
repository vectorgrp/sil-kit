// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

/*! Shutdown behaviour of the fully assembled dashboard REST client.
 *
 *  These exercise the real production wiring - DashboardRestClient over DashboardSystemServiceClient
 *  over RetryingHttpClient over AsioHttpClient - against a loopback server, rather than mocks.
 *
 *  The case that matters is a dashboard server that accepts the connection and then never answers.
 *  IsBulkUpdateSupported() is the very first thing the registry's dashboard worker thread does, so
 *  before this rework that request blocked forever: oatpp set no socket timeouts, and the abort hook
 *  the destructor called ran only after the worker thread had already been joined. The registry
 *  therefore hung on shutdown. Both halves of the fix are covered here: the request is now bounded
 *  by a read deadline, and Abort() can cut it short from another thread.
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
using namespace std::chrono_literals;
using VSilKit::Tests::FakeHttpServer;

namespace SilKit {
namespace Dashboard {
namespace {

// Loose on purpose: these only need to separate "bounded" from "hung".
constexpr auto kGenerousBound = 60s;

class Test_DashboardShutdown : public Test
{
public:
    void SetUp() override
    {
        EXPECT_CALL(_dummyLogger, GetLogLevel).WillRepeatedly(Return(Services::Logging::Level::Off));
    }

    auto CreateClient(uint16_t port) -> std::shared_ptr<DashboardRestClient>
    {
        return std::make_shared<DashboardRestClient>(&_dummyLogger,
                                                     "http://127.0.0.1:" + std::to_string(port));
    }

    NiceMock<Core::Tests::MockLogger> _dummyLogger;
};

TEST_F(Test_DashboardShutdown, IsBulkUpdateSupported_AgainstASilentServer_CanBeAborted)
{
    FakeHttpServer server{FakeHttpServer::Always("")}; // accepts, never answers

    const auto client = CreateClient(server.Port());

    std::thread aborter{[&client] {
        std::this_thread::sleep_for(200ms);
        client->Abort();
    }};

    const auto start = std::chrono::steady_clock::now();
    const auto supported = client->IsBulkUpdateSupported();
    const auto elapsed = std::chrono::steady_clock::now() - start;
    aborter.join();

    EXPECT_FALSE(supported);
    EXPECT_LT(elapsed, kGenerousBound) << "Abort() must unblock the in-flight probe";
}

TEST_F(Test_DashboardShutdown, IsBulkUpdateSupported_WithNothingListening_FailsWithoutHanging)
{
    // Port 1 is reserved and never has a listener.
    const auto client = CreateClient(1);

    const auto start = std::chrono::steady_clock::now();
    const auto supported = client->IsBulkUpdateSupported();
    const auto elapsed = std::chrono::steady_clock::now() - start;

    EXPECT_FALSE(supported);
    EXPECT_LT(elapsed, kGenerousBound);
}

TEST_F(Test_DashboardShutdown, Abort_IsIdempotentAndSafeBeforeAnyRequest)
{
    FakeHttpServer server{FakeHttpServer::Always(FakeHttpServer::MakeReply(200, "{}"))};

    const auto client = CreateClient(server.Port());

    client->Abort();
    client->Abort();

    // Once aborted the client stays aborted, so the probe fails fast instead of contacting a server
    // that would have answered.
    EXPECT_FALSE(client->IsBulkUpdateSupported());
}

TEST_F(Test_DashboardShutdown, Destruction_AfterAnAbortedRequest_DoesNotBlock)
{
    FakeHttpServer server{FakeHttpServer::Always("")};

    const auto start = std::chrono::steady_clock::now();
    {
        const auto client = CreateClient(server.Port());
        std::thread aborter{[&client] {
            std::this_thread::sleep_for(200ms);
            client->Abort();
        }};
        client->IsBulkUpdateSupported();
        aborter.join();
    }
    const auto elapsed = std::chrono::steady_clock::now() - start;

    EXPECT_LT(elapsed, kGenerousBound);
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
