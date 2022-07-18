// Copyright (c) Vector Informatik GmbH. All rights reserved.
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

#include "silkit/SilKit.hpp"
#include "silkit/services/orchestration/all.hpp"
#include "silkit/vendor/ISilKitRegistry.hpp"

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
    auto GetOrCreateSystemController() -> Services::Orchestration::ISystemController*;
    auto GetOrCreateLifecycleServiceNoTimeSync() -> Services::Orchestration::ILifecycleServiceNoTimeSync*;
    auto GetOrCreateLifecycleServiceWithTimeSync() -> Services::Orchestration::ILifecycleServiceWithTimeSync*;
    auto GetOrCreateLogger() -> Services::Logging::ILogger*;

private:
    std::string _name;
    FutureResult _result;
    std::unique_ptr<SilKit::IParticipant> _participant;

    Services::Orchestration::ISystemMonitor* _systemMonitor{nullptr};
    Services::Orchestration::ISystemController* _systemController{nullptr};
    Services::Orchestration::ILifecycleServiceNoTimeSync* _lifecycleServiceNoTimeSync{nullptr};
    Services::Orchestration::ILifecycleServiceWithTimeSync* _lifecycleServiceWithTimeSync{nullptr};
    Services::Logging::ILogger* _logger{nullptr};

    friend class SimTestHarness;
};


/// ! \brief SimTestHarness allows creating a synchronized simulation
///          with multiple participants. 
class SimTestHarness
{
public:
    //!< when deferParticipantCreation is true, SimParticipants will be created in the GetParticipant calls instead of in the constructor.
    SimTestHarness(const std::vector<std::string>& syncParticipantNames, const std::string& registryUri,
                   bool deferParticipantCreation = false);
    ~SimTestHarness();
    //! \brief Run the simulation, return false if timeout is reached.
    bool Run(std::chrono::nanoseconds testRunTimeout = std::chrono::nanoseconds::min());
    //! \brief Get the SimParticipant by name
    SimParticipant* GetParticipant(const std::string& participantName);

private:
    void AddParticipant(const std::string& participantName);

    std::vector<std::string> _syncParticipantNames;
    std::string _registryUri;
    std::unique_ptr<SimSystemController> _simSystemController;
    std::map<std::string, std::unique_ptr<SimParticipant>> _simParticipants;
    std::unique_ptr<SilKit::Vendor::Vector::ISilKitRegistry> _registry;
};



} //end ns silkit
} //end ns test
