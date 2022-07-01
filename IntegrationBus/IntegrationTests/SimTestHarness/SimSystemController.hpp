// Copyright (c) Vector Informatik GmbH. All rights reserved.
#pragma once

#include "ib/IntegrationBus.hpp"

#include "ConfigurationTestUtils.hpp"

namespace ib {
namespace test {

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
            ib::CreateParticipant(ib::cfg::MakeEmptyParticipantConfiguration(), "SystemController", registryUri);

        _controller = _participant->GetSystemController();
        _controller->SetWorkflowConfiguration({_syncParticipantNames});
        _monitor = _participant->GetSystemMonitor();
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

    void OnParticipantStatusChanged(ib::mw::sync::ParticipantStatus status)
    {
        if (_isShuttingDown)
        {
            return;
        }
        _participantStates[status.participantName] = status.state;

        if (status.state == ib::mw::sync::ParticipantState::Stopped
            || status.state == ib::mw::sync::ParticipantState::Error)
        {
            _controller->Stop();
        }
    }

    void OnSystemStateChanged(ib::mw::sync::SystemState state)
    {
        if (_isShuttingDown)
        {
            return;
        }
        //std::cout << "SimTestHarness: System State is now " << state << std::endl;
        switch (state)
        {
        case ib::mw::sync::SystemState::ReadyToRun:
            _controller->Run();
            return;
        case ib::mw::sync::SystemState::Running:
            return;
        case ib::mw::sync::SystemState::Stopped:
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
    ib::mw::sync::ISystemController* _controller;
    ib::mw::sync::ISystemMonitor* _monitor;
    std::vector<std::string> _syncParticipantNames;
    std::map<std::string, ib::mw::sync::ParticipantState> _participantStates; //for printing status updates
    std::atomic<bool> _isShuttingDown{false};
    std::unique_ptr<ib::mw::IParticipant> _participant;
};

} // namespace test
} // namespace ib
