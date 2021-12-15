// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <functional>
#include "ServiceDatatypes.hpp"

#include "IComAdapterInternal.hpp"
#include "IIbServiceEndpoint.hpp"
#include "IIbEndpoint.hpp"
#include "IIbSender.hpp"

namespace ib {
namespace mw {
namespace service {

class ServiceDiscovery
    : public mw::IIbEndpoint<ServiceAnnouncement>
    , public mw::IIbSender<ServiceAnnouncement>
    , public IIbServiceEndpoint
{
public: //types
    using ServiceCreationHandlerT = std::function<void(const ServiceDescription&)>;
public: 
    ServiceDiscovery(IComAdapterInternal* comadapter, const std::string& participantName)
        : _comAdapter{comadapter}
        , _participantName{participantName}
    {
    }
    virtual ~ServiceDiscovery()
    {
    }
   
    //!< Publish a new ServiceId to all participants
    inline void NotifyServiceCreated(const ServiceDescription& serviceId);//TODO change this to const IIbServiceEndpoint& after moving supplementalData from ServiceDescription to ServiceId

    //!< Register a handler for asynchronous service creation notifications
    inline void RegisterServiceCreationHandler(ServiceCreationHandlerT handler);
public: // Interfaces

    //IIbEndpoint
    inline void SetEndpointAddress(const ib::mw::EndpointAddress& endpointAddress)  override;
    inline auto EndpointAddress() const -> const ib::mw::EndpointAddress& override;
    // IIbServiceEndpoint
    inline void SetServiceId(const mw::ServiceId& serviceId) override;
    inline auto GetServiceId() const -> const mw::ServiceId & override;
    inline void ReceiveIbMessage(const IIbServiceEndpoint* from, const ServiceAnnouncement& msg) override;

private:
    IComAdapterInternal* _comAdapter{nullptr};
    std::string _participantName;
    ServiceId _serviceId; //!< for the ServiceDiscovery controller itself
    std::vector<ServiceCreationHandlerT> _handlers;
    std::map<std::string /*participant name*/, ServiceAnnouncement> _announcementMap;
};

// ================================================================================
//  Inline Implementations
// ================================================================================


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
    if (from == this)
    {
        return; //no self deliveries
    }
    if (_announcementMap.count(msg.participantName) > 0)
    {
        // compute which services were added
        auto&& oldAnnouncement = _announcementMap[msg.participantName];
        if (!(oldAnnouncement.services.size() < msg.services.size()))
        {
            throw std::runtime_error("ServiceDiscovery: ReceiveIbMessage: Assertion failed");
        }
        for (auto i = oldAnnouncement.services.size(); i < msg.services.size(); i++)
        {
            const auto& newService = msg.services[i];
            for (auto&& handler : _handlers)
            {
                handler(newService);
            }
        }
    }
    else 
    {
        for (auto&& service : msg.services)
        {
            for (auto&& handler : _handlers)
            {
                handler(service);
            }
        }
    }
    _announcementMap[msg.participantName] = msg;
}


void ServiceDiscovery::NotifyServiceCreated(const ServiceDescription& serviceId)
{
    auto& announcement = _announcementMap[_participantName];
    announcement.participantName = _participantName;
    announcement.services.push_back(serviceId);
    _comAdapter->SendIbMessage(this, announcement);//ensure message history is updated
}

void ServiceDiscovery::RegisterServiceCreationHandler(ServiceCreationHandlerT handler)
{
    _handlers.emplace_back(std::move(handler));
}
} // namespace service
} // namespace mw
} // namespace ib

