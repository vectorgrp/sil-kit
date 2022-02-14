// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <functional>
#include <unordered_map>
#include <mutex>
#include <vector>
#include <atomic>

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
    virtual ~ServiceDiscovery() noexcept;
  
public: //IServiceDiscovery
    //!< Publish a locally created new ServiceId to all other participants
    void NotifyServiceCreated(const ServiceDescriptor& serviceDescriptor) override;
    //!< Publish a participant-local service removal to all other participants
    void NotifyServiceRemoved(const ServiceDescriptor& serviceDescriptor) override;
    //!< Register a handler for asynchronous service creation notifications
    void RegisterServiceDiscoveryHandler(ServiceDiscoveryHandlerT handler) override;
    std::vector<ServiceDescriptor> GetRemoteServices() const override;

public: // Interfaces

    // IIbServiceEndpoint
    void SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor) override;
    auto GetServiceDescriptor() const -> const mw::ServiceDescriptor & override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const ServiceAnnouncement& msg) override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const ServiceDiscoveryEvent& msg) override;
private: //methods
    void ReceivedServiceRemoval(const ServiceDescriptor&);
    void ReceivedServiceAddition(const ServiceDescriptor&);
    void CallHandlers(ServiceDiscoveryEvent::Type eventType, const ServiceDescriptor& serviceDescriptor);

private:
    IComAdapterInternal* _comAdapter{nullptr};
    std::string _participantName;
    ServiceDescriptor _serviceDescriptor; //!< for the ServiceDiscovery controller itself
    std::vector<ServiceDiscoveryHandlerT> _handlers;
    ServiceAnnouncement _announcement;
    //!< a cache for computing additions/removals per participant
    using ServiceMap = std::unordered_map<std::string /*serviceDescriptor*/, ServiceDescriptor>;
    std::unordered_map<std::string /* participant name */, ServiceMap> _announcedServices; 
    mutable std::recursive_mutex _discoveryMx;
    std::atomic<bool> _shuttingDown{false};
};

// ================================================================================
//  Inline Implementations
// ================================================================================

} // namespace service
} // namespace mw
} // namespace ib

