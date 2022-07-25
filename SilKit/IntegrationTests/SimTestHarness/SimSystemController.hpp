/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
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
