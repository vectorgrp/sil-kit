// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <mutex>
#include <set>
#include <unordered_map>
#include <deque>

#include "silkit/experimental/netsim/INetworkSimulator.hpp"
#include "INetworkSimulatorInternal.hpp"

#include "IParticipantInternal.hpp"
#include "IServiceDiscovery.hpp"
#include "ILogger.hpp"

#include "SimulatedNetworkRouter.hpp"
#include "SimulatedNetworkInternal.hpp"

namespace SilKit {
namespace Experimental {
namespace NetworkSimulation {

static const std::set<SilKit::Config::NetworkType> AllowedNetworkTypes{
    SilKit::Config::NetworkType::CAN,
    SilKit::Config::NetworkType::LIN,
    SilKit::Config::NetworkType::FlexRay,
    SilKit::Config::NetworkType::Ethernet,
};

class NetworkSimulatorInternal
    : public INetworkSimulator
    , public INetworkSimulatorInternal
{
public:
    NetworkSimulatorInternal(Core::IParticipantInternal* participant);

    // INetworkSimulator
    void SimulateNetwork(const std::string& networkName, SimulatedNetworkType networkType,
                         std::unique_ptr<ISimulatedNetwork> simulatedNetwork) override;

    void Start() override;

    // INetworkSimulatorInternal
    auto GetServiceDescriptorString(ControllerDescriptor controllerDescriptor) -> std::string const override;
     

private:

    void CreateSimulatedNetwork(const std::string& networkName, SimulatedNetworkType networkType,
                                std::unique_ptr<ISimulatedNetwork> userSimulatedNetwork);

    void DiscoveryHandler(Core::Discovery::ServiceDiscoveryEvent::Type type,
                          const Core::ServiceDescriptor& serviceDescriptor);

    void OnNetworkDiscovered(const std::string& networkName, const Experimental::NetworkSimulation::SimulatedNetworkType networkType);
    void OnNetworkRemoved(const std::string& networkName, const Experimental::NetworkSimulation::SimulatedNetworkType networkType);

    void OnControllerDiscovered(const SilKit::Core::ServiceDescriptor& serviceDescriptor);
    void OnControllerRemoved(const std::string& fromParticipantName, const std::string& networkName,
                             Experimental::NetworkSimulation::SimulatedNetworkType networkType,
                             Core::EndpointId serviceId);

    auto LookupSimulatedNetwork(const std::string& networkName, SimulatedNetworkType networkType)
        -> SimulatedNetworkInternal*;

    ControllerDescriptor NextControllerDescriptor();

    Core::IParticipantInternal* _participant = nullptr;
    SilKit::Services::Logging::ILogger* _logger;
    std::mutex _discoveredNetworksMutex;
    std::set<std::string> _discoveredNetworks;
    std::unordered_map<std::string, size_t> _controllerCountPerNetwork;

    using SimulatedNetworksByNetworkName =
        std::unordered_map<std::string /*networkName*/, std::unique_ptr<SimulatedNetworkInternal>>;
    using SimulatedNetworksByTypeAndNetworkName =
        std::unordered_map<SimulatedNetworkType /*networkType*/, SimulatedNetworksByNetworkName>;
    SimulatedNetworksByTypeAndNetworkName _simulatedNetworks;

    std::atomic<bool> _networkSimulatorStarted;

    std::unordered_map<ControllerDescriptor, Core::ServiceDescriptor> _serviceDescriptorByControllerDescriptor;
    std::atomic<uint64_t> _nextControllerDescriptor{0};
};

} // namespace NetworkSimulation
} // namespace Experimental
} // namespace SilKit
