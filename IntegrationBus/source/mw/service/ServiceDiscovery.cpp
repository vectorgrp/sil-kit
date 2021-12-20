// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "ServiceDiscovery.hpp"

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
    //NB: currently we only compute additions and a removal will trigger the assertion failure
    size_t lastAnnouncementIndex = 0;
    if (_announcementMap.find(msg.participantName) != _announcementMap.end())
    {
        // compute which services were added compared to last time we saw an announcement from participant
        const auto& oldAnnouncement = _announcementMap[msg.participantName];
        if (!(oldAnnouncement.services.size() < msg.services.size()))
        {
            throw std::runtime_error("ServiceDiscovery: ReceiveIbMessage: Assertion failed");
        }
        lastAnnouncementIndex = oldAnnouncement.services.size();
    }
    for (auto idx = lastAnnouncementIndex; idx < msg.services.size(); idx++)
    {
        const auto& service = msg.services[idx];
        for (auto&& handler : _handlers)
        {
            handler(Type::ServiceCreated, service);
        }
    }
    _announcementMap[msg.participantName] = msg;
}


void ServiceDiscovery::NotifyServiceCreated(const ServiceDescription& serviceId)
{
    auto& announcement = _announcementMap[_participantName]; //update our existing entry
    announcement.participantName = _participantName;
    announcement.services.push_back(serviceId);
    _comAdapter->SendIbMessage(this, announcement);//ensure message history is updated
}

void ServiceDiscovery::RegisterServiceDiscoveryHandler(ServiceDiscoveryHandlerT handler)
{
    _handlers.emplace_back(std::move(handler));
}

} // namespace service
} // namespace mw
} // namespace ib
