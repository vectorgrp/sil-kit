// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "SimTestHarness.hpp"
#include "ParticipantConfiguration.hpp"

#include <algorithm>
#include <chrono>

#include "ib/mw/sync/string_utils.hpp"
#include "ib/extensions/CreateExtension.hpp"


using namespace std::literals::chrono_literals;

namespace 
{
auto Now()
{
    auto now = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(now);
}

} // namespace
namespace ib {
namespace test {

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
    SimSystemController(const std::vector<std::string>& syncParticipantNames, uint32_t domainId) : _syncParticipantNames{syncParticipantNames}
    {
        _comAdapter =
            ib::CreateSimulationParticipant(ib::cfg::CreateDummyConfiguration(), "SystemController", domainId, false);

        _controller = _comAdapter->GetSystemController();
        _monitor = _comAdapter->GetSystemMonitor();
        _monitor->SetSynchronizedParticipants(_syncParticipantNames);

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
        for (const auto& name : _syncParticipantNames)
        {
            std::cout << "SimTestHarness: Sending ParticipantCommand::Init to participant \""
                << name << "\"" << std::endl;
            _controller->Initialize(name);
        }
    }

    void OnParticipantStatusChanged(ib::mw::sync::ParticipantStatus status)
    {
        if (_participantStates.count(status.participantName) > 0
            && _participantStates[status.participantName] != status.state)
        {
            std::cout << "SimTestHarness: participant state of " << status.participantName << " is now " << status.state
                      << std::endl;
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
        std::cout << "SimTestHarness: System State is now " << state << std::endl;
        switch (state)
        {
        case ib::mw::sync::SystemState::Idle:
            InitializeAll();
            return;
        case ib::mw::sync::SystemState::Initialized:
            _controller->Run();
            return;
        case ib::mw::sync::SystemState::Running:
            return;
        case ib::mw::sync::SystemState::Stopped:
            _controller->Shutdown();
            return;
        default:
            return;
        }
    }
private:
    ib::mw::sync::ISystemController* _controller;
    ib::mw::sync::ISystemMonitor* _monitor;
    std::unique_ptr<ib::mw::IComAdapter> _comAdapter;
    std::vector<std::string> _syncParticipantNames;
    std::map<std::string, ib::mw::sync::ParticipantState> _participantStates; //for printing status updates
};

////////////////////////////////////////
// SimTestHarness
////////////////////////////////////////
SimTestHarness::SimTestHarness(const std::vector<std::string>& syncParticipantNames, uint32_t domainId, bool deferParticipantCreation)
    : _syncParticipantNames{ syncParticipantNames }
    , _domainId{domainId}
{

    // start registry
    ib::cfg::Config dummyCfg;
    _registry = ib::extensions::CreateIbRegistry(dummyCfg);
    _registry->ProvideDomain(_domainId);

    // configure and add participants
    if (!deferParticipantCreation)
    {
        for (auto&& name : _syncParticipantNames)
        {
            AddParticipant(name);
        }
    }

    _simSystemController = std::make_unique<SimSystemController>(_syncParticipantNames, _domainId);
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
    //C++17: for (auto& [name, participant] : _simParticipants)
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
            std::cout << "SimTestHarness: participant "
                << participant->Name()
                << ": timeout " << testRunTimeout.count() << " reached. Stopping." << std::endl;
            participant->Stop();
            timeRemaining = 0s;
            continue;
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
    if (_simParticipants.count(participantName) == 0)
    {
        //deferred participant creation
        auto it = std::find(_syncParticipantNames.begin(), _syncParticipantNames.end(), participantName);
                
        if (it == _syncParticipantNames.end())
        {
            throw std::runtime_error{ "SimTestHarness::GetParticipant: unknown participant " + participantName };
        }
        AddParticipant(*it);
    }
    return _simParticipants[participantName].get();
}

void SimTestHarness::AddParticipant(const std::string& participantName)
{
    auto participant = std::make_unique<SimParticipant>();
    participant->_name = participantName;
    
    participant->_comAdapter = 
        ib::CreateSimulationParticipant(ib::cfg::CreateDummyConfiguration(), _domainId, participantName, true);

    //    Let's make sure the SystemController is cached, in case the user
    //    needs it during simulation (e.g., calling Stop()).
    auto* systemCtrl = participant->ComAdapter()->GetSystemController();
    (void)systemCtrl;

    // mandatory sim task for time synced simulation
    // by default, we do no operation during simulation task, the user should override this
    auto* partCtrl = participant->ComAdapter()->GetParticipantController();
    partCtrl->SetSimulationTask([name = participant->Name()](auto, auto) {
        //std::cout << name << ": SimulationTask not defined!" << std::endl;
    });

    partCtrl->SetInitHandler([name = participantName](auto){
        std::cout << "SimTestHarness: "  << name << " Init was called!" << std::endl;
    });

    _simParticipants[participantName] = std::move(participant);
}


} // namespace test
} // namespace ib
