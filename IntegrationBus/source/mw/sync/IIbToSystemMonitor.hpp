// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "silkit/core/sync/SyncDatatypes.hpp"
#include "IReceiver.hpp"
#include "ISender.hpp"

namespace SilKit {
namespace Core {
namespace Orchestration {

class IMsgForSystemMonitor
    : public Core::IReceiver<ParticipantStatus, WorkflowConfiguration>
    , public Core::ISender<>
{
};

} // namespace Orchestration
} // namespace Core
} // namespace SilKit
