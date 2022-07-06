// Copyright (c) Vector Informatik GmbH. All rights reserved.

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
        _systemMaster.systemController->Shutdown("SystemMaster");
        FAIL() << ss.str();
    }
    
    void SetupRegistryAndSystemMaster(const std::string& registryUri, bool sync, const std::vector<std::string>& requiredParticipantNames)
    {
        try
        {
            RunRegistry(registryUri);
            if (sync)
            {
                RunSystemMaster(registryUri, requiredParticipantNames);
            }
        }
        catch (const std::exception& error)
        {
            ShutdownOnException(error);
        }
    }

    void SystemMasterStop() { _systemMaster.systemController->Stop(); }

    void ShutdownInfrastructure()
    {
        _systemMaster.participant.reset();
        _registry.reset();
    }

private:

    void RunRegistry(const std::string& registryUri)
    {
        _registry = SilKit::Vendor::CreateSilKitRegistry(SilKit::Config::MakeEmptyParticipantConfiguration());
        _registry->ProvideDomain(registryUri);
    }

    void RunSystemMaster(const std::string& registryUri, const std::vector<std::string>& requiredParticipantNames)
    {
        _systemMaster.participant =
            SilKit::CreateParticipant(SilKit::Config::MakeEmptyParticipantConfiguration(), "SystemMaster", registryUri);

        _systemMaster.systemController = _systemMaster.participant->GetSystemController();
        _systemMaster.systemMonitor = _systemMaster.participant->GetSystemMonitor();
        _systemMaster.systemController->SetWorkflowConfiguration({requiredParticipantNames});

        _systemMaster.systemMonitor->AddSystemStateHandler([this, requiredParticipantNames](SystemState newState) {
            switch (newState)
            {
            case SystemState::ReadyToRun:
                _systemMaster.systemController->Run();
                break;
            case SystemState::Stopped:
                for (auto&& name : requiredParticipantNames)
                {
                    _systemMaster.systemController->Shutdown(name);
                }
                _systemMaster.systemController->Shutdown("SystemMaster");
                break;
            case SystemState::Error:
                for (auto&& name : requiredParticipantNames)
                {
                    _systemMaster.systemController->Shutdown(name);
                }
                _systemMaster.systemController->Shutdown("SystemMaster");
                break;
            default: break;
            }
        });

        _systemMaster.systemMonitor->AddParticipantStatusHandler([this](const ParticipantStatus& newStatus) {
            switch (newStatus.state)
            {
            case ParticipantState::Error: _systemMaster.systemController->Shutdown("SystemMaster"); break;
            default: break;
            }
        });
    }

    std::unique_ptr<SilKit::Vendor::ISilKitRegistry> _registry;

    struct SystemMaster
    {
        std::unique_ptr<IParticipant> participant;
        ISystemController* systemController;
        ISystemMonitor* systemMonitor;
    };

    SystemMaster _systemMaster;

};
