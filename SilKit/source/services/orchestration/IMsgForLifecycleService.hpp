// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "SyncDatatypes.hpp"
#include "IReceiver.hpp"
#include "ISender.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

class IMsgForLifecycleService
    : public Core::IReceiver<ParticipantCommand, SystemCommand>
    , public Core::ISender<ParticipantStatus>
{
};

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
