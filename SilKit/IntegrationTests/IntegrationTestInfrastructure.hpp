// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/vendor/CreateSilKitRegistry.hpp"
#include "silkit/SilKit.hpp"
#include "silkit/services/all.hpp"

#include "ConfigurationTestUtils.hpp"
#include "IParticipantInternal.hpp"
#include "CreateParticipantImpl.hpp"

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

    void SetupRegistryAndSystemMaster(const std::string& registryUri, bool sync,
                                      std::vector<std::string> requiredParticipantNames)
    {
        try
        {
            _registryUri = RunRegistry(registryUri);
            if (sync)
            {
                RunSystemMaster(_registryUri, std::move(requiredParticipantNames));
            }
        }
        catch (const std::exception& error)
        {
            ShutdownOnException(error);
        }
    }

    auto GetRegistryUri() -> std::string
    {
        return _registryUri;
    }

    void SystemMasterStop()
    {
        using namespace std::chrono_literals;
        ASSERT_EQ(_systemMaster.systemStateRunning.wait_for(1s), std::future_status::ready);
        _systemMaster.lifecycleService->Stop("SystemMaster Stop");
    }

    void ShutdownInfrastructure()
    {
        _systemMaster.participant.reset();
        _registry.reset();
    }

private:
    auto RunRegistry(const std::string& registryUri) -> std::string
    {
        _registry = SilKit::Vendor::Vector::CreateSilKitRegistry(SilKit::Config::MakeEmptyParticipantConfiguration());
        return _registry->StartListening(registryUri);
    }

    void RunSystemMaster(const std::string& registryUri, std::vector<std::string> requiredParticipantNames)
    {
        _systemMaster.participant = SilKit::CreateParticipantImpl(
            SilKit::Config::MakeEmptyParticipantConfigurationImpl(), systemMasterName, registryUri);

        auto participantInternal = dynamic_cast<SilKit::Core::IParticipantInternal*>(_systemMaster.participant.get());
        _systemMaster.systemController = participantInternal->GetSystemController();

        _systemMaster.systemMonitor = _systemMaster.participant->CreateSystemMonitor();
        _systemMaster.lifecycleService =
            _systemMaster.participant->CreateLifecycleService({OperationMode::Coordinated});

        ITimeSyncService* timeSyncService = _systemMaster.lifecycleService->CreateTimeSyncService();
        timeSyncService->SetSimulationStepHandler(
            [](std::chrono::nanoseconds /*now*/, std::chrono::nanoseconds /*duration*/) {}, std::chrono::seconds{1});

        requiredParticipantNames.push_back(systemMasterName);
        _systemMaster.systemController->SetWorkflowConfiguration({requiredParticipantNames});

        _systemMaster.systemStateRunning = _systemMaster.systemStateRunningPromise.get_future();

        _systemMaster.systemMonitor->AddSystemStateHandler([this, requiredParticipantNames](SystemState newState) {
            switch (newState)
            {
            case SystemState::Error:
                _systemMaster.systemController->AbortSimulation();
                break;
            case SystemState::Running:
                _systemMaster.systemStateRunningPromise.set_value();
                break;
            default:
                break;
            }
        });

        _systemMaster.systemMonitor->AddParticipantStatusHandler([this](const ParticipantStatus& newStatus) {
            switch (newStatus.state)
            {
            case ParticipantState::Error:
                _systemMaster.systemController->AbortSimulation();
                break;
            default:
                break;
            }
        });

        _systemMaster.lifecycleService->StartLifecycle();
    }

    std::unique_ptr<SilKit::Vendor::Vector::ISilKitRegistry> _registry;
    std::string _registryUri{"not yet defined"};
    const std::string systemMasterName{"SystemMaster"};

    struct SystemMaster
    {
        std::unique_ptr<IParticipant> participant;
        SilKit::Experimental::Services::Orchestration::ISystemController* systemController;
        ISystemMonitor* systemMonitor;
        ILifecycleService* lifecycleService;

        std::promise<void> systemStateRunningPromise;
        std::future<void> systemStateRunning;
    };

    SystemMaster _systemMaster;
};
