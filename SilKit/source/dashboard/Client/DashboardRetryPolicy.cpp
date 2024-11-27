// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "DashboardRetryPolicy.hpp"

#include "silkit/SilKitMacros.hpp"

namespace SilKit {
namespace Dashboard {

DashboardRetryPolicy::DashboardRetryPolicy(std::size_t retryCount)
    : _retryCount(retryCount)
{
}

void DashboardRetryPolicy::AbortAllRetries()
{
    _abortRetries = true;
}

bool DashboardRetryPolicy::canRetry(const Context& context)
{
    if (_abortRetries)
        return false;
    if (_retryCount == InfiniteRetries)
        return true;
    return context.attempt < static_cast<int64_t>(_retryCount);
}

bool DashboardRetryPolicy::retryOnResponse(v_int32 responseStatusCode, const Context& context)
{
    SILKIT_UNUSED_ARG(context);
    if (_abortRetries)
        return false;
    if (responseStatusCode == 503)
        return true;
    return false;
}

v_int64 DashboardRetryPolicy::waitForMicroseconds(const Context& context)
{
    SILKIT_UNUSED_ARG(context);
    if (_abortRetries)
        return 0;
    return std::chrono::duration_cast<std::chrono::microseconds>(_defaultSleep).count();
}

} // namespace Dashboard
} // namespace SilKit
