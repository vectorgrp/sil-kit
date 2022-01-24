// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "ParticipantControllerBuilder.hpp"

#include <algorithm>

namespace ib {
namespace cfg {
inline namespace deprecated {

ParticipantControllerBuilder::ParticipantControllerBuilder(ParticipantBuilder* parent)
    : ParentBuilder<ParticipantBuilder>{parent}
{
}

auto ParticipantControllerBuilder::Build() -> ParticipantController
{
    if (_config.syncType == SyncType::Unsynchronized)
        throw Misconfiguration{"ParticipantController must specify valid SyncType"};
   
    ParticipantController newConfig{};
    std::swap(_config, newConfig);
    return newConfig;
}

auto ParticipantControllerBuilder::WithSyncType(SyncType syncType) -> ParticipantControllerBuilder&
{
    _config.syncType = syncType;
    return *this;
}

auto ParticipantControllerBuilder::WithExecTimeLimitSoft(std::chrono::milliseconds limit) -> ParticipantControllerBuilder&
{
    _config.execTimeLimitSoft = limit;
    return *this;
}
auto ParticipantControllerBuilder::WithExecTimeLimitHard(std::chrono::milliseconds limit) -> ParticipantControllerBuilder&
{
    _config.execTimeLimitHard = limit;
    return *this;
}

auto ParticipantControllerBuilder::operator->() -> ParticipantControllerBuilder*
{
    return this;
}

} // inline namespace deprecated
} // namespace cfg
} // namespace ib
