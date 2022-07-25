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

#include <chrono>
#include <string>
#include <memory>
#include <mutex>

#include "ITimeProvider.hpp"
#include "Timer.hpp"
#include "SynchronizedHandlers.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {


class TimeProvider : public ITimeProvider
{
public:
    //CTor
    virtual ~TimeProvider() = default;
    TimeProvider();
public:
    //ITimeProvider
    inline auto Now() const -> std::chrono::nanoseconds override;
    inline auto TimeProviderName() const -> const std::string& override;
    inline HandlerId AddNextSimStepHandler(NextSimStepHandlerT handler) override;
    inline void RemoveNextSimStepHandler(HandlerId handlerId) override;
    inline void SetTime(std::chrono::nanoseconds now, std::chrono::nanoseconds duration) override;
    inline void SetSynchronized(bool isSynchronized) override;
    inline bool IsSynchronized() const override;

    void ConfigureTimeProvider(Orchestration::TimeProviderKind timeProviderKind) override;

private: //Members
    mutable std::mutex _mutex;
    std::unique_ptr<ITimeProvider> _currentProvider;
};

//////////////////////////////////////////////////////////////////////
// Inline Implementations
/////////////////////////////////////////////////////////////////////


auto TimeProvider::Now() const -> std::chrono::nanoseconds
{
    std::unique_lock<decltype(_mutex)> lock{_mutex};
    return _currentProvider->Now();
}

auto TimeProvider::TimeProviderName() const -> const std::string&
{
    std::unique_lock<decltype(_mutex)> lock{_mutex};
    return _currentProvider->TimeProviderName();
}

HandlerId TimeProvider::AddNextSimStepHandler(NextSimStepHandlerT handler)
{
    std::unique_lock<decltype(_mutex)> lock{_mutex};
    return _currentProvider->AddNextSimStepHandler(std::move(handler));
}

void TimeProvider::RemoveNextSimStepHandler(HandlerId handlerId)
{
    std::unique_lock<decltype(_mutex)> lock{_mutex};
    _currentProvider->RemoveNextSimStepHandler(handlerId);
}

void TimeProvider::SetTime(std::chrono::nanoseconds now, std::chrono::nanoseconds duration)
{
    std::unique_lock<decltype(_mutex)> lock{_mutex};
    _currentProvider->SetTime(now, duration);
}

void TimeProvider::SetSynchronized(bool isSynchronized)
{
    std::unique_lock<decltype(_mutex)> lock{_mutex};
    _currentProvider->SetSynchronized(isSynchronized);
}

bool TimeProvider::IsSynchronized() const
{
    std::unique_lock<decltype(_mutex)> lock{_mutex};
    return _currentProvider->IsSynchronized();
}

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
