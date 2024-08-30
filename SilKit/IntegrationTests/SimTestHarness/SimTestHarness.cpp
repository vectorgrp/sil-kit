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
#include <iostream>

#include "silkit/config/IParticipantConfiguration.hpp"
#include "silkit/experimental/participant/ParticipantExtensions.hpp"
#include "silkit/participant/IParticipant.hpp"
#include "silkit/services/orchestration/string_utils.hpp"
#include "silkit/vendor/CreateSilKitRegistry.hpp"

#include "SimSystemController.hpp"

using namespace std::literals::chrono_literals;

namespace {
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

void SimParticipant::Disconnect()
{
    _participant.reset();
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

auto SimParticipant::GetOrCreateLifecycleService(SilKit::Services::Orchestration::LifecycleConfiguration
                                                     startConfiguration) -> Services::Orchestration::ILifecycleService*
{
    if (!_lifecycleService)
    {
        _lifecycleService = _participant->CreateLifecycleService(startConfiguration);
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

auto SimParticipant::GetOrCreateNetworkSimulator() -> Experimental::NetworkSimulation::INetworkSimulator*
{
    if (!_networkSimulator)
    {
        _networkSimulator = SilKit::Experimental::Participant::CreateNetworkSimulator(_participant.get());
    }
    return _networkSimulator;
}

auto SimParticipant::GetLogger() -> Services::Logging::ILogger*
{
    if (!_logger)
    {
        _logger = _participant->GetLogger();
    }
    return _logger;
}


static auto MakeSimTestHarnessArgs(const std::vector<std::string>& syncParticipantNames, const std::string& registryUri,
                                   bool deferParticipantCreation, bool deferSystemControllerCreation,
                                   const std::vector<std::string>& asyncParticipantNames) -> SimTestHarnessArgs
{
    SimTestHarnessArgs args;
    args.syncParticipantNames = syncParticipantNames;
    args.asyncParticipantNames = asyncParticipantNames;
    args.registry.listenUri = registryUri;
    args.deferParticipantCreation = deferParticipantCreation;
    args.deferSystemControllerCreation = deferSystemControllerCreation;
    return args;
}


////////////////////////////////////////
// SimTestHarness
////////////////////////////////////////

SimTestHarness::SimTestHarness(const std::vector<std::string>& syncParticipantNames, const std::string& registryUri,
                               bool deferParticipantCreation, bool deferSystemControllerCreation,
                               const std::vector<std::string>& asyncParticipantNames)
    : SimTestHarness(MakeSimTestHarnessArgs(syncParticipantNames, registryUri, deferParticipantCreation,
                                            deferSystemControllerCreation, asyncParticipantNames))
{
}

SimTestHarness::SimTestHarness(const SilKit::Tests::SimTestHarnessArgs& args)
    : _syncParticipantNames{args.syncParticipantNames}
    , _asyncParticipantNames{args.asyncParticipantNames}
{
    // start registry
    _registry = SilKit::Vendor::Vector::CreateSilKitRegistry(
        SilKit::Config::ParticipantConfigurationFromString(args.registry.participantConfiguration));
    _registryUri = _registry->StartListening(args.registry.listenUri);

    // configure and add participants
    if (!args.deferParticipantCreation)
    {
        for (auto&& name : _syncParticipantNames)
        {
            AddParticipant(name, "");
        }
    }

    if (!args.deferSystemControllerCreation)
    {
        CreateSystemController();
    }
}

SimTestHarness::~SimTestHarness() = default;

void SimTestHarness::CreateSystemController()
{
    if (!_syncParticipantNames.empty())
    {
        _syncParticipantNames.push_back(internalSystemMonitorName);
        _simSystemController = std::make_unique<SimSystemController>(_syncParticipantNames, _registryUri);
    }
}

bool SimTestHarness::Run(std::chrono::nanoseconds testRunTimeout)
{
    return Run(testRunTimeout, {});
}

bool SimTestHarness::Run(std::chrono::nanoseconds testRunTimeout, const std::vector<std::string>& keepAlive)
{
    auto lock = Lock();
    std::promise<void> simulationFinishedPromise;
    auto simulationFinishedFuture = simulationFinishedPromise.get_future();
    if (!_syncParticipantNames.empty())
    {
        // Create a monitor, add it to the list of simParticipants, then start all participants
        AddParticipant(internalSystemMonitorName, "");
        auto monitor = _simParticipants[internalSystemMonitorName]->GetOrCreateSystemMonitor();
        monitor->AddSystemStateHandler([&](auto systemState) {
            if (systemState == SilKit::Services::Orchestration::SystemState::Shutdown)
            {
                simulationFinishedPromise.set_value();
            }
        });
    }

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
        if (std::find(keepAlive.begin(), keepAlive.end(), kv.first) != keepAlive.end())
        {
            // Ignore waiting for participants in keepAlive list
            continue;
        }

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

    if (!_syncParticipantNames.empty())
    {
        if (simulationFinishedFuture.wait_for(1s) != std::future_status::ready)
        {
            runStatus = false;
        }
    }

    return runStatus;
}

SimParticipant* SimTestHarness::GetParticipant(const std::string& participantName,
                                               const std::string& participantConfiguration)
{
    auto lock = Lock();
    if (_simParticipants.count(participantName) == 0)
    {
        //deferred participant creation
        if (!IsSync(participantName))
        {
            if (!IsAsync(participantName))
            {
                throw SilKitError{"SimTestHarness::GetParticipant: unknown participant " + participantName};
            }
            else
            {
                AddParticipant(participantName, participantConfiguration,
                               {SilKit::Services::Orchestration::OperationMode::Autonomous});
            }
        }
        else
        {
            AddParticipant(participantName, participantConfiguration);
        }
    }
    return _simParticipants[participantName].get();
}

auto SimTestHarness::GetRegistryUri() const -> std::string
{
    return _registryUri;
}

SimParticipant* SimTestHarness::GetParticipant(const std::string& participantName)
{
    return GetParticipant(participantName, "");
}

void SimTestHarness::AddParticipant(const std::string& participantName, const std::string& participantConfiguration,
                                    SilKit::Services::Orchestration::LifecycleConfiguration startConfiguration)
{
    auto participant = std::make_unique<SimParticipant>();
    participant->_name = participantName;

    participant->_participant = SilKit::CreateParticipant(
        SilKit::Config::ParticipantConfigurationFromString(participantConfiguration), participantName, _registryUri);

    // mandatory sim task for time synced simulation
    // by default, we do no operation during simulation task, the user should override this
    auto* lifecycleService = participant->GetOrCreateLifecycleService(startConfiguration);
    if (startConfiguration.operationMode == SilKit::Services::Orchestration::OperationMode::Coordinated)
    {
        auto* timeSyncService = participant->GetOrCreateTimeSyncService();
        timeSyncService->SetSimulationStepHandler([](auto, auto) {}, 1ms);
    }

    lifecycleService->SetCommunicationReadyHandler([]() {});

    _simParticipants[participantName] = std::move(participant);
}

bool SimTestHarness::IsSync(const std::string& participantName)
{
    auto it = std::find(_syncParticipantNames.begin(), _syncParticipantNames.end(), participantName);
    return (it != _syncParticipantNames.end());
}

bool SimTestHarness::IsAsync(const std::string& participantName)
{
    auto it = std::find(_asyncParticipantNames.begin(), _asyncParticipantNames.end(), participantName);
    return (it != _asyncParticipantNames.end());
}

void SimTestHarness::Reset()
{
    ResetParticipants();
    ResetRegistry();
}

void SimTestHarness::ResetRegistry()
{
    auto lock = Lock();
    _registry.reset();
    _registryUri.clear();
    _syncParticipantNames.clear();
    _asyncParticipantNames.clear();
}

void SimTestHarness::ResetParticipants()
{
    auto lock = Lock();
    _simParticipants.clear();
    if (_simSystemController)
    {
        _simSystemController.reset();
    }
}

} // namespace Tests
} // namespace SilKit
