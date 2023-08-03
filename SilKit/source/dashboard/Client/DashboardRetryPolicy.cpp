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
