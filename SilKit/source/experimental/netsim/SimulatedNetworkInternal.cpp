// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "SimulatedNetworkInternal.hpp"

#include "eventproducers/CanEventProducer.hpp"
#include "eventproducers/FlexRayEventProducer.hpp"
#include "eventproducers/EthernetEventProducer.hpp"
#include "eventproducers/LinEventProducer.hpp"

namespace SilKit {
namespace Experimental {
namespace NetworkSimulation {

SimulatedNetworkInternal::SimulatedNetworkInternal(Core::IParticipantInternal* participant,
                                                   const std::string& networkName, SimulatedNetworkType networkType,
                                                   std::unique_ptr<ISimulatedNetwork> userSimulatedNetwork)
    : _participant{participant}
    , _logger{participant->GetLogger()}
    , _networkName{networkName}
    , _networkType{networkType}
    , _userSimulatedNetwork{std::move(userSimulatedNetwork)}
{
    _simulatedNetworkRouter = std::make_unique<SimulatedNetworkRouter>(_participant, _networkName, _networkType);
}

void SimulatedNetworkInternal::CreateAndSetEventProducer()
{
    // Create EventProducer and hand over to user
    std::unique_ptr<IEventProducer> eventProducer{};

    switch (_networkType)
    {
    case SimulatedNetworkType::CAN:
        eventProducer = std::make_unique<Can::CanEventProducer>(_simulatedNetworkRouter.get());
        _eventProducer = eventProducer.get();
        _userSimulatedNetwork->SetEventProducer(std::move(eventProducer));
        break;
    case SimulatedNetworkType::FlexRay:
        eventProducer = std::make_unique<Flexray::FlexRayEventProducer>(_simulatedNetworkRouter.get());
        _eventProducer = eventProducer.get();
        _userSimulatedNetwork->SetEventProducer(std::move(eventProducer));
        break;
    case SimulatedNetworkType::Ethernet:
        eventProducer = std::make_unique<Ethernet::EthernetEventProducer>(_simulatedNetworkRouter.get());
        _eventProducer = eventProducer.get();
        _userSimulatedNetwork->SetEventProducer(std::move(eventProducer));
        break;
    case SimulatedNetworkType::LIN:
        eventProducer = std::make_unique<Lin::LinEventProducer>(_simulatedNetworkRouter.get());
        _eventProducer = eventProducer.get();
        _userSimulatedNetwork->SetEventProducer(std::move(eventProducer));
        break;
    default:
        break;
    }
}

void SimulatedNetworkInternal::AddSimulatedController(const SilKit::Core::ServiceDescriptor& serviceDescriptor,
                                                      ControllerDescriptor controllerDescriptor)
{
    auto controllerName = serviceDescriptor.GetServiceName();
    auto fromParticipantName = serviceDescriptor.GetParticipantName();
    auto serviceId = serviceDescriptor.GetServiceId();
    auto serviceDescriptorStr = serviceDescriptor.to_string();

    _controllerDescriptors[fromParticipantName][serviceId] = controllerDescriptor;

    auto userSimulatedController =
        _userSimulatedNetwork->ProvideSimulatedController(controllerDescriptor);
    
    if (userSimulatedController)
    {
        _simulatedNetworkRouter->AddSimulatedController(fromParticipantName, controllerName, serviceId,
                                                        controllerDescriptor, userSimulatedController);
    }
    else
    {
        SilKit::Services::Logging::Warn(
            _logger, "NetworkSimulation: No simulated controller was provided for controller '{}' on participant '{}'", controllerName, fromParticipantName);
    }
}

auto SimulatedNetworkInternal::LookupControllerDescriptor(const std::string& fromParticipantName,
                                                          Core::EndpointId serviceId)
    -> std::pair<bool, ControllerDescriptor>
{
    auto it_controllerDescriptorsByParticipant = _controllerDescriptors.find(fromParticipantName);
    if (it_controllerDescriptorsByParticipant != _controllerDescriptors.end())
    {
        auto it_controllerDescriptorByServiceId = it_controllerDescriptorsByParticipant->second.find(serviceId);
        if (it_controllerDescriptorByServiceId != it_controllerDescriptorsByParticipant->second.end())
        {
            return {true, it_controllerDescriptorByServiceId->second};
        }
    }
    SilKit::Services::Logging::Error(
        _logger, "NetworkSimulation: Cannot associate participant name + service Id to controller descriptor.");
    return {false, {}};
}

void SimulatedNetworkInternal::RemoveControllerDescriptor(const std::string& fromParticipantName,
                                                          Core::EndpointId serviceId)
{
    auto it_controllerDescriptorsByParticipant = _controllerDescriptors.find(fromParticipantName);
    if (it_controllerDescriptorsByParticipant != _controllerDescriptors.end())
    {
        auto it_controllerDescriptorByServiceId = it_controllerDescriptorsByParticipant->second.find(serviceId);
        if (it_controllerDescriptorByServiceId != it_controllerDescriptorsByParticipant->second.end())
        {
            it_controllerDescriptorsByParticipant->second.erase(it_controllerDescriptorByServiceId);
        }
        if (it_controllerDescriptorsByParticipant->second.empty())
        {
            _controllerDescriptors.erase(it_controllerDescriptorsByParticipant);
        }
    }
}

void SimulatedNetworkInternal::RemoveSimulatedController(const std::string& fromParticipantName, Core::EndpointId serviceId)
{
    auto controllerDescriptorLookup = LookupControllerDescriptor(fromParticipantName, serviceId);
    if (controllerDescriptorLookup.first)
    {
        _userSimulatedNetwork->SimulatedControllerRemoved(controllerDescriptorLookup.second);
        RemoveControllerDescriptor(fromParticipantName, serviceId);
        _simulatedNetworkRouter->RemoveSimulatedController(fromParticipantName, serviceId, controllerDescriptorLookup.second);
    }
}

} // namespace NetworkSimulation
} // namespace Experimental
} // namespace SilKit
