// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#include "TimeSyncBuilder.hpp"

namespace ib {
namespace cfg {


TimeSyncBuilder::TimeSyncBuilder(TimeSync::SyncType type)
{
    _config.syncType = type;
}

auto TimeSyncBuilder::WithTickPeriod(std::chrono::nanoseconds period) -> TimeSyncBuilder&
{
    _config.tickPeriod = period;
    return *this;
}

auto TimeSyncBuilder::Build() -> TimeSync
{
    return _config;
}


} // namespace cfg
} // namespace ib
