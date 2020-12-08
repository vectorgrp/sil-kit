// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once
#include <vector>
#include <mutex>

#include "ib/mw/sync/ITimeProvider.hpp"
#include "Timer.hpp"

namespace ib {
namespace mw {
namespace sync {

class WallclockProvider : public ITimeProvider
{
public:
    WallclockProvider(std::chrono::nanoseconds tickPeriod)
        :_tickPeriod{tickPeriod}
    {
        _timer.WithPeriod(tickPeriod, [this](const auto& now) {
            std::unique_lock<std::mutex> lock{_mx};
            for (const auto& handler : _handlers)
            {
                handler(now, _tickPeriod);
            }
        });
    }

    inline auto Now() const -> std::chrono::nanoseconds;
    inline auto TimeProviderName() const -> const std::string&;
    inline void RegisterNextSimStepHandler(NextSimStepHandlerT handler);
private:
    std::vector<NextSimStepHandlerT> _handlers;
    const std::string _name{"WallclockProvider"};
    std::chrono::nanoseconds _tickPeriod{0};
    util::Timer _timer;
    std::mutex _mx;
};

//////////////////////////////////////////////////////////////////////
// Inline Implementations
/////////////////////////////////////////////////////////////////////

void WallclockProvider::RegisterNextSimStepHandler(NextSimStepHandlerT handler)
{
    std::unique_lock<std::mutex> lock{_mx};
    _handlers.emplace_back(std::move(handler));
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

} //end sync
} //end mw
} //end ib
