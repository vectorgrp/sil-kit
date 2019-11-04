// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "ParticipantControllerBuilder.hpp"


namespace ib {
namespace cfg {

ParticipantControllerBuilder::ParticipantControllerBuilder(ParticipantBuilder* parent)
    : ParentBuilder<ParticipantBuilder>{parent}
{
    config._is_configured = true;
}

auto ParticipantControllerBuilder::Build() -> ParticipantController
{
    if (config.syncType == SyncType::Unsynchronized)
        throw Misconfiguration{"ParticipantController must specify valid SyncType"};
    
    ParticipantController newConfig;
    std::swap(config, newConfig);
    return std::move(newConfig);
}

auto ParticipantControllerBuilder::WithSyncType(SyncType syncType) -> ParticipantControllerBuilder&
{
    config.syncType = syncType;
    return *this;
}

auto ParticipantControllerBuilder::WithExecTimeLimitSoft(std::chrono::milliseconds limit) -> ParticipantControllerBuilder&
{
    config.execTimeLimitSoft = limit;
    return *this;
}
auto ParticipantControllerBuilder::WithExecTimeLimitHard(std::chrono::milliseconds limit) -> ParticipantControllerBuilder&
{
    config.execTimeLimitHard = limit;
    return *this;
}

auto ParticipantControllerBuilder::operator->() -> ParticipantControllerBuilder*
{
    return this;
}

} // namespace cfg
} // namespace ib
