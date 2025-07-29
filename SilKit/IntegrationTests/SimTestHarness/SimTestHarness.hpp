// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

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

namespace SilKit {
namespace Tests {
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
    void Disconnect();

    // Helpers to circumvent one-time-only orchestration service creation
    auto GetOrCreateSystemMonitor() -> Services::Orchestration::ISystemMonitor*;
    auto GetOrCreateSystemController() -> Experimental::Services::Orchestration::ISystemController*;
    auto GetOrCreateLifecycleService(SilKit::Services::Orchestration::LifecycleConfiguration startConfiguration =
                                         {SilKit::Services::Orchestration::OperationMode::Coordinated})
        -> Services::Orchestration::ILifecycleService*;
    auto GetOrCreateTimeSyncService() -> Services::Orchestration::ITimeSyncService*;
    auto GetOrCreateNetworkSimulator() -> Experimental::NetworkSimulation::INetworkSimulator*;
    auto GetLogger() -> Services::Logging::ILogger*;

private:
    std::string _name;
    FutureResult _result;
    std::unique_ptr<SilKit::IParticipant> _participant;

    Services::Orchestration::ISystemMonitor* _systemMonitor{nullptr};
    Experimental::Services::Orchestration::ISystemController* _systemController{nullptr};
    Services::Orchestration::ILifecycleService* _lifecycleService{nullptr};
    Services::Orchestration::ITimeSyncService* _timeSyncService{nullptr};
    Experimental::NetworkSimulation::INetworkSimulator* _networkSimulator{nullptr};
    Services::Logging::ILogger* _logger{nullptr};

    friend class SimTestHarness;
};


struct SimTestHarnessArgs
{
    std::vector<std::string> syncParticipantNames{};
    std::vector<std::string> asyncParticipantNames{};

    /// If true, the participants will be created when GetParticipant is called. If false, all participants are created
    /// in the SimTestHarness constructor.
    bool deferParticipantCreation{false};
    /// If true, the system controller is only created if CreateSystemController is called. If false, it will be created
    /// in the SimTestHarness constructor.
    bool deferSystemControllerCreation{false};

    struct
    {
        std::string participantConfiguration{""};
        std::string listenUri{"silkit://127.0.0.1:0"};
    } registry;
};


/// SimTestHarness allows creating a synchronized simulation with multiple participants.
class SimTestHarness
{
public:
    explicit SimTestHarness(const SimTestHarnessArgs& args);

    /// \deprecated Please use the single-argument constructor which takes a SimTestHarnessArgs object. Since the struct
    ///             fields are named when assigned, the resulting code should be much more readable.
    SimTestHarness(const std::vector<std::string>& syncParticipantNames, const std::string& registryUri,
                   bool deferParticipantCreation = false, bool deferSystemControllerCreation = false,
                   const std::vector<std::string>& asyncParticipantNames = std::vector<std::string>());

    ~SimTestHarness();

    void CreateSystemController();
    //! \brief Run the simulation, return false if timeout is reached.
    bool Run(std::chrono::nanoseconds testRunTimeout = std::chrono::nanoseconds::min());
    //! \brief Run the simulation for at most \p testRunTimeout and wait for all participants to shutdown, except the participants mentioned in \p keepAlive.
    //! \return false if timeout is reached
    bool Run(std::chrono::nanoseconds testRunTimeout, const std::vector<std::string>& keepAlive);

    //! \brief Get the SimParticipant by name
    SimParticipant* GetParticipant(const std::string& participantName);
    //! \brief Get the SimParticipant by name. If it does not exist yet, create a SimParticipant with the specified name and provide its ParticipantConfiguration as a string.
    SimParticipant* GetParticipant(const std::string& participantName, const std::string& participantConfiguration);

    auto GetRegistryUri() const -> std::string;
    auto GetRegistry() const -> SilKit::Vendor::Vector::ISilKitRegistry*
    {
        return _registry.get();
    }

    // clear everything and destroy participants:
    void Reset();
    void ResetRegistry();
    void ResetParticipants();

private:
    void AddParticipant(const std::string& participantName, const std::string& participantConfiguration,
                        SilKit::Services::Orchestration::LifecycleConfiguration startConfiguration = {
                            SilKit::Services::Orchestration::OperationMode::Coordinated});
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


} // namespace Tests
} // namespace SilKit
