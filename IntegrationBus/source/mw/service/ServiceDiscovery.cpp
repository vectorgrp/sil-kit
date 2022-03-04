// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "ServiceDiscovery.hpp"
#include "ib/mw/logging/ILogger.hpp"

#include <set>


namespace ib {
namespace mw {
namespace service {

ServiceDiscovery::ServiceDiscovery(IComAdapterInternal* comadapter, const std::string& participantName)
    : _comAdapter{comadapter}
    , _participantName{participantName}
{
    _announcement.participantName = participantName;
}
ServiceDiscovery::~ServiceDiscovery() noexcept
{
    // We might still receive asynchronous IB messages or callbacks
    // when shutting down. we set a  guard here and prevent mutating our internal maps
    _shuttingDown = true;
}

void ServiceDiscovery::SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto ServiceDiscovery::GetServiceDescriptor() const -> const mw::ServiceDescriptor&
{
    return _serviceDescriptor;
}

void ServiceDiscovery::ReceiveIbMessage(const IIbServiceEndpoint* /*from*/, const ServiceAnnouncement& msg)
{
    if (_shuttingDown)
    {
        return; 
    }
    // Service announcement are sent when a new participant joins the simulation 
    std::unique_lock<decltype(_discoveryMx)> lock(_discoveryMx);

    auto&& announcementMap = _announcedServices[msg.participantName];
    for (auto&& service : msg.services)
    {
        const auto serviceName = to_string(service);
        if (announcementMap.count(serviceName) > 0)
        {
            //we already know this service, do not advertise it again
            continue;
        }
        else
        {
            announcementMap[serviceName] = service;
            CallHandlers(ServiceDiscoveryEvent::Type::ServiceCreated , service);
        }
    }
}

void ServiceDiscovery::ReceivedServiceRemoval(const ServiceDescriptor& serviceDescriptor)
{
    std::unique_lock<decltype(_discoveryMx)> lock(_discoveryMx);

    auto&& announcementMap = _announcedServices[serviceDescriptor.GetParticipantName()];
    auto numErased = announcementMap.erase(to_string(serviceDescriptor));

    if (numErased == 0)
    {
        //we only notify once per event
        return;
    }

    CallHandlers(ServiceDiscoveryEvent::Type::ServiceRemoved, serviceDescriptor);
}

void ServiceDiscovery::ReceivedServiceAddition(const ServiceDescriptor& serviceDescriptor)
{
    std::unique_lock<decltype(_discoveryMx)> lock(_discoveryMx);

    auto&& announcementMap = _announcedServices[serviceDescriptor.GetParticipantName()];
    const auto cachedServiceKey = to_string(serviceDescriptor);

    if (announcementMap.count(cachedServiceKey) > 0)
    {
        //we already now this participant's service
        return;
    }
        
    // A remote participant might be unknown, however, it will send an event for its own ServiceDiscovery service
    // when first joining the simulation. React by announcing all services of this participant
    std::string supplControllerType;
    if (serviceDescriptor.GetSupplementalDataItem(mw::service::controllerType, supplControllerType)
        && supplControllerType == mw::service::controllerTypeServiceDiscovery)
    {
        _comAdapter->SendIbMessage(this, serviceDescriptor.GetParticipantName(), _announcement);

    }

    // Update the cache
    announcementMap[cachedServiceKey] = serviceDescriptor;

    CallHandlers(ServiceDiscoveryEvent::Type::ServiceCreated, serviceDescriptor);
}


void ServiceDiscovery::CallHandlers(ServiceDiscoveryEvent::Type eventType, const ServiceDescriptor& serviceDescriptor) const
{
    // CallHandlers must be used with a lock on _discoveryMx
    for (auto&& handler : _handlers)
    {
        handler(eventType, serviceDescriptor);
    }
}

void ServiceDiscovery::ReceiveIbMessage(const IIbServiceEndpoint* /*from*/, const ServiceDiscoveryEvent& msg)
{
    if (_shuttingDown)
    {
        return; 
    }

    if (msg.type == ServiceDiscoveryEvent::Type::ServiceCreated)
    {
        ReceivedServiceAddition(msg.service);
    }
    else
    {
        ReceivedServiceRemoval(msg.service);
    }
}

void ServiceDiscovery::NotifyServiceCreated(const ServiceDescriptor& serviceDescriptor)
{
    if (_shuttingDown)
    {
        return; 
    }

    ServiceDiscoveryEvent event;
    event.type = ServiceDiscoveryEvent::Type::ServiceCreated;
    event.service = serviceDescriptor;

    std::unique_lock<decltype(_discoveryMx)> lock(_discoveryMx);
    _announcedServices[_participantName][to_string(serviceDescriptor)] = serviceDescriptor;
    _announcement.services.push_back(serviceDescriptor);

    _comAdapter->SendIbMessage(this, std::move(event));
}

void ServiceDiscovery::NotifyServiceRemoved(const ServiceDescriptor& serviceDescriptor)
{
    if (_shuttingDown)
    {
        return; 
    }
    std::unique_lock<decltype(_discoveryMx)> lock(_discoveryMx);

    auto&& announcementMap = _announcedServices[_participantName];
    announcementMap.erase(to_string(serviceDescriptor));

    //update our announcement table
    for (auto i = _announcement.services.begin();
        i != _announcement.services.end();
        ++i)
    {
        if (i->GetServiceId() == serviceDescriptor.GetServiceId())
        {
            _announcement.services.erase(i);
            break;
        }
    }

    ServiceDiscoveryEvent event;
    event.type = ServiceDiscoveryEvent::Type::ServiceRemoved;
    event.service = serviceDescriptor;
    _comAdapter->SendIbMessage(this, std::move(event));
}

std::vector<ServiceDescriptor> ServiceDiscovery::GetRemoteServices() const
{
    std::unique_lock<decltype(_discoveryMx)> lock(_discoveryMx);
    std::vector<ServiceDescriptor> createdServices;
    for (auto&& mapKeyval : _announcedServices)
    {
        //no self notifications
        if (mapKeyval.first == _participantName)
            continue;

        for (auto&& serviceKeyval : mapKeyval.second)
        {
            createdServices.push_back(serviceKeyval.second);
        }
    }
    return createdServices;
}


void ServiceDiscovery::OnParticpantShutdown(const std::string& participantName)
{
    if (participantName == _participantName)
    {
        return;
    }

    // Locally announce removal of all services from the leaving participant
    std::unique_lock<decltype(_discoveryMx)> lock(_discoveryMx);
    auto announcedIt = _announcedServices.find(participantName);
    if (announcedIt != _announcedServices.end())
    {
        for (const auto& serviceMap : (*announcedIt).second)
        {
            CallHandlers(ServiceDiscoveryEvent::Type::ServiceRemoved, serviceMap.second);
        }
        _announcedServices.erase(announcedIt);
    }
}

void ServiceDiscovery::RegisterServiceDiscoveryHandler(ServiceDiscoveryHandlerT handler)
{
    if (_shuttingDown)
    {
        return;
    }
    // Notify the new handler about the existing services and register the handler. 
    // This must be one atomic operation, as in between calls of ReceivedServiceAddition
    // in the IO-Worker thread leads to loss of ServiceDiscoveryEvents.

    std::unique_lock<decltype(_discoveryMx)> lock(_discoveryMx);
    for (auto&& mapKeyval : _announcedServices)
    {
        //no self notifications
        if (mapKeyval.first == _participantName)
            continue;

        for (auto&& serviceKeyval : mapKeyval.second)
        {
            handler(ServiceDiscoveryEvent::Type::ServiceCreated, serviceKeyval.second);
        }
    }
    _handlers.emplace_back(std::move(handler));
}

} // namespace service
} // namespace mw
} // namespace ib
