// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <atomic>

#include "OatppHeaders.hpp"

namespace SilKit {
namespace Dashboard {
class DashboardRetryPolicy : public oatpp::web::client::RetryPolicy
{
public:
    DashboardRetryPolicy(std::size_t retryCount = InfiniteRetries);

public:
    constexpr static std::size_t InfiniteRetries{0};

    std::chrono::milliseconds _defaultSleep{300};
    std::size_t _retryCount{0};
    std::atomic<bool> _abortRetries{false};

    void AbortAllRetries();
    bool canRetry(const Context& context) override;
    bool retryOnResponse(v_int32 responseStatusCode, const Context& context) override;
    v_int64 waitForMicroseconds(const Context& context) override;
};
} // namespace Dashboard
} // namespace SilKit
