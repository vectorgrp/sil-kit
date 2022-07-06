// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "silkit/services/orchestration/SyncDatatypes.hpp"
#include "IReceiver.hpp"
#include "ISender.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

class IMsgForSystemMonitor
    : public Core::IReceiver<ParticipantStatus, WorkflowConfiguration>
    , public Core::ISender<>
{
};

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
