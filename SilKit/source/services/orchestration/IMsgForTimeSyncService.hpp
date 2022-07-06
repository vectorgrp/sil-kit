// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "SyncDatatypes.hpp"
#include "IReceiver.hpp"
#include "ISender.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

class IMsgForTimeSyncService
    : public Core::IReceiver<ParticipantCommand, NextSimTask, SystemCommand>
    , public Core::ISender<ParticipantStatus, NextSimTask>
{
};

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
