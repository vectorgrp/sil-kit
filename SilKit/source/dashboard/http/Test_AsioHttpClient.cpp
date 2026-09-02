// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "dashboard/http/AsioHttpClient.hpp"

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "dashboard/http/FakeHttpServer.hpp"

#include "gtest/gtest.h"

#include "dashboard/http/RetryingHttpClient.hpp"

namespace VSilKit {
namespace {

using namespace std::chrono_literals;

/* No test here measures wall-clock time. Where a deadline is the feature under test the deadline is
 * set short and only its effect is asserted; where a deadline must NOT be what ends the request it
 * is set out of reach, so a request that returns can only have been aborted. Threads hand over
 * through a Latch, never a sleep. */

using VSilKit::Tests::FakeHttpServer;
using VSilKit::Tests::Latch;

auto Reply(int statusCode, const std::string& body, const std::string& extraHeaders = {}) -> std::string
{
    return FakeHttpServer::MakeReply(statusCode, body, extraHeaders);
}

auto AlwaysReply(std::string reply) -> FakeHttpServer::Handler
{
    return FakeHttpServer::Always(std::move(reply));
}

class Test_AsioHttpClient : public testing::Test
{
public:
    static auto FastTimeouts() -> AsioHttpClientTimeouts
    {
        AsioHttpClientTimeouts timeouts{};
        timeouts.connect = 1s;
        timeouts.write = 1s;
        timeouts.read = 150ms;
        return timeouts;
    }
};

TEST_F(Test_AsioHttpClient, Post_SendsExactlyTheExpectedRequestBytes)
{
    FakeHttpServer server{AlwaysReply(Reply(201, R"({"id":42})"))};
    AsioHttpClient client{nullptr, "127.0.0.1", server.Port()};

    const auto result = client.Post("system-service/v1.0/simulations", R"({"started":1})");

    ASSERT_FALSE(result.transportError);
    EXPECT_EQ(result.statusCode, 201);
    EXPECT_EQ(result.body, R"({"id":42})");

    const auto requests = server.Requests();
    ASSERT_EQ(requests.size(), 1u);
    const std::string expected = "POST /system-service/v1.0/simulations HTTP/1.1\r\n"
                                 "Host: 127.0.0.1:"
                                 + std::to_string(server.Port())
                                 + "\r\n"
                                   "Connection: keep-alive\r\n"
                                   "Content-Type: application/json\r\n"
                                   "Content-Length: 13\r\n"
                                   "\r\n"
                                   R"({"started":1})";
    EXPECT_EQ(requests[0], expected);
}

TEST_F(Test_AsioHttpClient, Post_ReusesASingleConnection)
{
    FakeHttpServer server{AlwaysReply(Reply(200, "{}"))};
    AsioHttpClient client{nullptr, "127.0.0.1", server.Port()};

    EXPECT_FALSE(client.Post("a", "{}").transportError);
    EXPECT_FALSE(client.Post("b", "{}").transportError);

    EXPECT_EQ(server.AcceptCount(), 1) << "the keep-alive connection should be reused";
    EXPECT_EQ(server.Requests().size(), 2u);
}

TEST_F(Test_AsioHttpClient, Post_ReconnectsWhenTheServerClosesTheConnection)
{
    FakeHttpServer server{AlwaysReply(Reply(200, "{}", "Connection: close\r\n")), true};
    AsioHttpClient client{nullptr, "127.0.0.1", server.Port()};

    EXPECT_FALSE(client.Post("a", "{}").transportError);
    EXPECT_FALSE(client.Post("b", "{}").transportError);

    EXPECT_EQ(server.AcceptCount(), 2);
}

// A response with no body must still leave the connection usable, or the two status-only endpoints
// would force a reconnect on every bulk update.
TEST_F(Test_AsioHttpClient, Post_HandlesABodylessResponseAndKeepsTheConnection)
{
    FakeHttpServer server{AlwaysReply("HTTP/1.1 204 No Content\r\n\r\n")};
    AsioHttpClient client{nullptr, "127.0.0.1", server.Port()};

    const auto first = client.Post("a", "{}");
    const auto second = client.Post("b", "{}");

    ASSERT_FALSE(first.transportError);
    EXPECT_EQ(first.statusCode, 204);
    ASSERT_FALSE(second.transportError);
    EXPECT_EQ(second.statusCode, 204);
    EXPECT_EQ(server.AcceptCount(), 1);
}

TEST_F(Test_AsioHttpClient, Post_ReassemblesAChunkedResponseBody)
{
    const std::string chunked = std::string{"HTTP/1.1 201 Created\r\nTransfer-Encoding: chunked\r\n\r\n"}
                                + "5\r\n" + R"({"id")" + "\r\n" + "4\r\n" + R"(:42})" + "\r\n" + "0\r\n\r\n";
    FakeHttpServer server{AlwaysReply(chunked)};
    AsioHttpClient client{nullptr, "127.0.0.1", server.Port()};

    const auto result = client.Post("a", "{}");

    ASSERT_FALSE(result.transportError);
    EXPECT_EQ(result.statusCode, 201);
    EXPECT_EQ(result.body, R"({"id":42})");
}

TEST_F(Test_AsioHttpClient, Post_SkipsAnInterimResponse)
{
    FakeHttpServer server{AlwaysReply(std::string{"HTTP/1.1 100 Continue\r\n\r\n"} + Reply(201, R"({"id":7})"))};
    AsioHttpClient client{nullptr, "127.0.0.1", server.Port()};

    const auto result = client.Post("a", "{}");

    ASSERT_FALSE(result.transportError);
    EXPECT_EQ(result.statusCode, 201);
    EXPECT_EQ(result.body, R"({"id":7})");
}

TEST_F(Test_AsioHttpClient, Post_ReportsATransportErrorForAMalformedResponse)
{
    FakeHttpServer server{AlwaysReply("this is not an HTTP response\r\n\r\n")};
    AsioHttpClient client{nullptr, "127.0.0.1", server.Port(), FastTimeouts()};

    EXPECT_TRUE(client.Post("a", "{}").transportError);
}

TEST_F(Test_AsioHttpClient, Post_ReportsATransportErrorWhenNothingIsListening)
{
    // Port 1 is reserved and never has a listener. The connect deadline is out of reach, so a
    // refused connection is the only thing that can end this call.
    AsioHttpClientTimeouts unreachableDeadlines{};
    unreachableDeadlines.connect = 1h;
    AsioHttpClient client{nullptr, "127.0.0.1", 1, unreachableDeadlines};

    EXPECT_TRUE(client.Post("a", "{}").transportError);
}

/*! oatpp set no socket timeouts at all, so this case used to hang forever.
 *
 *  Here the read deadline is the feature under test, so it is deliberately short. The assertion is
 *  on its effect - the request came back and reported failure - not on how long it took.
 */
TEST_F(Test_AsioHttpClient, Post_TimesOutWhenTheServerNeverAnswers)
{
    FakeHttpServer server{AlwaysReply("")};
    AsioHttpClient client{nullptr, "127.0.0.1", server.Port(), FastTimeouts()};

    EXPECT_TRUE(client.Post("a", "{}").transportError) << "the read deadline should have fired";
}

/*! Abort() cancels a read that nothing else could end.
 *
 *  The deadline is out of reach and the server never answers, so if Abort() fails to cancel the
 *  pending read this blocks and the harness timeout reports it - a verdict that does not depend on
 *  how fast the machine is. The handler signals once the request has arrived, which is the moment
 *  the client is committed to the read.
 */
TEST_F(Test_AsioHttpClient, Abort_UnblocksAnInFlightRequest)
{
    Latch requestReceived;
    FakeHttpServer server{[&requestReceived](const std::string&) {
        requestReceived.Signal();
        return std::string{};
    }};

    AsioHttpClientTimeouts unreachableDeadlines{};
    unreachableDeadlines.read = 1h;
    AsioHttpClient client{nullptr, "127.0.0.1", server.Port(), unreachableDeadlines};

    std::thread aborter{[&] {
        requestReceived.Wait();
        client.Abort();
    }};

    const auto result = client.Post("a", "{}");
    aborter.join();

    EXPECT_TRUE(result.transportError);
    EXPECT_TRUE(client.Post("a", "{}").transportError) << "an aborted client stays aborted";
}

TEST_F(Test_AsioHttpClient, RetryingHttpClient_OverTheRealTransport_RecoversFromServiceUnavailable)
{
    std::atomic<int> attempts{0};
    FakeHttpServer server{[&attempts](const std::string&) {
        return ++attempts <= 2 ? Reply(503, "") : Reply(200, "{}");
    }};

    auto transport = std::make_shared<AsioHttpClient>(nullptr, "127.0.0.1", server.Port());
    HttpRetryPolicy policy{};
    policy.backoff = 1ms;
    RetryingHttpClient client{transport, policy};

    const auto result = client.Post("a", "{}");

    ASSERT_FALSE(result.transportError);
    EXPECT_EQ(result.statusCode, 200);
    EXPECT_EQ(attempts.load(), 3);
}

} // namespace
} // namespace VSilKit
