/* Copyright (c) 2022 Vector Informatik GmbH
 
Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

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
