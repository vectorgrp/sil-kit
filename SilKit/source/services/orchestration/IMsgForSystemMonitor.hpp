// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/services/orchestration/OrchestrationDatatypes.hpp"
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
