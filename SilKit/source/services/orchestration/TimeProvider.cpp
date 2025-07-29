// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "TimeProvider.hpp"

#include <mutex>


namespace {

// The 'default' value for timestamps on participants without time-sync is the minimum duration in nanoseconds.
constexpr auto DEFAULT_NOW_TIMESTAMP_WITHOUT_SYNC = std::chrono::nanoseconds::min();

} // namespace


using namespace std::chrono_literals;


namespace SilKit {
namespace Services {
namespace Orchestration {

// Actual Provider Implementations
namespace {


class ProviderBase : public ITimeProviderImpl
{
public:
    ProviderBase(std::string name, ITimeProviderImplListener& listener)
        : _name{std::move(name)}
        , _listener{&listener}
    {
    }

    // ITimeProvierImpl (partial)

    auto TimeProviderName() const -> const std::string& final
    {
        return _name;
    }

    void SetActive(bool value) final
    {
        _active = value;
    }

protected:
    void NotifyListenerAboutTick(std::chrono::nanoseconds now, std::chrono::nanoseconds duration)
    {
        if (!_active)
        {
            return;
        }

        _listener->OnTick(now, duration);
    }

private:
    std::string _name;
    ITimeProviderImplListener* _listener;
    std::atomic<bool> _active{false};
};


class WallclockProvider final : public ProviderBase
{
public:
    explicit WallclockProvider(ITimeProviderImplListener& consumer, std::chrono::nanoseconds tickPeriod)
        : ProviderBase("WallclockProvider", consumer)
        , _tickPeriod{tickPeriod}
    {
    }

    void OnHandlerAdded() final
    {
        _timer.WithPeriod(_tickPeriod, [this](const auto& now) { NotifyListenerAboutTick(now, _tickPeriod); });
    }

    auto Now() const -> std::chrono::nanoseconds override
    {
        const auto now = std::chrono::high_resolution_clock::now().time_since_epoch();
        return now;
    }

    void SetTime(std::chrono::nanoseconds, std::chrono::nanoseconds) override {}

private:
    std::chrono::nanoseconds _tickPeriod{0};
    Util::Timer _timer;
};


class NoSyncProvider final : public ProviderBase
{
public:
    explicit NoSyncProvider(ITimeProviderImplListener& consumer)
        : ProviderBase("NoSyncProvider", consumer)
    {
    }

    void OnHandlerAdded() override
    {
        _timer.WithPeriod(_tickPeriod, [this](const auto& now) { NotifyListenerAboutTick(now, _tickPeriod); });
    }

    auto Now() const -> std::chrono::nanoseconds override
    {
        return DEFAULT_NOW_TIMESTAMP_WITHOUT_SYNC;
    }

    void SetTime(std::chrono::nanoseconds, std::chrono::nanoseconds) override {}

private:
    std::chrono::nanoseconds _tickPeriod{100000};
    Util::Timer _timer;
};


/// A caching time provider: we update its internal state whenever the controller's simulation time changes.
/// This ensures that the our time provider is available even after the TimeSyncService gets destructed.
class SynchronizedVirtualTimeProvider final : public ProviderBase
{
public:
    explicit SynchronizedVirtualTimeProvider(ITimeProviderImplListener& listener)
        : ProviderBase("SynchronizedVirtualTimeProvider", listener)
    {
    }

    void OnHandlerAdded() override {}

    auto Now() const -> std::chrono::nanoseconds override
    {
        return _now;
    }

    void SetTime(std::chrono::nanoseconds now, std::chrono::nanoseconds duration) override
    {
        _now = now;
        // tell our users about the next simulation step
        NotifyListenerAboutTick(now, duration);
    }

private:
    std::chrono::nanoseconds _now{DEFAULT_NOW_TIMESTAMP_WITHOUT_SYNC};
};


} // namespace


TimeProvider::TimeProvider()
    : _currentProvider{std::make_unique<NoSyncProvider>(static_cast<ITimeProviderImplListener&>(*this))}
{
}

void TimeProvider::ConfigureTimeProvider(Orchestration::TimeProviderKind timeProviderKind)
{
    // NB: The destructor of the 'old' time provider implementation must be called _without_ the lock being held.
    //     Otherwise, the timer-thread (used in the WallclockProvider / NoSyncProvider) will be joined under lock, and
    //     cause a deadlock when changing implementations.
    std::unique_ptr<ITimeProviderImpl> providerPtr;

    {
        std::unique_lock<decltype(_mutex)> lock{_mutex};

        switch (timeProviderKind)
        {
        case Orchestration::TimeProviderKind::NoSync:
            providerPtr = std::make_unique<NoSyncProvider>(static_cast<ITimeProviderImplListener&>(*this));
            break;
        case Orchestration::TimeProviderKind::WallClock:
            providerPtr = std::make_unique<WallclockProvider>(static_cast<ITimeProviderImplListener&>(*this), 1ms);
            break;
        case Orchestration::TimeProviderKind::SyncTime:
            providerPtr =
                std::make_unique<SynchronizedVirtualTimeProvider>(static_cast<ITimeProviderImplListener&>(*this));
            break;
        default:
            break;
        }

        if (providerPtr)
        {
            using std::swap;

            _currentProvider->SetActive(false);

            // swap the newly created provider with the current provider
            swap(_currentProvider, providerPtr);

            _currentProvider->SetActive(true);
        }
    }
}

void TimeProvider::OnTick(std::chrono::nanoseconds now, std::chrono::nanoseconds duration)
{
    std::unique_lock<decltype(_mutex)> lock{_mutex};
    _handlers.InvokeAll(now, duration);
}


} // namespace Orchestration
} // namespace Services
} // namespace SilKit
