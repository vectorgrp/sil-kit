// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <chrono>
#include <string>

#include "ib/mw/sync/ITimeProvider.hpp"

#include "Timer.hpp"
#include "SynchronizedHandlers.hpp"

namespace ib {
namespace mw {
namespace sync {

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

private:
    util::SynchronizedHandlers<NextSimStepHandlerT> _handlers;
    const std::string _name{"WallclockProvider"};
    std::chrono::nanoseconds _tickPeriod{0};
    util::Timer _timer;
};

//////////////////////////////////////////////////////////////////////
// Inline Implementations
/////////////////////////////////////////////////////////////////////

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

auto WallclockProvider::Now() const -> std::chrono::nanoseconds
{
    const auto now = std::chrono::high_resolution_clock::now().time_since_epoch();
    return now;
}

auto WallclockProvider::TimeProviderName() const -> const std::string&
{
    return _name;
}

} // namespace sync
} // namespace mw
} // namespace ib
