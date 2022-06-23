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

void SystemController::Shutdown() const
{
    SendSystemCommand(SystemCommand::Kind::Shutdown);
}

void SystemController::PrepareColdswap() const
{
    SendSystemCommand(SystemCommand::Kind::PrepareColdswap);
}

void SystemController::ExecuteColdswap() const
{
    _participant->FlushSendBuffers();
    SendSystemCommand(SystemCommand::Kind::ExecuteColdswap);
}

void SystemController::AbortSimulation() const
{
    SendSystemCommand(SystemCommand::Kind::AbortSimulation);
}

void SystemController::SetRequiredParticipants(const std::vector<std::string>& participantNames)
{
    ExpectedParticipants expectedParticipants{participantNames};
    //  Distribute to SystemMonitors (including self delivery) 
    _participant->SendIbMessage(this, std::move(expectedParticipants));
}

} // namespace sync
} // namespace mw
} // namespace ib
