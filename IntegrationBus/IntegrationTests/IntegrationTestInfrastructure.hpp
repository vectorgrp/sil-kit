// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/vendor/CreateIbRegistry.hpp"
#include "ib/IntegrationBus.hpp"
#include "ib/sim/all.hpp"
#include "MockParticipantConfiguration.hpp"

using namespace ib::mw;
using namespace ib::mw::sync;
using namespace ib::cfg;

class TestInfrastructure
{

public:
    TestInfrastructure() {}

    void ShutdownOnException(const std::exception& error)
    {
        std::stringstream ss;
        ss << "Something went wrong: " << error.what() << std::endl;
        _systemMaster.systemController->Shutdown();
        FAIL() << ss.str();
    }
    
    void SetupRegistryAndSystemMaster(uint32_t domainId, bool sync, const std::vector<std::string>& requiredParticipantNames)
    {
        try
        {
            RunRegistry(domainId);
            if (sync)
            {
                RunSystemMaster(domainId, requiredParticipantNames);
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

    void RunRegistry(uint32_t domainId)
    {
        _registry = ib::vendor::CreateIbRegistry(ib::cfg::MockParticipantConfiguration());
        _registry->ProvideDomain(domainId);
    }

    void RunSystemMaster(uint32_t domainId, const std::vector<std::string>& requiredParticipantNames)
    {
        _systemMaster.participant =
            ib::CreateParticipant(ib::cfg::MockParticipantConfiguration(), "SystemMaster", domainId);

        _systemMaster.systemController = _systemMaster.participant->GetSystemController();
        _systemMaster.systemMonitor = _systemMaster.participant->GetSystemMonitor();
        _systemMaster.systemController->SetRequiredParticipants(requiredParticipantNames);

        _systemMaster.systemMonitor->RegisterSystemStateHandler([this, requiredParticipantNames](SystemState newState) {
            switch (newState)
            {
            case SystemState::ServicesCreated:
                for (auto&& name : requiredParticipantNames)
                {
                    _systemMaster.systemController->Initialize(name);
                }
                break;
            case SystemState::ReadyToRun: _systemMaster.systemController->Run(); break;
            case SystemState::Stopped: _systemMaster.systemController->Shutdown(); break;
            case SystemState::Error: _systemMaster.systemController->Shutdown(); break;
            default: break;
            }
        });

        _systemMaster.systemMonitor->RegisterParticipantStatusHandler([this](const ParticipantStatus& newStatus) {
            switch (newStatus.state)
            {
            case ParticipantState::Error: _systemMaster.systemController->Shutdown(); break;
            default: break;
            }
        });
    }

    std::unique_ptr<ib::vendor::IIbRegistry> _registry;

    struct SystemMaster
    {
        std::unique_ptr<IParticipant> participant;
        ISystemController* systemController;
        ISystemMonitor* systemMonitor;
    };

    SystemMaster _systemMaster;

};
