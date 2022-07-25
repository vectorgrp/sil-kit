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
#include "SimTestHarness.hpp"
#include "SimSystemController.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>

#include "silkit/services/orchestration/string_utils.hpp"
#include "silkit/vendor/CreateSilKitRegistry.hpp"

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
namespace SilKit {
namespace Tests {

auto SimParticipant::Name() const -> const std::string&
{
    return _name;
}

auto SimParticipant::Participant() const -> SilKit::IParticipant*
{
    return _participant.get();
}

auto SimParticipant::Result() -> std::future<SilKit::Services::Orchestration::ParticipantState>&
{
    return _result;
}

void SimParticipant::Stop()
{
    GetOrCreateSystemController()->Stop();
}

auto SimParticipant::GetOrCreateSystemMonitor() -> Services::Orchestration::ISystemMonitor*
{
    if (!_systemMonitor)
    {
        _systemMonitor = _participant->CreateSystemMonitor();
    }
    return _systemMonitor;
}

auto SimParticipant::GetOrCreateSystemController() -> Services::Orchestration::ISystemController*
{
    if (!_systemController)
    {
        _systemController = _participant->CreateSystemController();
    }
    return _systemController;
}

auto SimParticipant::GetOrCreateLifecycleServiceNoTimeSync() -> Services::Orchestration::ILifecycleServiceNoTimeSync*
{
    if (!_lifecycleServiceNoTimeSync)
    {
        _lifecycleServiceNoTimeSync = _participant->CreateLifecycleServiceNoTimeSync();
    }
    return _lifecycleServiceNoTimeSync;
}

auto SimParticipant::GetOrCreateLifecycleServiceWithTimeSync() -> Services::Orchestration::ILifecycleServiceWithTimeSync*
{
    if (!_lifecycleServiceWithTimeSync)
    {
        _lifecycleServiceWithTimeSync = _participant->CreateLifecycleServiceWithTimeSync();
    }
    return _lifecycleServiceWithTimeSync;
}

auto SimParticipant::GetOrCreateLogger() -> Services::Logging::ILogger*
{
    if (!_logger)
    {
        _logger = _participant->CreateLogger();
    }
    return _logger;
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
    _registry = SilKit::Vendor::Vector::CreateSilKitRegistry(SilKit::Config::MakeEmptyParticipantConfiguration());
    _registry->StartListening(_registryUri);

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
        auto* lifecycleService = participant->GetOrCreateLifecycleServiceWithTimeSync();
        participant->_result = lifecycleService->StartLifecycle({true, true});
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
        SilKit::CreateParticipant(SilKit::Config::MakeEmptyParticipantConfiguration(), participantName, _registryUri);

    //    Let's make sure the SystemController is cached, in case the user
    //    needs it during simulation (e.g., calling Stop()).
    auto* systemCtrl = participant->GetOrCreateSystemController();
    (void)systemCtrl;

    // mandatory sim task for time synced simulation
    // by default, we do no operation during simulation task, the user should override this
    auto* lifecycleService = participant->GetOrCreateLifecycleServiceWithTimeSync();
    auto* timeSyncService = lifecycleService->GetTimeSyncService();
    timeSyncService->SetSimulationStepHandler([name = participant->Name()](auto, auto) {
    }, 1ms);

    lifecycleService->SetCommunicationReadyHandler([name = participantName]() {
    });

    _simParticipants[participantName] = std::move(participant);
}


} // namespace Tests
} // namespace SilKit
