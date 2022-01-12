// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <functional>
#include <unordered_map>
#include <mutex>

#include "ServiceDatatypes.hpp"

#include "IComAdapterInternal.hpp"
#include "IIbServiceEndpoint.hpp"
#include "IIbEndpoint.hpp"
#include "IIbSender.hpp"
#include "IServiceDiscovery.hpp"

namespace ib {
namespace mw {
namespace service {

class ServiceDiscovery
    : public mw::IIbEndpoint<ServiceAnnouncement, ServiceDiscoveryEvent>
    , public mw::IIbSender<ServiceAnnouncement, ServiceDiscoveryEvent>
    , public IIbServiceEndpoint
    , public service::IServiceDiscovery
{
public: 
    using IServiceDiscovery::ServiceDiscoveryHandlerT;

    ServiceDiscovery(IComAdapterInternal* comadapter, const std::string& participantName);
    virtual ~ServiceDiscovery() = default;
  
public: //IServiceDiscovery
    //!< Publish a locally created new ServiceId to all other participants
    void NotifyServiceCreated(const ServiceDescriptor& serviceDescriptor) override;
    //!< Publish a participant-local service removal to all other participants
    void NotifyServiceRemoved(const ServiceDescriptor& serviceDescriptor) override;
    //!< Register a handler for asynchronous service creation notifications
    void RegisterServiceDiscoveryHandler(ServiceDiscoveryHandlerT handler) override;
    void Initialize() override;
public: // Interfaces

    //IIbEndpoint
    void SetEndpointAddress(const ib::mw::EndpointAddress& endpointAddress)  override;
    auto EndpointAddress() const -> const ib::mw::EndpointAddress& override;
    // IIbServiceEndpoint
    void SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor) override;
    auto GetServiceDescriptor() const -> const mw::ServiceDescriptor & override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const ServiceAnnouncement& msg) override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const ServiceDiscoveryEvent& msg) override;
private: //methods
    void ReceivedServiceRemoval(const ServiceDescriptor&);
    void ReceivedServiceAddition(const ServiceDescriptor&);
private:
    IComAdapterInternal* _comAdapter{nullptr};
    std::string _participantName;
    ServiceDescriptor _serviceDescriptor; //!< for the ServiceDiscovery controller itself
    std::vector<ServiceDiscoveryHandlerT> _handlers;
    ServiceAnnouncement _announcement;
    //!< a cache for computing additions/removals per participant
    using ServiceMap = std::unordered_map<std::string /*serviceDescriptor*/, ServiceDescriptor>;
    std::unordered_map<std::string /* participant name */, ServiceMap> _announcedServices; 
};

// ================================================================================
//  Inline Implementations
// ================================================================================

} // namespace service
} // namespace mw
} // namespace ib

