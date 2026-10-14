// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "silkit/SilKit.hpp"
#include "silkit/experimental/management/ManagementDatatypes.hpp"

namespace SilKit {
namespace Experimental {
namespace Management {

//! Difference between two consecutive \ref Simulations states, by simulation name.
struct SimulationsChange
{
    std::vector<std::string> created;
    std::vector<std::string> removed;
    //! Simulations whose participants or required participants changed.
    std::vector<std::string> changed;
};

using RegistryInfoHandler = std::function<void(const RegistryInfo& registryInfo)>;

using SimulationsHandler = std::function<void(const Simulations& simulations, const SimulationsChange& change)>;

//! \p reply is only valid if \p callStatus is \ref SilKit::Services::Rpc::RpcCallStatus::Success.
using ParticipantConfigurationHandler =
    std::function<void(SilKit::Services::Rpc::RpcCallStatus callStatus, const ParticipantConfigurationReply& reply)>;

/*! \brief Experimental: client of the SIL Kit management simulation, built on the public pub/sub and RPC API.
 *
 * Observes the management participant in the management simulation. Handlers are invoked on a SIL Kit thread, except
 * that the setters invoke a new handler immediately on the calling thread and the destructor invokes the handlers of
 * pending requests. Handlers must not block and must not call the setter they were registered with.
 *
 * \warning Experimental: may change or be removed without notice.
 */
class ManagementClient
{
public:
    //! \p participant must be connected to the management simulation, see \ref MakeManagementUri.
    explicit ManagementClient(std::unique_ptr<SilKit::IParticipant> participant);

    //! Invokes the handlers of pending requests with \ref SilKit::Services::Rpc::RpcCallStatus::UndefinedError before
    //! returning; they must not use the client. No handler is invoked afterwards.
    ~ManagementClient();

    ManagementClient(const ManagementClient&) = delete;
    ManagementClient& operator=(const ManagementClient&) = delete;

    /*! \brief Sets the handler for the \ref RegistryInfo published by the management participant.
     *
     * If the info is already known, the handler is invoked immediately. If the registry has no management participant
     * (older SIL Kit or `Experimental.Management.Enable` not set), the handler is never invoked.
     */
    void SetRegistryInfoHandler(RegistryInfoHandler handler);

    /*! \brief Sets the handler for changes of the simulations on the registry.
     *
     * If a state is already known, the handler is invoked immediately, reporting all simulations as created.
     */
    void SetSimulationsHandler(SimulationsHandler handler);

    //! Requests the configuration a participant shared with the registry; \p handler is invoked once.
    void GetParticipantConfiguration(const std::string& simulationName, const std::string& participantName,
                                     std::chrono::nanoseconds timeout, ParticipantConfigurationHandler handler);

private:
    static auto Diff(const Simulations& previous, const Simulations& next) -> SimulationsChange;

    void OnRegistryInfo(const RegistryInfo& registryInfo);
    void OnSimulations(const Simulations& simulations);
    void OnCallResult(const SilKit::Services::Rpc::RpcCallResultEvent& event);

private:
    std::mutex _mutex;
    bool _hasRegistryInfo{false};
    RegistryInfo _registryInfo;
    RegistryInfoHandler _registryInfoHandler;
    bool _hasSimulations{false};
    Simulations _simulations;
    SimulationsHandler _simulationsHandler;

    std::mutex _callsMutex;
    uintptr_t _nextCallId{1};
    std::map<uintptr_t, ParticipantConfigurationHandler> _pendingCalls;

