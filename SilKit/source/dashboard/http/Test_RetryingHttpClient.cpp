// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "dashboard/http/RetryingHttpClient.hpp"

#include <chrono>
#include <memory>
#include <thread>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "dashboard/http/Mocks/MockHttpClient.hpp"

namespace VSilKit {
namespace {

using namespace std::chrono_literals;
using testing::Return;

auto Ok(int statusCode, std::string body = {}) -> HttpResult
{
    return HttpResult{false, statusCode, std::move(body)};
}

auto Unavailable() -> HttpResult
{
    return HttpResult{};
}

class Test_RetryingHttpClient : public testing::Test
{
public:
    void SetUp() override { _inner = std::make_shared<MockHttpClient>(); }

    auto CreateClient(HttpRetryPolicy policy = {}) -> RetryingHttpClient
    {
        return RetryingHttpClient{_inner, policy};
    }

    //! A policy with a negligible backoff, so timing does not dominate the test run.
    static auto FastPolicy() -> HttpRetryPolicy
    {
        HttpRetryPolicy policy{};
        policy.backoff = 1ms;
        return policy;
    }

    std::shared_ptr<MockHttpClient> _inner;
};

TEST_F(Test_RetryingHttpClient, Post_SuccessOnFirstAttempt_CallsInnerOnce)
{
    EXPECT_CALL(*_inner, Post("path", "{}")).WillOnce(Return(Ok(200, "body")));
    EXPECT_CALL(*_inner, Reset()).Times(0);

    auto client = CreateClient(FastPolicy());
    const auto result = client.Post("path", "{}");

    EXPECT_FALSE(result.transportError);
    EXPECT_EQ(result.statusCode, 200);
    EXPECT_EQ(result.body, "body");
}

TEST_F(Test_RetryingHttpClient, Post_NonRetryableStatus_IsReturnedImmediately)
{
    EXPECT_CALL(*_inner, Post("path", "{}")).WillOnce(Return(Ok(500)));

    auto client = CreateClient(FastPolicy());
    const auto result = client.Post("path", "{}");

    EXPECT_FALSE(result.transportError);
    EXPECT_EQ(result.statusCode, 500);
}

TEST_F(Test_RetryingHttpClient, Post_ServiceUnavailableThenSuccess_RetriesAndSucceeds)
{
    EXPECT_CALL(*_inner, Post("path", "{}")).WillOnce(Return(Ok(503))).WillOnce(Return(Ok(200)));
    EXPECT_CALL(*_inner, Reset()).Times(1);

    auto client = CreateClient(FastPolicy());
    const auto result = client.Post("path", "{}");

    EXPECT_FALSE(result.transportError);
    EXPECT_EQ(result.statusCode, 200);
}

// oatpp returned a retryable status from the final attempt to the caller rather than converting it
// into a transport error; that behaviour is preserved deliberately.
TEST_F(Test_RetryingHttpClient, Post_ServiceUnavailableOnEveryAttempt_ReturnsTheStatus)
{
    EXPECT_CALL(*_inner, Post("path", "{}")).Times(3).WillRepeatedly(Return(Ok(503)));
    EXPECT_CALL(*_inner, Reset()).Times(2);

    auto client = CreateClient(FastPolicy());
    const auto result = client.Post("path", "{}");

    EXPECT_FALSE(result.transportError);
    EXPECT_EQ(result.statusCode, 503);
}

TEST_F(Test_RetryingHttpClient, Post_TransportErrorOnEveryAttempt_ReportsTransportError)
{
    EXPECT_CALL(*_inner, Post("path", "{}")).Times(3).WillRepeatedly(Return(Unavailable()));
    EXPECT_CALL(*_inner, Reset()).Times(2);

    auto client = CreateClient(FastPolicy());
    const auto result = client.Post("path", "{}");

    EXPECT_TRUE(result.transportError);
}

TEST_F(Test_RetryingHttpClient, Post_TransportErrorThenSuccess_Recovers)
{
    EXPECT_CALL(*_inner, Post("path", "{}")).WillOnce(Return(Unavailable())).WillOnce(Return(Ok(201, "{}")));
    EXPECT_CALL(*_inner, Reset()).Times(1);

    auto client = CreateClient(FastPolicy());
    const auto result = client.Post("path", "{}");

    EXPECT_FALSE(result.transportError);
    EXPECT_EQ(result.statusCode, 201);
}

TEST_F(Test_RetryingHttpClient, Post_AfterAbort_DoesNotCallInner)
{
    EXPECT_CALL(*_inner, Post(testing::_, testing::_)).Times(0);
    EXPECT_CALL(*_inner, Abort()).Times(1);

    auto client = CreateClient(FastPolicy());
    client.Abort();
    const auto result = client.Post("path", "{}");

    EXPECT_TRUE(result.transportError);
}

TEST_F(Test_RetryingHttpClient, Abort_DuringBackoff_ReturnsWithoutWaitingOutTheBackoff)
{
    // A long backoff: if it were not interruptible, this test would take 30 s.
    HttpRetryPolicy policy{};
    policy.backoff = 30s;

    EXPECT_CALL(*_inner, Post("path", "{}")).WillOnce(Return(Ok(503)));
    EXPECT_CALL(*_inner, Reset()).Times(1);
    EXPECT_CALL(*_inner, Abort()).Times(1);

    auto client = CreateClient(policy);

    std::thread aborter{[&client] {
        std::this_thread::sleep_for(50ms);
        client.Abort();
    }};

    const auto start = std::chrono::steady_clock::now();
    const auto result = client.Post("path", "{}");
    const auto elapsed = std::chrono::steady_clock::now() - start;
    aborter.join();

    EXPECT_TRUE(result.transportError);
    EXPECT_LT(elapsed, 5s) << "Abort() must cut the backoff short";
}

TEST_F(Test_RetryingHttpClient, Post_RespectsAConfiguredAttemptLimit)
{
    HttpRetryPolicy policy = FastPolicy();
    policy.maxAttempts = 5;

    EXPECT_CALL(*_inner, Post("path", "{}")).Times(5).WillRepeatedly(Return(Ok(503)));
    EXPECT_CALL(*_inner, Reset()).Times(4);

    auto client = CreateClient(policy);
    EXPECT_EQ(client.Post("path", "{}").statusCode, 503);
}

TEST_F(Test_RetryingHttpClient, Reset_IsForwardedToInner)
{
    EXPECT_CALL(*_inner, Reset()).Times(1);

    auto client = CreateClient();
    client.Reset();
}

} // namespace
} // namespace VSilKit
