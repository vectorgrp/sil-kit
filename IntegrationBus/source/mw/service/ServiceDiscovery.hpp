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
    enum class Type {
        ServiceCreated,
        ServiceRemoved,
    };
    using ServiceDiscoveryHandlerT = std::function<void(Type discoveryType, const ServiceDescription&)>;
public: 
    ServiceDiscovery(IComAdapterInternal* comadapter, const std::string& participantName);
    virtual ~ServiceDiscovery() = default;
   
    //!< Publish a locally created new ServiceId to all participants
    void NotifyServiceCreated(const ServiceDescription& serviceId);//TODO change this to const IIbServiceEndpoint& after moving supplementalData from ServiceDescription to ServiceId

    //!< Register a handler for asynchronous service creation notifications
    void RegisterServiceDiscoveryHandler(ServiceDiscoveryHandlerT handler);
public: // Interfaces

    //IIbEndpoint
    void SetEndpointAddress(const ib::mw::EndpointAddress& endpointAddress)  override;
    auto EndpointAddress() const -> const ib::mw::EndpointAddress& override;
    // IIbServiceEndpoint
    void SetServiceId(const mw::ServiceId& serviceId) override;
    auto GetServiceId() const -> const mw::ServiceId & override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const ServiceAnnouncement& msg) override;

private:
    IComAdapterInternal* _comAdapter{nullptr};
    std::string _participantName;
    ServiceId _serviceId; //!< for the ServiceDiscovery controller itself
    std::vector<ServiceDiscoveryHandlerT> _handlers;
    std::map<std::string /*participant name*/, ServiceAnnouncement> _announcementMap;
};

// ================================================================================
//  Inline Implementations
// ================================================================================

} // namespace service
} // namespace mw
} // namespace ib

