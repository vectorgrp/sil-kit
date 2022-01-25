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
    systemMonitor->RegisterParticipantStatusHandler([this](const mw::sync::ParticipantStatus& status) {
        std::unique_lock<std::mutex> lock(_mx);
        if (status.participantName == _participantName)
        {
            // don't talk to myself
            return;
        }
        _comAdapter->SendIbMessage(this, _announcement);
    });
}


void ServiceDiscovery::ReceiveIbMessage(const IIbServiceEndpoint* from, const ServiceAnnouncement& msg)
{
    // Service announcement are sent when a new participant joins the simulation 
    std::unique_lock<std::mutex> lock(_mx);
    auto notifyCreated = [this](auto&& service)
    {
        for (auto&& handler : _handlers)
        {
            handler(ServiceDiscoveryEvent::Type::ServiceCreated, service);
        }
    };
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
            notifyCreated(service);
            announcementMap[serviceName] = service;
        }
    }
}

void ServiceDiscovery::ReceivedServiceRemoval(const ServiceDescriptor& serviceDescriptor)
{
    {
        std::unique_lock<std::mutex> lock(_mx);
        auto&& announcementMap = _announcedServices[serviceDescriptor.participantName];
        auto numErased = announcementMap.erase(to_string(serviceDescriptor));

        if (numErased == 0)
        {
            //we only notify once per event
            return;
        }
    }

    for (auto&& handler : _handlers)
    {
        handler(ServiceDiscoveryEvent::Type::ServiceRemoved, serviceDescriptor);
    }
}

void ServiceDiscovery::ReceivedServiceAddition(const ServiceDescriptor& serviceDescriptor)
{
    {
        std::unique_lock<std::mutex> lock(_mx);
        auto&& announcementMap = _announcedServices[serviceDescriptor.participantName];
        const auto cachedServiceKey = to_string(serviceDescriptor);

        if (announcementMap.count(cachedServiceKey) > 0)
        {
            //we already now this participant's service
            return;
        }

        // Update the cache
        announcementMap[cachedServiceKey] = serviceDescriptor;

    }
    for (auto&& handler : _handlers)
    {
        handler(ServiceDiscoveryEvent::Type::ServiceCreated, serviceDescriptor);
    }
}

void ServiceDiscovery::ReceiveIbMessage(const IIbServiceEndpoint* from, const ServiceDiscoveryEvent& msg)
{
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
    ServiceDiscoveryEvent event;
    event.type = ServiceDiscoveryEvent::Type::ServiceCreated;
    event.service = serviceDescriptor;
    {
        std::unique_lock<std::mutex> lock(_mx);
        _announcedServices[_participantName][to_string(serviceDescriptor)] = serviceDescriptor;
        _announcement.services.push_back(serviceDescriptor);
    }
    _comAdapter->SendIbMessage(this, std::move(event));
}

void ServiceDiscovery::NotifyServiceRemoved(const ServiceDescriptor& serviceDescriptor)
{
    {
        std::unique_lock<std::mutex> lock(_mx);
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
    }

    ServiceDiscoveryEvent event;
    event.type = ServiceDiscoveryEvent::Type::ServiceRemoved;
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
            handler(ServiceDiscoveryEvent::Type::ServiceCreated, serviceKeyval.second);
        }
    }
    _handlers.emplace_back(std::move(handler));
}

} // namespace service
} // namespace mw
} // namespace ib
