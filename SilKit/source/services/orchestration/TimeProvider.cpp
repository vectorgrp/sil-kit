
// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "TimeProvider.hpp"

using namespace std::chrono_literals;
namespace SilKit {
namespace Core {
namespace Orchestration {
// Actual Provider Implementations
namespace detail {
struct ProviderBase : ITimeProvider
{
    std::chrono::nanoseconds _now{};
    std::string _name;
    bool _isSynchronized{false};

    ProviderBase(std::string name)
        : _name{std::move(name)}
    {
    }

    virtual auto Now() const -> std::chrono::nanoseconds override
    {
        return _now;
    }
    virtual auto TimeProviderName() const -> const std::string& override
    {
        return _name;
    }
    virtual HandlerId AddNextSimStepHandler(NextSimStepHandlerT ) override
    {
        // No Op
        return {};
    }
    virtual void RemoveNextSimStepHandler(HandlerId ) override
    {
        // No Op
    }
    virtual void SetTime(std::chrono::nanoseconds now, std::chrono::nanoseconds ) override
    {
        _now = now;
    }

    virtual void ConfigureTimeProvider(Orchestration::TimeProviderKind ) override
    {
        // No Op
    }

    void SetSynchronized(bool isSynchronized) override
    {
        _isSynchronized = isSynchronized;
    }

    virtual bool IsSynchronized() const
    {
        return _isSynchronized;
    }
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
    HandlerId AddNextSimStepHandler(NextSimStepHandlerT simStepHandler)
    {
        const auto handlerId = _handlers.Add(std::move(simStepHandler));

        _timer.WithPeriod(_tickPeriod, [this](const auto& now) {
            _handlers.InvokeAll(now, _tickPeriod);
        });

        return handlerId;
    }

    void RemoveNextSimStepHandler(HandlerId handlerId)
    {
        _handlers.Remove(handlerId);
    }

    auto Now() const -> std::chrono::nanoseconds
    {
        const auto now = std::chrono::high_resolution_clock::now().time_since_epoch();
        return now;
    }

private:
    Util::SynchronizedHandlers<NextSimStepHandlerT> _handlers;
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
    auto Now() const -> std::chrono::nanoseconds override
    {
        // always return std::chrono::nanoseconds::min
        return std::chrono::nanoseconds::duration::min();
    }

private:
    const std::string _name{"NoSyncProvider"};
};


//! \brief  A caching time provider: we update its internal state whenever the controller's
//          simulation time changes.
// This ensures that the our time provider is available even after
// the TimeSyncService gets destructed.
class SynchronizedVirtualTimeProvider : public ProviderBase
{
public:
    SynchronizedVirtualTimeProvider()
        : ProviderBase("SyncrhonizedVirtualTimeProvider")
    {
    }

    HandlerId AddNextSimStepHandler(NextSimStepHandlerT simStepHandler) override
    {
        return _handlers.Add(std::move(simStepHandler));
    }

    void RemoveNextSimStepHandler(HandlerId handlerId) override
    {
        _handlers.Remove(handlerId);
    }

    void SetTime(std::chrono::nanoseconds now, std::chrono::nanoseconds duration) override
    {
        // tell our users about the next simulation step
        _handlers.InvokeAll(now, duration);
        _now = now;
    }

private:
    const std::string _name{"ParticipantTimeProvider"};
    Util::SynchronizedHandlers<NextSimStepHandlerT> _handlers;
};

} // namespace detail

//TimeProvider
TimeProvider::TimeProvider()
    :_currentProvider{std::make_unique<detail::NoSyncProvider>()}
{
}

void TimeProvider::ConfigureTimeProvider(Orchestration::TimeProviderKind timeProviderKind)
{
    switch (timeProviderKind)
    {
    case Orchestration::TimeProviderKind::NoSync:
        _currentProvider = std::make_unique<detail::NoSyncProvider>(); 
        break;
    case Orchestration::TimeProviderKind::WallClock: 
        _currentProvider = std::make_unique<detail::WallclockProvider>(1ms); 
        break;
    case Orchestration::TimeProviderKind::SyncTime: 
        _currentProvider = std::make_unique<detail::SynchronizedVirtualTimeProvider>(); 
        break;
    default: break;
    }
}

} // namespace Orchestration
} // namespace Core
} // namespace SilKit
