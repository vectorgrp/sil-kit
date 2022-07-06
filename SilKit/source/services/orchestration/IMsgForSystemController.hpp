// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "silkit/core/sync/SyncDatatypes.hpp"
#include "IReceiver.hpp"
#include "ISender.hpp"

namespace SilKit {
namespace Core {
namespace Orchestration {

class IMsgForSystemController
    : public Core::IReceiver<>
    , public Core::ISender<ParticipantCommand, SystemCommand, WorkflowConfiguration>
{
};

} // namespace Orchestration
} // namespace Core
} // namespace SilKit
