// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "SimTestHarness.hpp"
#include "SimSystemController.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>

#include "ib/mw/sync/string_utils.hpp"
#include "ib/vendor/CreateIbRegistry.hpp"

#include "ConfigurationTestUtils.hpp"

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

ib::mw::IParticipant* SimParticipant::Participant() const
{
    return _participant.get();
}

std::future<ib::mw::sync::ParticipantState>& SimParticipant::Result()
{
    return _result;
}

void SimParticipant::Stop()
{
    Participant()->GetSystemController()->Stop();
}



////////////////////////////////////////
// SimTestHarness
////////////////////////////////////////
SimTestHarness::SimTestHarness(const std::vector<std::string>& syncParticipantNames, const std::string& registryUri,
                               bool deferParticipantCreation)
    : _syncParticipantNames{ syncParticipantNames }
    , _registryUri{registryUri}
{

    // start registry
    _registry = ib::vendor::CreateIbRegistry(ib::cfg::MakeEmptyParticipantConfiguration());
    _registry->ProvideDomain(_registryUri);

    // configure and add participants
    if (!deferParticipantCreation)
    {
        for (auto&& name : _syncParticipantNames)
        {
            AddParticipant(name);
        }
    }

    _simSystemController = std::make_unique<SimSystemController>(_syncParticipantNames, _registryUri);
}

SimTestHarness::~SimTestHarness() = default;

bool SimTestHarness::Run(std::chrono::nanoseconds testRunTimeout)
{
    // start all participants
    for (auto& kv : _simParticipants)
    {
        auto& participant = kv.second;
        auto* lifecycleService = participant->Participant()->GetLifecycleService();
        participant->_result = lifecycleService->StartLifecycleWithSyncTime(lifecycleService->GetTimeSyncService(), {true, true});
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
            std::cout << "SimTestHarness: participant " << participant->Name() << ": timeout " << testRunTimeout.count()
                      << " reached. Stopping." << std::endl;
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

    participant->_participant =
        ib::CreateParticipant(ib::cfg::MakeEmptyParticipantConfiguration(), participantName, _registryUri);

    //    Let's make sure the SystemController is cached, in case the user
    //    needs it during simulation (e.g., calling Stop()).
    auto* systemCtrl = participant->Participant()->GetSystemController();
    (void)systemCtrl;

    // mandatory sim task for time synced simulation
    // by default, we do no operation during simulation task, the user should override this
    auto* lifecycleService = participant->Participant()->GetLifecycleService();
    auto* timeSyncService = lifecycleService->GetTimeSyncService();
    timeSyncService->SetSimulationTask([name = participant->Name()](auto, auto) {
    });

    lifecycleService->SetCommunicationReadyHandler([name = participantName]() {
    });

    _simParticipants[participantName] = std::move(participant);
}


} // namespace test
} // namespace ib
