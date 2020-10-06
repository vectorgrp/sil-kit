// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "TimeSyncBuilder.hpp"

namespace ib {
namespace cfg {


TimeSyncBuilder::TimeSyncBuilder()
{
}

auto TimeSyncBuilder::WithTickPeriod(std::chrono::nanoseconds period) -> TimeSyncBuilder&
{
    _config.tickPeriod = period;
    return *this;
}

auto TimeSyncBuilder::WithStrictSyncPolicy() -> TimeSyncBuilder&
{
    _config.syncPolicy = TimeSync::SyncPolicy::Strict;
    return *this;
}

auto TimeSyncBuilder::WithLooseSyncPolicy() -> TimeSyncBuilder&
{
    _config.syncPolicy = TimeSync::SyncPolicy::Loose;
        return *this;
}

auto TimeSyncBuilder::WithSyncPolicy(TimeSync::SyncPolicy syncPolicy) -> TimeSyncBuilder&
{
    _config.syncPolicy = syncPolicy;
    return *this;
}

auto TimeSyncBuilder::Build() -> TimeSync
{
    TimeSync newConfig{};
    std::swap(_config, newConfig);
    return newConfig;
}


} // namespace cfg
} // namespace ib
