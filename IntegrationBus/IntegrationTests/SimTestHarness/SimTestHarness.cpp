// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "SimTestHarness.hpp"

#include <algorithm>

#include "ib/extensions/CreateExtension.hpp"

namespace 
{
auto Now()
{
    auto now = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(now);
}

}
namespace ib { namespace test {

const std::string& SimParticipant::Name() const
{
    return _name;
}
ib::mw::IComAdapter* SimParticipant::ComAdapter() const
{
    return _comAdapter.get();
}

std::future<ib::mw::sync::ParticipantState>& SimParticipant::Result()
{
    return _result;
}

void SimParticipant::Stop()
{
    ComAdapter()->GetSystemController()->Stop();
}


////////////////////////////////////////
// SimSystemController
////////////////////////////////////////
class SimSystemController
{
public:
    SimSystemController() = delete;
    SimSystemController(const ib::cfg::Config& config, uint32_t domainId)
        : _cfg{config}
    {
        _comAdapter = 
            ib::CreateComAdapter(_cfg, "SystemController", domainId);

        _controller = _comAdapter->GetSystemController();
        _monitor = _comAdapter->GetSystemMonitor();

        _monitor->RegisterSystemStateHandler(
            std::bind(&SimSystemController::OnSystemStateChanged, this, std::placeholders::_1)
        );
        _monitor->RegisterParticipantStatusHandler(
            std::bind(&SimSystemController::OnParticipantStatusChanged, this, std::placeholders::_1)
        );
    }

    ~SimSystemController() = default;

    void InitializeAll()
    {
        for (const auto& participant : _cfg.simulationSetup.participants)
        {
            if (!participant.participantController)
                continue;
            std::cout << "Sending ParticipantCommand::Init to participant \""
                << participant.name << "\"" << std::endl;
            _controller->Initialize(participant.name);
        }
    }

    void OnParticipantStatusChanged(ib::mw::sync::ParticipantStatus status)
    {
        if (status.state == ib::mw::sync::ParticipantState::Stopped
            || status.state == ib::mw::sync::ParticipantState::Error)
        {
            _controller->Stop();
        }
    }

    void OnSystemStateChanged(ib::mw::sync::SystemState state)
    {
        switch (state)
        {
        case ib::mw::sync::SystemState::Idle:
        std::cout << "System Idle" << std::endl;
        InitializeAll();
        return;
        case ib::mw::sync::SystemState::Initialized:
        std::cout << "System Initialized" << std::endl;
        _controller->Run();
        return;
        case ib::mw::sync::SystemState::Running:
        std::cout << "System Running" << std::endl;
        return;
        case ib::mw::sync::SystemState::Stopped:
        std::cout << "System Shutdown" << std::endl;
        _controller->Shutdown();
        return;
        default:
        return;
        }
    }
private:
    ib::cfg::Config _cfg;

    ib::mw::sync::ISystemController* _controller;
    ib::mw::sync::ISystemMonitor* _monitor;
    std::unique_ptr<ib::mw::IComAdapter> _comAdapter;
};

////////////////////////////////////////
// SimTestHarness
////////////////////////////////////////
SimTestHarness::SimTestHarness(ib::cfg::Config config, uint32_t domainId)
    : _config{config}
    , _domainId{domainId}
{
    // we might have to declare a sync master among the participants
    bool needSyncMaster =
        _config.middlewareConfig.activeMiddleware == ib::cfg::Middleware::FastRTPS;
    for (auto& participantConfig : _config.simulationSetup.participants)
    {
        if (participantConfig.isSyncMaster)
        {
            //the user configured a sync  master
            needSyncMaster = false;
            break;
        }
    }

    if (needSyncMaster)
    {
        // no sync master was defined, so pick one at random:
        _config.simulationSetup.participants.at(0).isSyncMaster = true;
    }

    // add participant for system controller
    {
        ib::mw::ParticipantId id{0};
        for (const auto& participant : config.simulationSetup.participants)
        {
            id = ::std::max(participant.id, id);
        }
        ib::cfg::Participant scCfg{};
        scCfg.name = "SystemController";
        scCfg.id = id + 1;
        _config.simulationSetup.participants.emplace_back(std::move(scCfg));
    }
    // start registry for VAsio
    if (_config.middlewareConfig.activeMiddleware == ib::cfg::Middleware::VAsio)
    {
        _registry = ib::extensions::CreateIbRegistry(_config);
        _registry->ProvideDomain(_domainId);
    }

    // configure and add participants
    _config.simulationSetup.timeSync.tickPeriod = std::chrono::milliseconds{1};
    for (auto& participantConfig : _config.simulationSetup.participants)
    {
        if (participantConfig.name == "SystemController")
        {
            continue;
        }

        if (!participantConfig.participantController)
        {
            ib::cfg::ParticipantController pc{};
            pc.syncType = ib::cfg::SyncType::DiscreteTime;
            participantConfig.participantController = std::move(pc);
        }
        AddParticipant(participantConfig);
    }

    _simSystemController = std::make_unique<SimSystemController>(_config, _domainId);
}

SimTestHarness::~SimTestHarness() = default;

bool SimTestHarness::Run(std::chrono::nanoseconds testRunTimeout)
{
    // start all participants
    for (auto& kv : _simParticipants)
    {
        auto& participant = kv.second;
        auto* participantController = participant->ComAdapter()->GetParticipantController();
        participant->_result = participantController->RunAsync();
    }

    // wait until simulation is finished or timeout is reached
    const bool noTimeout = testRunTimeout == std::chrono::nanoseconds::min();
    bool runStatus = true;
    const auto startTime = Now();
    auto timeRemaining = testRunTimeout;
    //for (auto& [name, participant] : _simParticipants)
    for (auto& kv : _simParticipants)
    {
        auto& participant = kv.second;
        if (noTimeout)
        {
            participant->Result().wait();
            continue;
        }

        auto status = participant->Result().wait_for(timeRemaining);
        if (status == std::future_status::timeout)
        {
            runStatus = false;
        }
        auto timeSlept = Now() - startTime;
        if (timeSlept >= testRunTimeout)
        {
            // need to stop the participants
            participant->Stop();
            break;
        }
        else
        {
            timeRemaining = testRunTimeout - timeSlept;
        }

    }

    return runStatus;
}

SimParticipant* SimTestHarness::GetParticipant(const std::string& participantName)
{
    return _simParticipants[participantName].get();
}

void SimTestHarness::AddParticipant(ib::cfg::Participant participantConfig)
{

    auto participant = std::make_unique<SimParticipant>();
    participant->_name = participantConfig.name;
    participant->_comAdapter = 
        ib::CreateComAdapter(_config, participantConfig.name, _domainId);

    //    Let's make sure the SystemController is cached, in case the user
    //    needs it during simulation (e.g., calling Stop()).
    auto* systemCtrl = participant->ComAdapter()->GetSystemController();
    (void)systemCtrl;

    // mandatory sim task for time synced simulation
    // by default, we do no operation during simulation task, the user should override this
    auto* partCtrl = participant ->ComAdapter() ->GetParticipantController();
    partCtrl->SetSimulationTask([name = participant->Name()](auto, auto) {
        //std::cout << name << ": SimulationTask not defined!" << std::endl;
    });

    partCtrl->SetInitHandler([name = participantConfig.name](auto){
        std::cout << name << " Init!" << std::endl;
    });

    _simParticipants[participantConfig.name] = std::move(participant);
}


} //end ns ib
} //end ns test
