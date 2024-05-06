// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <unordered_map>

#include "ILogger.hpp"
#include "SimulatedNetworkRouter.hpp"
#include "silkit/experimental/netsim/all.hpp"

namespace SilKit {
namespace Experimental {
namespace NetworkSimulation {

class SimulatedNetworkInternal
{
public:
    SimulatedNetworkInternal(Core::IParticipantInternal* participant, const std::string& networkName,
                             SimulatedNetworkType networkType, std::unique_ptr<ISimulatedNetwork> userSimulatedNetwork);

    void CreateAndSetEventProducer();
    void AddSimulatedController(const SilKit::Core::ServiceDescriptor& serviceDescriptor,
                                ControllerDescriptor controllerDescriptor);
    void RemoveSimulatedController(const std::string& fromParticipantName, Core::EndpointId serviceId);
    
    bool HasEventProducer()
    {
        return _eventProducer != nullptr;
    }

private:

    auto LookupControllerDescriptor(const std::string& fromParticipantName, Core::EndpointId serviceId)
        -> std::pair<bool, ControllerDescriptor>;
    void RemoveControllerDescriptor(const std::string& fromParticipantName, Core::EndpointId serviceId);

    Core::IParticipantInternal* _participant = nullptr;
    SilKit::Services::Logging::ILogger* _logger;
    
    std::string _networkName;
    SimulatedNetworkType _networkType;

    std::unique_ptr<SimulatedNetworkRouter> _simulatedNetworkRouter;
    std::unique_ptr<ISimulatedNetwork> _userSimulatedNetwork;

    IEventProducer* _eventProducer{nullptr};

    // Bookkeeping to get from participantName + serviceId to controllerDescriptor
    using ControllerDescriptorByServiceId = std::unordered_map<Core::EndpointId /*serviceId*/, ControllerDescriptor>;
    using ControllerDescriptorByParticipantAndServiceId =
        std::unordered_map<std::string /*participantName*/, ControllerDescriptorByServiceId>;
    ControllerDescriptorByParticipantAndServiceId _controllerDescriptors;
};

} // namespace NetworkSimulation
} // namespace Experimental
} // namespace SilKit