    std::unique_ptr<SilKit::IParticipant> _participant;
    SilKit::Services::Rpc::IRpcClient* _participantConfigurationClient{nullptr};
};

//! A participant name for a client in the management simulation, unique with high probability.
inline auto MakeClientParticipantName() -> std::string
{
    std::random_device randomDevice;
    std::ostringstream name;
    name << "client-" << std::hex << randomDevice() << randomDevice();
    return name.str();
}

//! Creates a \ref ManagementClient for the registry at \p registryUri (any simulation path is ignored).
inline auto CreateManagementClient(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfiguration,
                                   const std::string& registryUri) -> std::unique_ptr<ManagementClient>
{
    return std::make_unique<ManagementClient>(SilKit::CreateParticipant(
        std::move(participantConfiguration), MakeClientParticipantName(), MakeManagementUri(registryUri)));
}

// ================================================================================
//  Inline Implementations
// ================================================================================

inline ManagementClient::ManagementClient(std::unique_ptr<SilKit::IParticipant> participant)
    : _participant{std::move(participant)}
{
    _participant->CreateDataSubscriber(
        "ManagementRegistryInfo", SilKit::Services::PubSub::PubSubSpec{Registry::InfoTopic(), MediaType()},
        [this](SilKit::Services::PubSub::IDataSubscriber*, const SilKit::Services::PubSub::DataMessageEvent& event) {
        try
        {
            OnRegistryInfo(Deserialize<RegistryInfo>(event.data));
        }
        catch (const std::exception&)
        {
            // ignore malformed messages
        }
    });

    _participant->CreateDataSubscriber(
        "ManagementSimulations", SilKit::Services::PubSub::PubSubSpec{Registry::SimulationsTopic(), MediaType()},
        [this](SilKit::Services::PubSub::IDataSubscriber*, const SilKit::Services::PubSub::DataMessageEvent& event) {
        try
        {
            OnSimulations(Deserialize<Simulations>(event.data));
        }
        catch (const std::exception&)
        {
            // ignore malformed messages
        }
    });

    _participantConfigurationClient = _participant->CreateRpcClient(
        "ManagementGetParticipantConfiguration",
        SilKit::Services::Rpc::RpcSpec{Registry::GetParticipantConfigurationFunction(), MediaType()},
        [this](SilKit::Services::Rpc::IRpcClient*, const SilKit::Services::Rpc::RpcCallResultEvent& event) {
        OnCallResult(event);
    });
}

inline ManagementClient::~ManagementClient()
{
    // stops all callbacks, so the remaining requests are never answered
    _participant.reset();

    for (auto& pendingCall : _pendingCalls)
    {
        if (pendingCall.second)
        {
            pendingCall.second(SilKit::Services::Rpc::RpcCallStatus::UndefinedError, ParticipantConfigurationReply{});
        }
    }
}

inline void ManagementClient::SetRegistryInfoHandler(RegistryInfoHandler handler)
{
    std::lock_guard<std::mutex> lock{_mutex};
    _registryInfoHandler = std::move(handler);
    if (_registryInfoHandler && _hasRegistryInfo)
    {
        _registryInfoHandler(_registryInfo);
    }
}

inline void ManagementClient::SetSimulationsHandler(SimulationsHandler handler)
{
    std::lock_guard<std::mutex> lock{_mutex};
    _simulationsHandler = std::move(handler);
    if (_simulationsHandler && _hasSimulations)
    {
        _simulationsHandler(_simulations, Diff(Simulations{}, _simulations));
    }
}

inline void ManagementClient::GetParticipantConfiguration(const std::string& simulationName,
                                                          const std::string& participantName,
                                                          std::chrono::nanoseconds timeout,
                                                          ParticipantConfigurationHandler handler)
{
    uintptr_t callId{};
    {
        std::lock_guard<std::mutex> lock{_callsMutex};
        callId = _nextCallId++;
        _pendingCalls[callId] = std::move(handler);
    }

    ParticipantConfigurationRequest request;
    request.simulationName = simulationName;
    request.participantName = participantName;
    _participantConfigurationClient->CallWithTimeout(Serialize(request), timeout, reinterpret_cast<void*>(callId));
}

inline auto ManagementClient::Diff(const Simulations& previous, const Simulations& next) -> SimulationsChange
{
    std::map<std::string, const SimulationInfo*> previousByName;
    for (const auto& simulation : previous.simulations)
    {
        previousByName[simulation.name] = &simulation;
    }

    SimulationsChange change;
    for (const auto& simulation : next.simulations)
    {
        const auto it = previousByName.find(simulation.name);
        if (it == previousByName.end())
        {
            change.created.push_back(simulation.name);
            continue;
        }

        if (it->second->participantNames != simulation.participantNames
            || it->second->requiredParticipantNames != simulation.requiredParticipantNames)
        {
            change.changed.push_back(simulation.name);
        }
        previousByName.erase(it);
    }

    for (const auto& simulation : previousByName)
    {
        change.removed.push_back(simulation.first);
    }
    return change;
}

inline void ManagementClient::OnRegistryInfo(const RegistryInfo& registryInfo)
{
    std::lock_guard<std::mutex> lock{_mutex};
    _registryInfo = registryInfo;
    _hasRegistryInfo = true;
    if (_registryInfoHandler)
    {
        _registryInfoHandler(_registryInfo);
    }
}

inline void ManagementClient::OnSimulations(const Simulations& simulations)
{
    std::lock_guard<std::mutex> lock{_mutex};
    if (_hasSimulations && simulations.sequence <= _simulations.sequence)
    {
        return;
    }

    const auto change = Diff(_simulations, simulations);
    _simulations = simulations;
    _hasSimulations = true;

    if (_simulationsHandler)
    {
        _simulationsHandler(_simulations, change);
    }
}

inline void ManagementClient::OnCallResult(const SilKit::Services::Rpc::RpcCallResultEvent& event)
{
    ParticipantConfigurationHandler handler;
    {
        std::lock_guard<std::mutex> lock{_callsMutex};
        const auto it = _pendingCalls.find(reinterpret_cast<uintptr_t>(event.userContext));
        if (it == _pendingCalls.end())
        {
            return;
        }
        handler = std::move(it->second);
        _pendingCalls.erase(it);
    }

    if (!handler)
    {
        return;
    }

    if (event.callStatus != SilKit::Services::Rpc::RpcCallStatus::Success)
    {
        handler(event.callStatus, ParticipantConfigurationReply{});
        return;
    }

    ParticipantConfigurationReply reply;
    try
    {
        reply = Deserialize<ParticipantConfigurationReply>(event.resultData);
    }
    catch (const std::exception&)
    {
        handler(SilKit::Services::Rpc::RpcCallStatus::UndefinedError, ParticipantConfigurationReply{});
        return;
    }
    handler(SilKit::Services::Rpc::RpcCallStatus::Success, reply);
}

} // namespace Management
} // namespace Experimental
} // namespace SilKit
