// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <chrono>
#include <cstddef>

namespace VSilKit {

/*! Retry behaviour for dashboard requests.
 *
 *  The defaults reproduce the oatpp RetryPolicy that this replaces (see
 *  oatpp/web/client/RequestExecutor.cpp): at most three attempts, a fixed 300 ms backoff, and
 *  retries only on HTTP 503 or on a transport failure. Note that a 503 on the *last* attempt is
 *  returned to the caller rather than turned into a transport error - oatpp behaved the same way.
 */
struct HttpRetryPolicy
{
    std::size_t maxAttempts{3};
    std::chrono::milliseconds backoff{300};

    auto ShouldRetry(int statusCode) const -> bool
    {
        return statusCode == 503;
    }
};

} // namespace VSilKit
