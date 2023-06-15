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


#include "TimeProvider.hpp"

#include <mutex>

using namespace std::chrono_literals;
namespace SilKit {
namespace Services {
namespace Orchestration {
// Actual Provider Implementations
namespace detail {

class ProviderBase : public ITimeProviderInternal
{
public:
    ProviderBase(std::string name)
        : _name{std::move(name)}
    {
    }

    auto Now() const -> std::chrono::nanoseconds override
    {
        return _now;
    }

    auto TimeProviderName() const -> const std::string& override
    {
        return _name;
    }

    HandlerId AddNextSimStepHandler(NextSimStepHandler nextSimStepHandler) override
    {
        return _handlers.Add(std::move(nextSimStepHandler));
    }

    void RemoveNextSimStepHandler(HandlerId handlerId) override
    {
        _handlers.Remove(handlerId);
    }

    void SetTime(std::chrono::nanoseconds now, std::chrono::nanoseconds ) override
    {
        _now = now;
    }

    void ConfigureTimeProvider(Orchestration::TimeProviderKind ) override
    {
        // No Op
    }

    void SetSynchronizeVirtualTime(bool isSynchronizingVirtualTime) override
    {
        _isSynchronizingVirtualTime = isSynchronizingVirtualTime;
    }

    bool IsSynchronizingVirtualTime() const override
    {
        return _isSynchronizingVirtualTime;
    }

public:
    auto MutableNextSimStepHandlers() -> Util::SynchronizedHandlers<NextSimStepHandler> & override
    {
        return _handlers;
    }

protected:
    std::chrono::nanoseconds _now{};
    std::string _name;
    bool _isSynchronizingVirtualTime{false};
    Util::SynchronizedHandlers<NextSimStepHandler> _handlers;
};

class WallclockProvider : public ProviderBase
{
public:
    WallclockProvider(std::chrono::nanoseconds tickPeriod)
        :ProviderBase("WallclockProvider")
        , _tickPeriod{tickPeriod}
    {
    }

    // Wall clock provider
    HandlerId AddNextSimStepHandler(NextSimStepHandler simStepHandler) override
    {
        const auto handlerId = ProviderBase::AddNextSimStepHandler(std::move(simStepHandler));

        _timer.WithPeriod(_tickPeriod, [this](const auto& now) {
            _handlers.InvokeAll(now, _tickPeriod);
        });

        return handlerId;
    }

    auto Now() const -> std::chrono::nanoseconds override
    {
        const auto now = std::chrono::high_resolution_clock::now().time_since_epoch();
        return now;
    }

private:
    std::chrono::nanoseconds _tickPeriod{0};
    Util::Timer _timer;
};


class NoSyncProvider : public ProviderBase
{
public:
    NoSyncProvider()
        : ProviderBase("NoSyncProvider")
    {
    }

    // From Wall clock provider
    HandlerId AddNextSimStepHandler(NextSimStepHandler simStepHandler) override
    {
        const auto handlerId = ProviderBase::AddNextSimStepHandler(std::move(simStepHandler));

        _timer.WithPeriod(_tickPeriod, [this](const auto& now) {
            _handlers.InvokeAll(now, _tickPeriod);
        });

        return handlerId;
    }

    auto Now() const -> std::chrono::nanoseconds override
    {
        // always return std::chrono::nanoseconds::min
        return std::chrono::nanoseconds::duration::min();
    }


private:
    std::chrono::nanoseconds _tickPeriod{100000};
    Util::Timer _timer;
};



//! \brief  A caching time provider: we update its internal state whenever the controller's
//          simulation time changes.
// This ensures that the our time provider is available even after
// the TimeSyncService gets destructed.
class SynchronizedVirtualTimeProvider : public ProviderBase
{
public:
    SynchronizedVirtualTimeProvider()
        : ProviderBase("SynchronizedVirtualTimeProvider")
    {
    }

    void SetTime(std::chrono::nanoseconds now, std::chrono::nanoseconds duration) override
    {
        _now = now;
        // tell our users about the next simulation step
        _handlers.InvokeAll(now, duration);
    }
};

} // namespace detail

//TimeProvider
TimeProvider::TimeProvider()
    :_currentProvider{std::make_unique<detail::NoSyncProvider>()}
{
}

void TimeProvider::ConfigureTimeProvider(Orchestration::TimeProviderKind timeProviderKind)
{
    const auto isSynchronizingVirtualTime = _currentProvider->IsSynchronizingVirtualTime();

    std::unique_lock<decltype(_mutex)> lock{_mutex};

    std::unique_ptr<detail::ITimeProviderInternal> providerPtr;

    switch (timeProviderKind)
    {
    case Orchestration::TimeProviderKind::NoSync:
        providerPtr = std::make_unique<detail::NoSyncProvider>();
        break;
    case Orchestration::TimeProviderKind::WallClock:
        providerPtr = std::make_unique<detail::WallclockProvider>(1ms);
        break;
    case Orchestration::TimeProviderKind::SyncTime:
        providerPtr = std::make_unique<detail::SynchronizedVirtualTimeProvider>();
        break;
    default: break;
    }

    if (providerPtr)
    {
        using std::swap;

        // swap the newly created provider with the current provider
        swap(_currentProvider, providerPtr);

        // swap the NextSimStepHandler's of the current provider and the last provider
        swap(_currentProvider->MutableNextSimStepHandlers(), providerPtr->MutableNextSimStepHandlers());
    }

    _currentProvider->SetSynchronizeVirtualTime(isSynchronizingVirtualTime);
}

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
