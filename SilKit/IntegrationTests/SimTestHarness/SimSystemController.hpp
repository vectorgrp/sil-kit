// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/SilKit.hpp"
#include "silkit/config/IParticipantConfiguration.hpp"
#include "silkit/experimental/participant/ParticipantExtensions.hpp"

namespace SilKit {
namespace Tests {

////////////////////////////////////////
// SimSystemController
////////////////////////////////////////
class SimSystemController
{
public:
    SimSystemController() = delete;
    SimSystemController(const std::vector<std::string>& syncParticipantNames, const std::string& registryUri)
        : _syncParticipantNames{syncParticipantNames}
    {
        _participant = SilKit::CreateParticipant(SilKit::Config::ParticipantConfigurationFromString(""),
                                                 "SystemController", registryUri);

        _controller = SilKit::Experimental::Participant::CreateSystemController(_participant.get());

        _controller->SetWorkflowConfiguration({_syncParticipantNames});
        _monitor = _participant->CreateSystemMonitor();
    }

    ~SimSystemController()
    {
        _isShuttingDown = true;
        _participant.reset();
    }

private:
    SilKit::Experimental::Services::Orchestration::ISystemController* _controller;
    SilKit::Services::Orchestration::ISystemMonitor* _monitor;
    std::vector<std::string> _syncParticipantNames;
    std::map<std::string, SilKit::Services::Orchestration::ParticipantState>
        _participantStates; //for printing status updates
    std::atomic<bool> _isShuttingDown{false};
    std::unique_ptr<SilKit::IParticipant> _participant;
};

} // namespace Tests
} // namespace SilKit
