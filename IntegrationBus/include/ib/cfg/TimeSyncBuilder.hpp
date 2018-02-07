// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/IbMacros.hpp"

#include "Config.hpp"

namespace ib {
namespace cfg {

class TimeSyncBuilder
{
public:
    IntegrationBusAPI TimeSyncBuilder(TimeSync::SyncType type);

    IntegrationBusAPI auto WithTickPeriod(std::chrono::nanoseconds period) -> TimeSyncBuilder&;

    IntegrationBusAPI auto Build() -> TimeSync;

private:
    TimeSync _config;
};

} // namespace cfg
} // namespace ib
