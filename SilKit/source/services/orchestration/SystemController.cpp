// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "SystemController.hpp"
#include "LifecycleService.hpp"
#include "Hash.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

SystemController::SystemController(Core::IParticipantInternal* participant)
    : _participant{participant}
{
}

void SystemController::AbortSimulation() const
{
    // AbortSimulation is directly processed in the lifecycle so that it can be called in user callbacks.
    // Self delivery of the SystemCommand is forbidden via traits.
    auto* lifecycleService = _participant->GetLifecycleService();
    dynamic_cast<SilKit::Services::Orchestration::LifecycleService*>(lifecycleService)
        ->AbortSimulation("SystemController requested AbortSimulation");

    SendSystemCommand(SystemCommand::Kind::AbortSimulation);
}

void SystemController::SetWorkflowConfiguration(const WorkflowConfiguration& workflowConfiguration)
{
    //  Distribute to SystemMonitors (including self delivery)
    _participant->SendMsg(this, workflowConfiguration);
}

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
