// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "silkit/services/orchestration/SyncDatatypes.hpp"
#include "IReceiver.hpp"
#include "ISender.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

class IMsgForSystemController
    : public Core::IReceiver<>
    , public Core::ISender<ParticipantCommand, SystemCommand, WorkflowConfiguration>
{
};

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
