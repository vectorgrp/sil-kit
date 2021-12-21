// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "ServiceDiscovery.hpp"

#include <set>

namespace ib {
namespace mw {
namespace service {

ServiceDiscovery::ServiceDiscovery(IComAdapterInternal* comadapter, const std::string& participantName)
        : _comAdapter{comadapter}
        , _participantName{participantName}
    {
    }

void ServiceDiscovery::SetEndpointAddress(const ib::mw::EndpointAddress& endpointAddress)
{
    _serviceId.legacyEpa = endpointAddress;
}
auto ServiceDiscovery::EndpointAddress() const -> const ib::mw::EndpointAddress&
{
    return _serviceId.legacyEpa;
}

void ServiceDiscovery::SetServiceId(const mw::ServiceId& serviceId)
{
    _serviceId = serviceId;
}

auto ServiceDiscovery::GetServiceId() const -> const mw::ServiceId&
{
    return _serviceId;
}

void ServiceDiscovery::ReceiveIbMessage(const IIbServiceEndpoint* from, const ServiceAnnouncement& msg)
{
    auto serviceCreated = [this](auto&& service) {
        for (auto&& handler : _handlers)
        {
            handler(Type::ServiceCreated, service);
        }
    };
    auto serviceRemoved = [this](auto&& service) {
        for (auto&& handler : _handlers)
        {
            handler(Type::ServiceRemoved, service);
        }
    };

    std::set<std::string> currentServices;
    auto& serviceMap = _announcedServices[msg.participantName];
    for (const auto& service : msg.services)
    {
        const auto idStr = to_string(service.serviceId);
        currentServices.insert(idStr);
        //if the service is not in the cache add it and notify 
        if (serviceMap.count(idStr) == 0)
        {
            serviceMap[idStr] = service;
            serviceCreated(service);
        }
        else
        {
            // it is in the cache, notify if it was modified
            auto& oldService = serviceMap[idStr];
            if (oldService != service)
            {
                serviceMap[idStr] = service;
                serviceCreated(service);
            }
        }
    }
    // now see if there are stale entries in the serviceMap, and notify removals
    // NB: do not modify the map while iterating it 
    ServiceMap newServiceMap;
    bool serviceMapUpdated = false;
    for (auto& keyVal : serviceMap)
    {
        const auto serviceId = keyVal.first;
        if (currentServices.count(serviceId) == 0)
        {
            // no longer part of the announced services
            serviceMapUpdated = true;
            serviceRemoved(keyVal.second);
        }
        else
        {
            newServiceMap[serviceId] = keyVal.second;
        }
    }
    if (serviceMapUpdated)
    {
        serviceMap = std::move(newServiceMap);
    }
}


void ServiceDiscovery::NotifyServiceCreated(const ServiceDescription& serviceId)
{
     //update our existing service cache entry
    auto& announcementMap = _announcedServices[_participantName];
    announcementMap[to_string(serviceId.serviceId)] = serviceId; 

    _announcement.participantName = _participantName;
    _announcement.services.push_back(serviceId);
    _comAdapter->SendIbMessage(this, _announcement);//ensure message history is updated
}

void ServiceDiscovery::NotifyServiceRemoved(const ServiceDescription& removedService)
{
    for (auto i = _announcement.services.begin();
        i != _announcement.services.end();
        ++i)
    {
        if (i->serviceId == removedService.serviceId)
        {
            _announcement.services.erase(i);
            break;
        }
    }
    // send updates to other participants
    _comAdapter->SendIbMessage(this, _announcement);
}
void ServiceDiscovery::RegisterServiceDiscoveryHandler(ServiceDiscoveryHandlerT handler)
{
    _handlers.emplace_back(std::move(handler));
}

} // namespace service
} // namespace mw
} // namespace ib
