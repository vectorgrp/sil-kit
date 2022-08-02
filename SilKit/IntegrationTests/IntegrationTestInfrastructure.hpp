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

#include "silkit/vendor/CreateSilKitRegistry.hpp"
#include "silkit/SilKit.hpp"
#include "silkit/services/all.hpp"
#include "ConfigurationTestUtils.hpp"

using namespace SilKit;
using namespace SilKit::Services::Orchestration;
using namespace SilKit::Config;

class TestInfrastructure
{

public:
    TestInfrastructure() {}

    void ShutdownOnException(const std::exception& error)
    {
        std::stringstream ss;
        ss << "Something went wrong: " << error.what() << std::endl;
        _systemMaster.systemController->AbortSimulation();
        FAIL() << ss.str();
    }
    
    void SetupRegistryAndSystemMaster(const std::string& registryUri, bool sync, std::vector<std::string> requiredParticipantNames)
    {
        try
        {
            RunRegistry(registryUri);
            if (sync)
            {
                RunSystemMaster(registryUri, std::move(requiredParticipantNames));
            }
        }
        catch (const std::exception& error)
        {
            ShutdownOnException(error);
        }
    }

    void SystemMasterStop() { _systemMaster.lifecycleService->Stop("SystemMaster Stop"); }

    void ShutdownInfrastructure()
    {
        _systemMaster.participant.reset();
        _registry.reset();
    }

private:

    void RunRegistry(const std::string& registryUri)
    {
        _registry = SilKit::Vendor::Vector::CreateSilKitRegistry(SilKit::Config::MakeEmptyParticipantConfiguration());
        _registry->StartListening(registryUri);
    }

    void RunSystemMaster(const std::string& registryUri, std::vector<std::string> requiredParticipantNames)
    {
        _systemMaster.participant = SilKit::CreateParticipant(SilKit::Config::MakeEmptyParticipantConfiguration(),
                                                              systemMasterName, registryUri);

        _systemMaster.systemController = _systemMaster.participant->CreateSystemController();
        _systemMaster.systemMonitor = _systemMaster.participant->CreateSystemMonitor();
        _systemMaster.lifecycleService = _systemMaster.participant->CreateLifecycleService();

        ITimeSyncService * timeSyncService = _systemMaster.lifecycleService->CreateTimeSyncService();
        timeSyncService->SetSimulationStepHandler([](std::chrono::nanoseconds /*now*/) {}, std::chrono::seconds{1});

        requiredParticipantNames.push_back(systemMasterName);
        _systemMaster.systemController->SetWorkflowConfiguration({requiredParticipantNames});

        _systemMaster.systemMonitor->AddSystemStateHandler([this, requiredParticipantNames](SystemState newState) {
            switch (newState)
            {
            case SystemState::Error:
                _systemMaster.systemController->AbortSimulation();
                break;
            default: break;
            }
        });

        _systemMaster.systemMonitor->AddParticipantStatusHandler([this](const ParticipantStatus& newStatus) {
            switch (newStatus.state)
            {
            case ParticipantState::Error: _systemMaster.systemController->AbortSimulation(); break;
            default: break;
            }
        });

        _systemMaster.lifecycleService->StartLifecycle({OperationMode::Coordinated});
    }

    std::unique_ptr<SilKit::Vendor::Vector::ISilKitRegistry> _registry;
    const std::string systemMasterName{"SystemMaster"};

    struct SystemMaster
    {
        std::unique_ptr<IParticipant> participant;
        ISystemController* systemController;
        ISystemMonitor* systemMonitor;
        ILifecycleService* lifecycleService;
    };

    SystemMaster _systemMaster;

};
