// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <chrono>
#include <string>

#include "ITimeProvider.hpp"
#include "Timer.hpp"
#include "SynchronizedHandlers.hpp"

namespace ib {
namespace mw {
namespace sync {

enum class TimeProviderKind : uint8_t
{
    NoSync = 0,
    WallClock = 1,
    SyncTime = 2
};

class WallclockProvider : public ITimeProvider
{
public:
    WallclockProvider(std::chrono::nanoseconds tickPeriod)
        : _tickPeriod{tickPeriod}
    {
    }

    inline auto Now() const -> std::chrono::nanoseconds override;
    inline auto TimeProviderName() const -> const std::string& override;
    inline HandlerId AddNextSimStepHandler(NextSimStepHandlerT handler) override;
    inline void RemoveNextSimStepHandler(HandlerId handlerId) override;
    inline void SetTime(std::chrono::nanoseconds now, std::chrono::nanoseconds duration) override;

private:
    util::SynchronizedHandlers<NextSimStepHandlerT> _handlers;
    const std::string _name{"WallclockProvider"};
    std::chrono::nanoseconds _tickPeriod{0};
    util::Timer _timer;
};

class NoSyncProvider : public ITimeProvider
{
public:
    inline auto Now() const -> std::chrono::nanoseconds override;
    inline auto TimeProviderName() const -> const std::string& override;
    inline HandlerId AddNextSimStepHandler(NextSimStepHandlerT handler) override;
    inline void RemoveNextSimStepHandler(HandlerId handlerId) override;
    inline void SetTime(std::chrono::nanoseconds now, std::chrono::nanoseconds duration) override;

private:
    const std::string _name{"NoSyncProvider"};
};


//! \brief  A caching time provider: we update its internal state whenever the controller's
//          simulation time changes.
// This ensures that the our time provider is available even after
// the TimeSyncService gets destructed.
class SynchronizedVirtualTimeProvider : public sync::ITimeProvider
{
public:
    inline auto Now() const -> std::chrono::nanoseconds override;
    inline auto TimeProviderName() const -> const std::string& override;
    inline HandlerId AddNextSimStepHandler(NextSimStepHandlerT handler) override;
    inline void RemoveNextSimStepHandler(HandlerId handlerId) override;
    inline void SetTime(std::chrono::nanoseconds now, std::chrono::nanoseconds duration) override;

private:
    std::chrono::nanoseconds _now;
    const std::string _name{"ParticipantTimeProvider"};
    util::SynchronizedHandlers<NextSimStepHandlerT> _handlers;
};

//////////////////////////////////////////////////////////////////////
// Inline Implementations
/////////////////////////////////////////////////////////////////////

// Wall clock provider
HandlerId WallclockProvider::AddNextSimStepHandler(NextSimStepHandlerT simStepHandler)
{
    const auto handlerId = _handlers.Add(std::move(simStepHandler));

    _timer.WithPeriod(_tickPeriod, [this](const auto& now) {
        _handlers.InvokeAll(now, _tickPeriod);
    });

    return handlerId;
}

void WallclockProvider::RemoveNextSimStepHandler(HandlerId handlerId)
{
    _handlers.Remove(handlerId);
}

void WallclockProvider::SetTime(std::chrono::nanoseconds /*now*/, std::chrono::nanoseconds /*duration*/)
{
    // NOP
}

auto WallclockProvider::Now() const -> std::chrono::nanoseconds
{
    const auto now = std::chrono::high_resolution_clock::now().time_since_epoch();
    return now;
}

auto WallclockProvider::TimeProviderName() const -> const std::string&
{
    return _name;
}

// No sync provider
HandlerId NoSyncProvider::AddNextSimStepHandler(NextSimStepHandlerT /*simStepHandler*/)
{
    // NOP
    return {};
}

void NoSyncProvider::RemoveNextSimStepHandler(HandlerId /*handlerId*/)
{
    // NOP
}

void NoSyncProvider::SetTime(std::chrono::nanoseconds /*now*/, std::chrono::nanoseconds /*duration*/)
{
    // NOP
}

auto NoSyncProvider::Now() const -> std::chrono::nanoseconds
{
    // always return std::chrono::nanoseconds::min
    return std::chrono::nanoseconds::duration::min();
}

auto NoSyncProvider::TimeProviderName() const -> const std::string&
{
    return _name;
}


// Synchronized virtual time provider
HandlerId SynchronizedVirtualTimeProvider::AddNextSimStepHandler(NextSimStepHandlerT simStepHandler)
{
    return _handlers.Add(std::move(simStepHandler));
}

void SynchronizedVirtualTimeProvider::RemoveNextSimStepHandler(HandlerId handlerId)
{
    _handlers.Remove(handlerId);
}

void SynchronizedVirtualTimeProvider::SetTime(std::chrono::nanoseconds now, std::chrono::nanoseconds duration)
{
    // tell our users about the next simulation step
    _handlers.InvokeAll(now, duration);
    _now = now;
}

auto SynchronizedVirtualTimeProvider::Now() const -> std::chrono::nanoseconds
{
    return _now;
}

auto SynchronizedVirtualTimeProvider::TimeProviderName() const -> const std::string&
{
    return _name;
}
} // namespace sync
} // namespace mw
} // namespace ib
