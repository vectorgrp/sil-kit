// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/sync/ITimeProvider.hpp"


namespace ib {
namespace mw {
namespace sync {

struct WallclockProvider : public ITimeProvider
{
    WallclockProvider() = default;
    const std::string _name{"WallclockProvider"};

    auto Now() const -> std::chrono::nanoseconds
    {
        return std::chrono::high_resolution_clock::now().time_since_epoch();
    }

    auto TimeProviderName() const -> const std::string& { return _name; }
};

} //end sync
} //end mw
} //end ib
