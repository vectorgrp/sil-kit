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

// Timing assertions are deliberately loose: CI machines are slow and oversubscribed. Each one only
// has to distinguish "bounded" from "hung", not measure latency.
constexpr auto kGenerousBound = 5s;

using VSilKit::Tests::FakeHttpServer;

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
        timeouts.read = 500ms;
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
    // Port 1 is reserved and never has a listener; the point is that we fail rather than hang.
    AsioHttpClient client{nullptr, "127.0.0.1", 1, FastTimeouts()};

    const auto start = std::chrono::steady_clock::now();
    const auto result = client.Post("a", "{}");
    const auto elapsed = std::chrono::steady_clock::now() - start;

    EXPECT_TRUE(result.transportError);
    EXPECT_LT(elapsed, kGenerousBound);
}

// oatpp set no socket timeouts at all, so this case used to hang forever.
TEST_F(Test_AsioHttpClient, Post_TimesOutWhenTheServerNeverAnswers)
{
    FakeHttpServer server{AlwaysReply("")};
    AsioHttpClient client{nullptr, "127.0.0.1", server.Port(), FastTimeouts()};

    const auto start = std::chrono::steady_clock::now();
    const auto result = client.Post("a", "{}");
    const auto elapsed = std::chrono::steady_clock::now() - start;

    EXPECT_TRUE(result.transportError);
    EXPECT_LT(elapsed, kGenerousBound) << "the read deadline should have fired";
}

TEST_F(Test_AsioHttpClient, Abort_UnblocksAnInFlightRequest)
{
    FakeHttpServer server{AlwaysReply("")};

    AsioHttpClientTimeouts patient{};
    patient.read = 60s; // long enough that only Abort() can end the wait in time
    AsioHttpClient client{nullptr, "127.0.0.1", server.Port(), patient};

    std::thread aborter{[&client] {
        std::this_thread::sleep_for(100ms);
        client.Abort();
    }};

    const auto start = std::chrono::steady_clock::now();
    const auto result = client.Post("a", "{}");
    const auto elapsed = std::chrono::steady_clock::now() - start;
    aborter.join();

    EXPECT_TRUE(result.transportError);
    EXPECT_LT(elapsed, kGenerousBound) << "Abort() must cancel the pending read";
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
