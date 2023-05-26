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
//
#pragma once

#include <map>
#include <string>
#include <thread>
#include <functional>
#include <future>
#include <chrono>
#include <memory>
#include <cstdint>
#include <mutex>

#include "silkit/SilKit.hpp"
#include "silkit/services/orchestration/all.hpp"
#include "silkit/vendor/ISilKitRegistry.hpp"

#include "silkit/experimental/services/orchestration/ISystemController.hpp"

namespace SilKit { namespace Tests {
//forward
class SimSystemController;

////////////////////////////////////////
// SimParticipant
////////////////////////////////////////
class SimParticipant
{
public:
    using FutureResult = std::future<SilKit::Services::Orchestration::ParticipantState>;

    SimParticipant(const SimParticipant&) = delete;
    SimParticipant operator=(const SimParticipant&) = delete;
    SimParticipant() = default;

    auto Name() const -> const std::string&;
    auto Participant() const -> SilKit::IParticipant*;
    auto Result() -> FutureResult&;
    void Stop();

    // Helpers to circumvent one-time-only orchestration service creation
    auto GetOrCreateSystemMonitor() -> Services::Orchestration::ISystemMonitor*;
    auto GetOrCreateSystemController() -> Experimental::Services::Orchestration::ISystemController*;
    auto GetOrCreateLifecycleService(SilKit::Services::Orchestration::LifecycleConfiguration startConfiguration =
                                         {SilKit::Services::Orchestration::OperationMode::Coordinated})
        -> Services::Orchestration::ILifecycleService*;
    auto GetOrCreateTimeSyncService() -> Services::Orchestration::ITimeSyncService*;
    auto GetOrGetLogger() -> Services::Logging::ILogger*;

private:
    std::string _name;
    FutureResult _result;
    std::unique_ptr<SilKit::IParticipant> _participant;

    Services::Orchestration::ISystemMonitor* _systemMonitor{nullptr};
    Experimental::Services::Orchestration::ISystemController* _systemController{nullptr};
    Services::Orchestration::ILifecycleService* _lifecycleService{nullptr};
    Services::Orchestration::ITimeSyncService* _timeSyncService{nullptr};
    Services::Logging::ILogger* _logger{nullptr};

    friend class SimTestHarness;
};


/// ! \brief SimTestHarness allows creating a synchronized simulation
///          with multiple participants. 
class SimTestHarness
{
public:
    //! when deferParticipantCreation is true, SimParticipants will be created in the GetParticipant calls instead of in the constructor.
    SimTestHarness(const std::vector<std::string>& syncParticipantNames, const std::string& registryUri,
                   bool deferParticipantCreation = false, bool deferSystemControllerCreation = false,
                   const std::vector<std::string>& asyncParticipantNames = std::vector<std::string>());

    ~SimTestHarness();

    void CreateSystemController();
    //! \brief Run the simulation, return false if timeout is reached.
    bool Run(std::chrono::nanoseconds testRunTimeout = std::chrono::nanoseconds::min());
    //! \brief Get the SimParticipant by name
    SimParticipant* GetParticipant(const std::string& participantName);
    //! \brief Get the SimParticipant by name. If it does not exist yet, create a SimParticipant with the specified name and provide its ParticipantConfiguration as a string.
    SimParticipant* GetParticipant(const std::string& participantName, const std::string& participantConfiguration);

    // clear everything and destroy participants:
    void Reset();
    void ResetRegistry();
    void ResetParticipants();

private:
    void AddParticipant(const std::string& participantName, const std::string& participantConfiguration,
                        SilKit::Services::Orchestration::LifecycleConfiguration startConfiguration = {SilKit::Services::Orchestration::OperationMode::Coordinated});
    bool IsSync(const std::string& participantName);
    bool IsAsync(const std::string& participantName);

    std::mutex _mutex;
    auto Lock() -> std::unique_lock<decltype(_mutex)>
    {
        return std::unique_lock<decltype(_mutex)>{_mutex};
    }

    std::vector<std::string> _syncParticipantNames;
    std::vector<std::string> _asyncParticipantNames;
    std::string _registryUri;
    std::unique_ptr<SimSystemController> _simSystemController;
    std::map<std::string, std::unique_ptr<SimParticipant>> _simParticipants;
    std::unique_ptr<SilKit::Vendor::Vector::ISilKitRegistry> _registry;
    const std::string internalSystemMonitorName = "InternalSystemMonitor";
};



} //end ns silkit
} //end ns test
