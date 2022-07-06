// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "SyncDatatypes.hpp"
#include "IReceiver.hpp"
#include "ISender.hpp"

namespace SilKit {
namespace Core {
namespace Orchestration {

class IMsgForParticipantController
    : public Core::IReceiver<ParticipantCommand, SystemCommand, NextSimTask>
    , public Core::ISender<ParticipantStatus, NextSimTask>
{
};

} // namespace Orchestration
} // namespace Core
} // namespace SilKit
