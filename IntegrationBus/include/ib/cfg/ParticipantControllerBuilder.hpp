// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <chrono>

#include "ib/IbMacros.hpp"

#include "fwd_decl.hpp"

#include "Config.hpp"
#include "ParentBuilder.hpp"

namespace ib {
namespace cfg {
inline namespace deprecated {

class ParticipantControllerBuilder : public ParentBuilder<ParticipantBuilder>
{
public:
    IntegrationBusAPI ParticipantControllerBuilder(ParticipantBuilder* parent);

    IntegrationBusAPI auto WithSyncType(SyncType syncType) -> ParticipantControllerBuilder&;
    IntegrationBusAPI auto WithExecTimeLimitSoft(std::chrono::milliseconds limit) -> ParticipantControllerBuilder&;
    IntegrationBusAPI auto WithExecTimeLimitHard(std::chrono::milliseconds limit) -> ParticipantControllerBuilder&;

    IntegrationBusAPI auto operator->() -> ParticipantControllerBuilder*;

    IntegrationBusAPI auto Build() -> ParticipantController;

private:
    ParticipantController _config;
};

} // namespace deprecated
} // namespace cfg
} // namespace ib
