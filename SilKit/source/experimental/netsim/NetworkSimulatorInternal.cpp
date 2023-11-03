// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <future>

#include "NetworkSimulatorInternal.hpp"
#include "NetworkSimulatorDatatypesInternal.hpp"
#include "silkit/experimental/netsim/string_utils.hpp"

#include "ILogger.hpp"
#include "procs/IParticipantReplies.hpp"

namespace SilKit {
namespace Experimental {
namespace NetworkSimulation {

NetworkSimulatorInternal::NetworkSimulatorInternal(Core::IParticipantInternal* participant)
    : _participant{participant}
    , _logger{participant->GetLogger()}
{
    _nextControllerDescriptor = 0;
    _networkSimulatorStarted = false;
}

// INetworkSimulator

void NetworkSimulatorInternal::SimulateNetwork(const std::string& networkName, SimulatedNetworkType networkType,
                                               std::unique_ptr<ISimulatedNetwork> simulatedNetwork)
{
    if (_networkSimulatorStarted)
    {
        SilKit::Services::Logging::Warn(_logger, "SimulateNetwork() must not be used after Start().");
        return;
    }

    _logger->Debug("SimulateNetwork '" + networkName + "' of type " + to_string(networkType));
    CreateSimulatedNetwork(networkName, networkType, std::move(simulatedNetwork));
}

void NetworkSimulatorInternal::Start() 
{
    if (_networkSimulatorStarted)
    {
        SilKit::Services::Logging::Warn(_logger, "Start() has already been called.");
        return;
    }

    _networkSimulatorStarted = true;

    if (_simulatedNetworks.empty())
    {
        SilKit::Services::Logging::Warn(_logger, "NetworkSimulator was started without any simulated networks.");
    }

    // Register the service discovery AFTER the network simulator has been registered.
    // Otherwise, the discovery events cannot be processed by the network simulator
    auto disco = _participant->GetServiceDiscovery();
    disco->RegisterServiceDiscoveryHandler([this](Core::Discovery::ServiceDiscoveryEvent::Type discoveryType,
                                                    const Core::ServiceDescriptor& serviceDescriptor) {
        DiscoveryHandler(discoveryType, serviceDescriptor);
    });

    // Barrier to guarantee catching two network simulators on the same simulated network
    std::promise<void> netSimStartBarrier{};
    _participant->GetParticipantRepliesProcedure()->CallAfterAllParticipantsReplied([&netSimStartBarrier]() {
        netSimStartBarrier.set_value();
    });
    netSimStartBarrier.get_future().wait();
}

// INetworkSimulatorInternal

auto NetworkSimulatorInternal::GetServiceDescriptorString(ControllerDescriptor controllerDescriptor) -> std::string const
{
    auto serviceDescriptor_it = _serviceDescriptorByControllerDescriptor.find(controllerDescriptor);
    if (serviceDescriptor_it == _serviceDescriptorByControllerDescriptor.end())
    {
        SilKit::Services::Logging::Warn(_logger,
                                        "GetServiceDescriptorString queried with an unknown controllerDescriptor.");
        return "";
    }
    return serviceDescriptor_it->second.to_string();
}

// Private

auto NetworkSimulatorInternal::NextControllerDescriptor() -> uint64_t
{
    // NetworkSimulator maintains the ControllerDescriptors, only accessible via cast to internal
    // These are unique per NetworkSimulator and thus per participant (only one NetworkSimulator allowed per participant)

    // Start at 1, as SilKit_Experimental_ControllerDescriptor_Invalid is 0
    return ++_nextControllerDescriptor;
}

void NetworkSimulatorInternal::CreateSimulatedNetwork(const std::string& networkName, SimulatedNetworkType networkType,
                                                      std::unique_ptr<ISimulatedNetwork> userSimulatedNetwork)
{
    auto simulatedNetworkInternal =
        std::make_unique<SimulatedNetworkInternal>(_participant, networkName, networkType, std::move(userSimulatedNetwork));
    
    auto networksOfType_it = _simulatedNetworks.find(networkType);
    if (networksOfType_it != _simulatedNetworks.end())
    {
        auto simulatedNetwork_it = networksOfType_it->second.find(networkName);
        if (simulatedNetwork_it != networksOfType_it->second.end())
        {
            throw SilKitError("Network already exists.");
        }
    }
    _simulatedNetworks[networkType][networkName] = std::move(simulatedNetworkInternal);
}


auto NetworkSimulatorInternal::LookupSimulatedNetwork(const std::string& networkName, SimulatedNetworkType networkType)
    -> SimulatedNetworkInternal*
{
    auto it_simulatedNetworksByType = _simulatedNetworks.find(networkType);
    if (it_simulatedNetworksByType != _simulatedNetworks.end())
    {
        auto it_simulatedNetworkByName = it_simulatedNetworksByType->second.find(networkName);
        if (it_simulatedNetworkByName != it_simulatedNetworksByType->second.end())
        {
            return it_simulatedNetworkByName->second.get();
        }
    }
    return nullptr;
}

void NetworkSimulatorInternal::DiscoveryHandler(SilKit::Core::Discovery::ServiceDiscoveryEvent::Type discoveryType,
                                               const SilKit::Core::ServiceDescriptor& serviceDescriptor)
{
    auto configNetworkType = serviceDescriptor.GetNetworkType();
    auto networkType = ConvertNetworkTypeFromConfig(configNetworkType);
    auto networkName = serviceDescriptor.GetNetworkName();
    auto serviceType = serviceDescriptor.GetServiceType();
    auto controllerName = serviceDescriptor.GetServiceName();
    auto fromParticipantName = serviceDescriptor.GetParticipantName();
    auto serviceId = serviceDescriptor.GetServiceId();

    // We see a ServiceType::Link from someone else, so this is another netsim announcing the network
    if (serviceType == SilKit::Core::ServiceType::Link && fromParticipantName != _participant->GetParticipantName())
    {
        auto simulatedNetwork = LookupSimulatedNetwork(networkName, networkType);
        if (simulatedNetwork)
        {
            auto errorMsg = "NetworkSimulation: Network '" + networkName + "' is already simulated by '"
                            + fromParticipantName + "'.";
            _logger->Error(errorMsg);
            throw SilKitError{errorMsg};
        }
    }
    if (serviceType != Core::ServiceType::Controller) 
    {
        return; // Only remote controllers
    }
    if (!AllowedNetworkTypes.count(configNetworkType)) 
    {
        return; // Only allowed network types
    }
    if (networkType == SimulatedNetworkType::Undefined)
    {
        throw SilKitError{"NetworkSimulation: Discovered network of undefined type"};
    }
    if (networkName.empty())
    {
        throw SilKitError{"NetworkSimulation: Empty network name"};
    }
    if (controllerName.empty())
    {
        throw SilKitError{"NetworkSimulation: Empty controller name"};
    }

    if (discoveryType == SilKit::Core::Discovery::ServiceDiscoveryEvent::Type::ServiceCreated)
    {
        OnNetworkDiscovered(networkName, networkType);
        OnControllerDiscovered(serviceDescriptor);
    }
    else if (discoveryType == SilKit::Core::Discovery::ServiceDiscoveryEvent::Type::ServiceRemoved)
    {
        OnNetworkRemoved(networkName, networkType);
        OnControllerRemoved(fromParticipantName, networkName, networkType, serviceId);
    }
}


void NetworkSimulatorInternal::OnNetworkDiscovered(const std::string& networkName, Experimental::NetworkSimulation::SimulatedNetworkType networkType)
{
    // Count all controllers per network to remove the network at some point
    _controllerCountPerNetwork[networkName]++;

    if (_discoveredNetworks.count(networkName)) 
    {
        return; // Already known
    }

    // Add new network
    {
        std::unique_lock<decltype(_discoveredNetworksMutex)> lock{_discoveredNetworksMutex};
        _discoveredNetworks.insert(networkName);
    }

    // If the new network is simulated, hand out an event producer to the user if not already done so
    auto simulatedNetwork = LookupSimulatedNetwork(networkName, networkType);
    if (simulatedNetwork)
    {
        if (!simulatedNetwork->HasEventProducer())
        {
            simulatedNetwork->CreateAndSetEventProducer();
        }
    }
}

void NetworkSimulatorInternal::OnNetworkRemoved(const std::string& networkName,
                                               Experimental::NetworkSimulation::SimulatedNetworkType /*networkType*/)
{
    _controllerCountPerNetwork[networkName]--;

    if (_controllerCountPerNetwork[networkName] != 0)
    {
        return; // Still some controllers left on the network
    }

    // Remove network
    {
        std::unique_lock<decltype(_discoveredNetworksMutex)> lock{_discoveredNetworksMutex};
        auto it = _discoveredNetworks.find(networkName);
        if (it != _discoveredNetworks.end())
        {
            _discoveredNetworks.erase(networkName);
        }
    }
}

void NetworkSimulatorInternal::OnControllerDiscovered(const SilKit::Core::ServiceDescriptor& serviceDescriptor)
{
    auto configNetworkType = serviceDescriptor.GetNetworkType();
    auto networkType = ConvertNetworkTypeFromConfig(configNetworkType);
    auto networkName = serviceDescriptor.GetNetworkName();

    // Bookkeeping of the relation controllerDescriptor <-> serviceDescriptor
    // Increases the controllerDescriptor even if the network is not simulated
    auto nextControllerDescriptor = NextControllerDescriptor();
    auto insertResult = _serviceDescriptorByControllerDescriptor.insert({nextControllerDescriptor, serviceDescriptor});
    if (!insertResult.second)
    {
        throw SilKit::SilKitError{"ControllerDescriptor is already associated with a serviceDescriptor"};
    }

    // Add the controller on a registered SimulatedNetwork
    auto simulatedNetwork = LookupSimulatedNetwork(networkName, networkType);
    if (simulatedNetwork)
    {
        simulatedNetwork->AddSimulatedController(serviceDescriptor, nextControllerDescriptor);
    }
}

void NetworkSimulatorInternal::OnControllerRemoved(const std::string& fromParticipantName,
                                                   const std::string& networkName,
                                                   Experimental::NetworkSimulation::SimulatedNetworkType networkType,
                                                   Core::EndpointId serviceId)
{
    auto simulatedNetwork = LookupSimulatedNetwork(networkName, networkType);
    if (simulatedNetwork)
    {
        simulatedNetwork->RemoveSimulatedController(fromParticipantName, serviceId);
    }
}

} // namespace NetworkSimulation
} // namespace Experimental
} // namespace SilKit
