// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "SystemController.hpp"
#include "Hash.hpp"

namespace ib {
namespace mw {
namespace sync {

SystemController::SystemController(IParticipantInternal* participant)
    : _participant{participant}
{
}

void SystemController::Initialize(const std::string&) const
{
    //SendParticipantCommand(util::hash::Hash(participantName), ParticipantCommand::Kind::Initialize);
}

void SystemController::Restart(const std::string& participantName) const
{
    SendParticipantCommand(util::hash::Hash(participantName), ParticipantCommand::Kind::Restart);
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
    SendParticipantCommand(util::hash::Hash(participantName), ParticipantCommand::Kind::Shutdown);
}

void SystemController::AbortSimulation() const
{
    SendSystemCommand(SystemCommand::Kind::AbortSimulation);
}

void SystemController::SetWorkflowConfiguration(const WorkflowConfiguration& workflowConfiguration)
{
    //  Distribute to SystemMonitors (including self delivery) 
    _participant->SendIbMessage(this, std::move(workflowConfiguration));
}

} // namespace sync
} // namespace mw
} // namespace ib
