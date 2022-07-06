// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "SystemController.hpp"
#include "Hash.hpp"

namespace SilKit {
namespace Core {
namespace Orchestration {

SystemController::SystemController(IParticipantInternal* participant)
    : _participant{participant}
{
}

void SystemController::Restart(const std::string& participantName) const
{
    SendParticipantCommand(Util::Hash::Hash(participantName), ParticipantCommand::Kind::Restart);
}

void SystemController::Run() const
{
    SendSystemCommand(SystemCommand::Kind::Run);
}

void SystemController::Stop() const
{
    SendSystemCommand(SystemCommand::Kind::Stop);
}

void SystemController::Shutdown(const std::string& participantName) const
{
    SendParticipantCommand(Util::Hash::Hash(participantName), ParticipantCommand::Kind::Shutdown);
}

void SystemController::AbortSimulation() const
{
    SendSystemCommand(SystemCommand::Kind::AbortSimulation);
}

void SystemController::SetWorkflowConfiguration(const WorkflowConfiguration& workflowConfiguration)
{
    //  Distribute to SystemMonitors (including self delivery) 
    _participant->SendMsg(this, workflowConfiguration);
}

} // namespace Orchestration
} // namespace Core
} // namespace SilKit
