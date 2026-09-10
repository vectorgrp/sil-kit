// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>

#include "dashboard/http/HttpRetryPolicy.hpp"
#include "dashboard/http/IHttpClient.hpp"

namespace VSilKit {

/*! Adds retries on top of another IHttpClient.
 *
 *  Keeping the policy in a decorator leaves the transport free of policy and makes the retry logic
 *  testable without sockets.
 *
 *  Unlike the oatpp policy it replaces, the backoff wait is interruptible: Abort() wakes it
 *  immediately instead of letting the 300 ms elapse, so shutdown is not delayed.
 */
class RetryingHttpClient final : public IHttpClient
{
public:
    RetryingHttpClient(std::shared_ptr<IHttpClient> inner, HttpRetryPolicy policy = {});

    auto Post(const std::string& path, const std::string& jsonBody) -> HttpResult override;
    void Reset() override;
    void Abort() override;

private:
    //! Returns false if the wait was cut short by Abort().
    auto SleepInterruptible(std::chrono::milliseconds duration) -> bool;

    std::shared_ptr<IHttpClient> _inner;
    HttpRetryPolicy _policy;

    std::mutex _mutex;
    std::condition_variable _abortCv;
    std::atomic<bool> _aborted{false};
};

} // namespace VSilKit
