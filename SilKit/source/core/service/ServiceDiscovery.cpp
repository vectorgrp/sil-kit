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

#include "ServiceDiscovery.hpp"
#include "silkit/services/logging/ILogger.hpp"

namespace SilKit {
namespace Core {
namespace Discovery {

ServiceDiscovery::ServiceDiscovery(IParticipantInternal* participant, const std::string& participantName)
    : _participant{participant}
    , _participantName{participantName}
{
}

ServiceDiscovery::~ServiceDiscovery() noexcept
{
    // We might still receive asynchronous SIL Kit messages or callbacks
    // when shutting down. we set a  guard here and prevent mutating our internal maps
    _shuttingDown = true;
}

void ServiceDiscovery::SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto ServiceDiscovery::GetServiceDescriptor() const -> const Core::ServiceDescriptor&
{
    return _serviceDescriptor;
}

void ServiceDiscovery::ReceiveMsg(const IServiceEndpoint* /*from*/, const ParticipantDiscoveryEvent& msg)
{
    if (_shuttingDown)
    {
        return;
    }
    OnParticpantAddition(msg);
}

void ServiceDiscovery::OnParticpantAddition(const ParticipantDiscoveryEvent& msg)
{
    // Service announcement are sent when a new participant joins the simulation
    std::unique_lock<decltype(_discoveryMx)> lock(_discoveryMx);
    auto&& fromParticipant = msg.participantName;
    auto&& announcementMap = _servicesByParticipant[fromParticipant];

    for (auto&& serviceDescriptor : msg.services)
    {
        // Check if already known
        const auto serviceName = to_string(serviceDescriptor);
        if (announcementMap.count(serviceName) > 0)
        {
            continue;
        }
        else
        {
            _specificDiscoveryStore.ServiceChange(ServiceDiscoveryEvent::Type::ServiceCreated, serviceDescriptor);
            // Store by service name
            announcementMap[serviceName] = serviceDescriptor;
            CallHandlers(ServiceDiscoveryEvent::Type::ServiceCreated, serviceDescriptor);
        }
    }
}

void ServiceDiscovery::OnParticpantRemoval(const std::string& participantName)
{
    if (participantName == _participantName)
    {
        return;
    }

    // Locally announce removal of all services from the leaving participant
    std::unique_lock<decltype(_discoveryMx)> lock(_discoveryMx);
    auto announcedIt = _servicesByParticipant.find(participantName);
    if (announcedIt != _servicesByParticipant.end())
    {
        for (const auto& serviceMap : (*announcedIt).second)
        {
            _specificDiscoveryStore.ServiceChange(ServiceDiscoveryEvent::Type::ServiceRemoved, serviceMap.second);
            CallHandlers(ServiceDiscoveryEvent::Type::ServiceRemoved, serviceMap.second);
        }
        _servicesByParticipant.erase(announcedIt);
    }
}

void ServiceDiscovery::NotifyServiceCreated(const ServiceDescriptor& serviceDescriptor)
{
    if (_shuttingDown)
    {
        return;
    }

    // No self delivery for ServiceDiscoveryEvent, trigger directly in this thread context
    OnServiceAddition(serviceDescriptor);

    ServiceDiscoveryEvent event;
    event.type = ServiceDiscoveryEvent::Type::ServiceCreated;
    event.serviceDescriptor = serviceDescriptor;
    _participant->SendMsg(this, std::move(event));
}

void ServiceDiscovery::NotifyServiceRemoved(const ServiceDescriptor& serviceDescriptor)
{
    if (_shuttingDown)
    {
        return;
    }

    // No self delivery for ServiceDiscoveryEvent, trigger directly in this thread context
    OnServiceRemoval(serviceDescriptor);

    ServiceDiscoveryEvent event;
    event.type = ServiceDiscoveryEvent::Type::ServiceRemoved;
    event.serviceDescriptor = serviceDescriptor;
    _participant->SendMsg(this, std::move(event));
}

void ServiceDiscovery::ReceiveMsg(const IServiceEndpoint* /*from*/, const ServiceDiscoveryEvent& msg)
{
    if (_shuttingDown)
    {
        return;
    }

    if (msg.type == ServiceDiscoveryEvent::Type::ServiceCreated)
    {
        OnServiceAddition(msg.serviceDescriptor);
    }
    else
    {
        OnServiceRemoval(msg.serviceDescriptor);
    }
}

void ServiceDiscovery::OnServiceAddition(const ServiceDescriptor& serviceDescriptor)
{
    std::unique_lock<decltype(_discoveryMx)> lock(_discoveryMx);
    auto&& fromParticipant = serviceDescriptor.GetParticipantName();
    auto&& announcementMap = _servicesByParticipant[fromParticipant];
    const auto cachedServiceKey = to_string(serviceDescriptor);
    if (announcementMap.count(cachedServiceKey) > 0)
    {
        //we already now this participant's service
        return;
    }

    // If we receive the event from ourselves, we skip announcing ourselves
    if (fromParticipant != _participantName)
    {
        std::string supplControllerTypeName;
        serviceDescriptor.GetSupplementalDataItem(Core::Discovery::controllerType, supplControllerTypeName);

        // A remote participant might be unknown, however, it will send an event for its own ServiceDiscovery service
        // when first joining the simulation. React by announcing all services of this participant
        if (supplControllerTypeName == Core::Discovery::controllerTypeServiceDiscovery)
        {
            AnnounceLocalParticipantTo(fromParticipant);
        }
    }

    // Update the cache
    announcementMap[cachedServiceKey] = serviceDescriptor;

    _specificDiscoveryStore.ServiceChange(ServiceDiscoveryEvent::Type::ServiceCreated, serviceDescriptor);
    CallHandlers(ServiceDiscoveryEvent::Type::ServiceCreated, serviceDescriptor);
}

void ServiceDiscovery::AnnounceLocalParticipantTo(const std::string& otherParticipant)
{
    ParticipantDiscoveryEvent localServices;
    localServices.participantName = _participantName;
    localServices.services.reserve(_servicesByParticipant[_participantName].size());
    for (const auto& thisParticipantServiceMap : _servicesByParticipant[_participantName])
    {
        localServices.services.push_back(thisParticipantServiceMap.second);
    }
    _participant->SendMsg(this, otherParticipant, std::move(localServices));
}

void ServiceDiscovery::OnServiceRemoval(const ServiceDescriptor& serviceDescriptor)
{
    std::unique_lock<decltype(_discoveryMx)> lock(_discoveryMx);
    auto&& fromParticipant = serviceDescriptor.GetParticipantName();
    auto&& announcementMap = _servicesByParticipant[fromParticipant];
    auto numErased = announcementMap.erase(to_string(serviceDescriptor));
    if (numErased == 0)
    {
        //we only notify once per event
        return;
    }

    _specificDiscoveryStore.ServiceChange(ServiceDiscoveryEvent::Type::ServiceRemoved, serviceDescriptor);
    CallHandlers(ServiceDiscoveryEvent::Type::ServiceRemoved, serviceDescriptor);
}

void ServiceDiscovery::CallHandlers(ServiceDiscoveryEvent::Type eventType, const ServiceDescriptor& serviceDescriptor) const
{
    // CallHandlers must be used with a lock on _discoveryMx
    for (auto&& handler : _handlers)
    {
        handler(eventType, serviceDescriptor);
    }
}

std::vector<ServiceDescriptor> ServiceDiscovery::GetServices() const
{
    std::unique_lock<decltype(_discoveryMx)> lock(_discoveryMx);
    std::vector<ServiceDescriptor> createdServices;
    // Aggregate all known services (including from ourself)
    for (auto&& participantServiceMap : _servicesByParticipant)
    {
        for (auto&& service : participantServiceMap.second)
        {
            createdServices.push_back(service.second);
        }
    }
    return createdServices;
}

void ServiceDiscovery::RegisterServiceDiscoveryHandler(ServiceDiscoveryHandler handler)
{
    if (_shuttingDown)
    {
        return;
    }
    // Notify the new handler about the existing services and register the handler.
    // This must be one atomic operation, as in between calls of OnServiceAddition
    // in the IO-Worker thread leads to loss of ServiceDiscoveryEvents.
    std::unique_lock<decltype(_discoveryMx)> lock(_discoveryMx);
    for (auto&& participantServices : _servicesByParticipant)
    {
        for (auto&& services : participantServices.second)
        {
            handler(ServiceDiscoveryEvent::Type::ServiceCreated, services.second);
        }
    }
    _handlers.emplace_back(std::move(handler));
}

void ServiceDiscovery::RegisterSpecificServiceDiscoveryHandler(
    ServiceDiscoveryHandler handler, const std::string& controllerType_, const std::string& topic,
    const std::vector<SilKit::Services::MatchingLabel>& labels)
{
    if (_shuttingDown)
    {
        return;
    }

    std::unique_lock<decltype(_discoveryMx)> lock(_discoveryMx);
    _specificDiscoveryStore.RegisterSpecificServiceDiscoveryHandler(handler, controllerType_, topic, labels);
}

} // namespace Discovery
} // namespace Core
} // namespace SilKit
