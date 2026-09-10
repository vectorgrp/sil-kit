// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "dashboard/http/RetryingHttpClient.hpp"

#include <utility>

namespace VSilKit {

RetryingHttpClient::RetryingHttpClient(std::shared_ptr<IHttpClient> inner, HttpRetryPolicy policy)
    : _inner(std::move(inner))
    , _policy(policy)
{
}

auto RetryingHttpClient::Post(const std::string& path, const std::string& jsonBody) -> HttpResult
{
    for (std::size_t attempt = 1;; ++attempt)
    {
        if (_aborted.load(std::memory_order_acquire))
        {
            return HttpResult{};
        }

        auto result = _inner->Post(path, jsonBody);

        const bool mayRetry = !_aborted.load(std::memory_order_acquire) && attempt < _policy.maxAttempts;

        if (!result.transportError)
        {
            // A retryable status on the final attempt is handed back to the caller, matching oatpp.
            if (!_policy.ShouldRetry(result.statusCode) || !mayRetry)
            {
                return result;
            }
        }
        else if (!mayRetry)
        {
            return HttpResult{};
        }

        _inner->Reset();
        if (!SleepInterruptible(_policy.backoff))
        {
            return HttpResult{};
        }
    }
}

void RetryingHttpClient::Reset()
{
    _inner->Reset();
}

void RetryingHttpClient::Abort()
{
    {
        std::lock_guard<std::mutex> lock{_mutex};
        _aborted.store(true, std::memory_order_release);
    }
    _abortCv.notify_all();
    _inner->Abort();
}

auto RetryingHttpClient::SleepInterruptible(std::chrono::milliseconds duration) -> bool
{
    std::unique_lock<std::mutex> lock{_mutex};
    const bool aborted = _abortCv.wait_for(lock, duration, [this] { return _aborted.load(std::memory_order_acquire); });
    return !aborted;
}

} // namespace VSilKit
