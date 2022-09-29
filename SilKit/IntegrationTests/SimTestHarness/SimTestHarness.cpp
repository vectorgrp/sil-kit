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

#include <algorithm>
#include <atomic>
#include <chrono>

#include "silkit/services/orchestration/string_utils.hpp"
#include "silkit/vendor/CreateSilKitRegistry.hpp"
#include "silkit/experimental/participant/ParticipantExtensions.hpp"

#include "ConfigurationTestUtils.hpp"
#include "SimSystemController.hpp"

#ifdef SILKIT_HOURGLASS
#include "silkit/hourglass/SilKit.hpp"
#include "silkit/hourglass/config/IParticipantConfiguration.hpp"
#endif

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
    GetOrCreateLifecycleService()->Stop("Stop");
}

auto SimParticipant::GetOrCreateSystemMonitor() -> Services::Orchestration::ISystemMonitor*
{
    if (!_systemMonitor)
    {
        _systemMonitor = _participant->CreateSystemMonitor();
    }
    return _systemMonitor;
}

auto SimParticipant::GetOrCreateSystemController() -> Experimental::Services::Orchestration::ISystemController*
{
    if (!_systemController)
    {
        _systemController = Experimental::Participant::CreateSystemController(_participant.get());
    }
    return _systemController;
}

auto SimParticipant::GetOrCreateLifecycleService() -> Services::Orchestration::ILifecycleService*
{
    if (!_lifecycleService)
    {
        _lifecycleService =
            _participant->CreateLifecycleService({SilKit::Services::Orchestration::OperationMode::Coordinated});
    }
    return _lifecycleService;
}

auto SimParticipant::GetOrCreateTimeSyncService() -> Services::Orchestration::ITimeSyncService*
{
    if (!_timeSyncService)
    {
        _timeSyncService = GetOrCreateLifecycleService()->CreateTimeSyncService();
    }
    return _timeSyncService;
}

auto SimParticipant::GetOrGetLogger() -> Services::Logging::ILogger*
{
    if (!_logger)
    {
        _logger = _participant->GetLogger();
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

    _syncParticipantNames.push_back(internalSystemMonitorName);
    _simSystemController = std::make_unique<SimSystemController>(_syncParticipantNames, _registryUri);
}

SimTestHarness::~SimTestHarness() = default;

bool SimTestHarness::Run(std::chrono::nanoseconds testRunTimeout)
{
    auto lock = Lock();
    std::promise<void> simulationFinishedPromise;
    auto simulationFinishedFuture = simulationFinishedPromise.get_future();

    // Create a monitor, add it to the list of simParticipants, then start all participants
    AddParticipant(internalSystemMonitorName);
    auto monitor = _simParticipants[internalSystemMonitorName]->GetOrCreateSystemMonitor();
    monitor->AddSystemStateHandler([&](auto systemState) {
        if (systemState == SilKit::Services::Orchestration::SystemState::Shutdown)
        {
            simulationFinishedPromise.set_value();
        }
    });

    // start all participants
    for (auto& kv : _simParticipants)
    {
        auto& participant = kv.second;
        auto* lifecycleService = participant->GetOrCreateLifecycleService();
        participant->_result = lifecycleService->StartLifecycle();
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

    if (simulationFinishedFuture.wait_for(1s) != std::future_status::ready)
    {
        runStatus = false;
    }

    return runStatus;
}

SimParticipant* SimTestHarness::GetParticipant(const std::string& participantName)
{
    auto lock = Lock();
    if (_simParticipants.count(participantName) == 0)
    {
        //deferred participant creation
        auto it = std::find(_syncParticipantNames.begin(), _syncParticipantNames.end(), participantName);

        if (it == _syncParticipantNames.end())
        {
            throw SilKitError{ "SimTestHarness::GetParticipant: unknown participant " + participantName };
        }
        AddParticipant(*it);
    }
    return _simParticipants[participantName].get();
}

void SimTestHarness::AddParticipant(const std::string& participantName)
{
    auto participant = std::make_unique<SimParticipant>();
    participant->_name = participantName;

#ifdef SILKIT_HOURGLASS
    using SilKit::Hourglass::CreateParticipant;
    using SilKit::Hourglass::Config::ParticipantConfigurationFromString;
#else
    using SilKit::CreateParticipant;
    using SilKit::Config::ParticipantConfigurationFromString;
#endif

    participant->_participant = CreateParticipant(ParticipantConfigurationFromString(""), participantName, _registryUri);

    // mandatory sim task for time synced simulation
    // by default, we do no operation during simulation task, the user should override this
    auto* lifecycleService = participant->GetOrCreateLifecycleService();
    auto* timeSyncService = participant->GetOrCreateTimeSyncService();
    timeSyncService->SetSimulationStepHandler([](auto, auto) {
    }, 1ms);

    lifecycleService->SetCommunicationReadyHandler([]() {
    });

    _simParticipants[participantName] = std::move(participant);
}


} // namespace Tests
} // namespace SilKit
