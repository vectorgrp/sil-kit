// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "ServiceDiscovery.hpp"

#include "ib/mw/sync/ISystemMonitor.hpp"

#include <set>


namespace ib {
namespace mw {
namespace service {

ServiceDiscovery::ServiceDiscovery(IComAdapterInternal* comadapter, const std::string& participantName)
    : _comAdapter{comadapter}
    , _participantName{participantName}
{
    //NB do not call Initialize() (i.e., access SystemMonitor) here, there is a
    // circular dependency between ServiceDiscovery and SystemMonitor.

    _announcement.participantName = participantName;
}

void ServiceDiscovery::SetEndpointAddress(const ib::mw::EndpointAddress& endpointAddress)
{
    _serviceDescriptor.legacyEpa = endpointAddress;
}
auto ServiceDiscovery::EndpointAddress() const -> const ib::mw::EndpointAddress&
{
    return _serviceDescriptor.legacyEpa;
}

void ServiceDiscovery::SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto ServiceDiscovery::GetServiceDescriptor() const -> const mw::ServiceDescriptor&
{
    return _serviceDescriptor;
}

void ServiceDiscovery::Initialize()
{
    auto systemMonitor = _comAdapter->GetSystemMonitor();
    systemMonitor->RegisterParticipantStatusHandler([this](auto status) {
        if (status.participantName == _participantName)
        {
            return;
        }
        _comAdapter->SendIbMessage(this, _announcement);
    });
}

void ServiceDiscovery::ReceiveIbMessage(const IIbServiceEndpoint* from, const ServiceAnnouncement& msg)
{
    auto notifyCreated = [this](auto&& service)
    {
        for (auto&& handler : _handlers)
        {
            handler(Type::ServiceCreated, service);
        }
    };
    // Service announcement are sent when a new participant becomes part of the simulation
    auto&& announcementMap = _announcedServices[msg.participantName];
    for (auto&& service : msg.services)
    {
        notifyCreated(service);
        announcementMap[to_string(service)] = service;
    }
}

void ServiceDiscovery::ReceivedServiceRemoval(const ServiceDescriptor& serviceDescriptor)
{
    auto&& announcementMap = _announcedServices[serviceDescriptor.participantName];
    auto numErased = announcementMap.erase(to_string(serviceDescriptor));
  
    if (numErased == 0)
    {
        //we only notify once per event
        return;
    }

    for (auto&& handler : _handlers)
    {
        handler(Type::ServiceRemoved, serviceDescriptor);
    }
}

void ServiceDiscovery::ReceivedServiceAddition(const ServiceDescriptor& serviceDescriptor)
{
    auto&& announcementMap = _announcedServices[serviceDescriptor.participantName];
    const auto cachedServiceKey = to_string(serviceDescriptor);

    if (announcementMap.count(cachedServiceKey) > 0)
    {
        //we already now this participant's service
        return;
    }
    
    // Update the cache
    announcementMap[cachedServiceKey] = serviceDescriptor;

    for (auto&& handler : _handlers)
    {
        handler(Type::ServiceCreated, serviceDescriptor);
    }
}

void ServiceDiscovery::ReceiveIbMessage(const IIbServiceEndpoint* from, const ServiceDiscoveryEvent& msg)
{
    if (msg.isCreated)
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
    ServiceDiscoveryEvent event;
    event.isCreated = true;
    event.service = serviceDescriptor;
    _announcedServices[_participantName][to_string(serviceDescriptor)] = serviceDescriptor;
    _announcement.services.push_back(serviceDescriptor);
    _comAdapter->SendIbMessage(this, std::move(event));
}

void ServiceDiscovery::NotifyServiceRemoved(const ServiceDescriptor& serviceDescriptor)
{
    auto&& announcementMap = _announcedServices[_participantName];

    announcementMap.erase(to_string(serviceDescriptor));

    //update our announcement table
    for (auto i = _announcement.services.begin();
        i != _announcement.services.end();
        ++i)
    {
        if (i->serviceId == serviceDescriptor.serviceId)
        {
            _announcement.services.erase(i);
            break;
        }
    }

    ServiceDiscoveryEvent event;
    event.isCreated = false;
    event.service = serviceDescriptor;
    _comAdapter->SendIbMessage(this, std::move(event));
}

void ServiceDiscovery::RegisterServiceDiscoveryHandler(ServiceDiscoveryHandlerT handler)
{
    //Notify about the existing services
    for (auto&& mapKeyval : _announcedServices)
    {
        //no self notifications
        if (mapKeyval.first == _participantName) continue;

        for (auto&& serviceKeyval : mapKeyval.second)
        {
            handler(Type::ServiceCreated, serviceKeyval.second);
        }
    }
    _handlers.emplace_back(std::move(handler));
}

} // namespace service
} // namespace mw
} // namespace ib
