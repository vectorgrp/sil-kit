// Copyright (c) Vector Informatik GmbH. All rights reserved.
#pragma once

#include "silkit/SilKit.hpp"

#include "ConfigurationTestUtils.hpp"

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
        _participant =
            SilKit::CreateParticipant(SilKit::Config::MakeEmptyParticipantConfiguration(), "SystemController", registryUri);

        _controller = _participant->CreateSystemController();
        _controller->SetWorkflowConfiguration({_syncParticipantNames});
        _monitor = _participant->CreateSystemMonitor();
        _monitor->AddSystemStateHandler(
            std::bind(&SimSystemController::OnSystemStateChanged, this, std::placeholders::_1));
        _monitor->AddParticipantStatusHandler(
            std::bind(&SimSystemController::OnParticipantStatusChanged, this, std::placeholders::_1));
    }

    ~SimSystemController()
    {
        _isShuttingDown = true;
        _participant.reset();
    }

    void OnParticipantStatusChanged(SilKit::Services::Orchestration::ParticipantStatus status)
    {
        if (_isShuttingDown)
        {
            return;
        }
        _participantStates[status.participantName] = status.state;

        if (status.state == SilKit::Services::Orchestration::ParticipantState::Stopped
            || status.state == SilKit::Services::Orchestration::ParticipantState::Error)
        {
            _controller->Stop();
        }
    }

    void OnSystemStateChanged(SilKit::Services::Orchestration::SystemState state)
    {
        if (_isShuttingDown)
        {
            return;
        }
        //std::cout << "SimTestHarness: System State is now " << state << std::endl;
        switch (state)
        {
        case SilKit::Services::Orchestration::SystemState::ReadyToRun:
            _controller->Run();
            return;
        case SilKit::Services::Orchestration::SystemState::Running:
            return;
        case SilKit::Services::Orchestration::SystemState::Stopped:
            for (const auto& name : _syncParticipantNames)
            {
                _controller->Shutdown(name);
            }
            return;
        default:
            return;
        }
    }
private:
    SilKit::Services::Orchestration::ISystemController* _controller;
    SilKit::Services::Orchestration::ISystemMonitor* _monitor;
    std::vector<std::string> _syncParticipantNames;
    std::map<std::string, SilKit::Services::Orchestration::ParticipantState> _participantStates; //for printing status updates
    std::atomic<bool> _isShuttingDown{false};
    std::unique_ptr<SilKit::IParticipant> _participant;
};

} // namespace Tests
} // namespace SilKit
