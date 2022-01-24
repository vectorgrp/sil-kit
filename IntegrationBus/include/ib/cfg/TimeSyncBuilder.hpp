// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/IbMacros.hpp"

#include "Config.hpp"

namespace ib {
namespace cfg {
inline namespace deprecated {

class TimeSyncBuilder
{
public:
    IntegrationBusAPI TimeSyncBuilder();

    IntegrationBusAPI auto WithTickPeriod(std::chrono::nanoseconds period) -> TimeSyncBuilder&;

    IntegrationBusAPI auto WithStrictSyncPolicy() -> TimeSyncBuilder&;
    IntegrationBusAPI auto WithLooseSyncPolicy() -> TimeSyncBuilder&;
    IntegrationBusAPI auto WithSyncPolicy(TimeSync::SyncPolicy syncPolicy) -> TimeSyncBuilder&;

    IntegrationBusAPI auto Build() -> TimeSync;

private:
    TimeSync _config;
};

} // namespace deprecated
} // namespace cfg
} // namespace ib
