// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/sync/ITimeProvider.hpp"


namespace ib {
namespace mw {
namespace sync {

class WallclockProvider : public ITimeProvider
{
public:
    WallclockProvider(std::chrono::nanoseconds tickPeriod)
        :_tickPeriod{tickPeriod}
    {
    }

    auto Now() const -> std::chrono::nanoseconds
    {
        return std::chrono::high_resolution_clock::now().time_since_epoch();
    }

    auto TimeProviderName() const -> const std::string& 
    {
        return _name; 
    }

    void RegisterNextSimStepHandler(NextSimStepHandlerT handler)
    {
        _handlers.emplace_back(std::move(handler));
    }
private:
    std::vector<NextSimStepHandlerT> _handlers;
    const std::string _name{"WallclockProvider"};
    std::chrono::nanoseconds _tickPeriod{0};
    //TODO add a timer, call handlers after _tickPeriod.
};

} //end sync
} //end mw
} //end ib
