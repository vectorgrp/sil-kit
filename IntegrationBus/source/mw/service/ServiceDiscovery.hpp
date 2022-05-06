// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <functional>
#include <unordered_map>
#include <mutex>
#include <vector>
#include <atomic>
#include <unordered_set>

#include "SpecificDiscoveryStore.hpp"

#include "IParticipantInternal.hpp"
#include "IIbServiceEndpoint.hpp"
#include "IIbReceiver.hpp"
#include "IIbSender.hpp"
#include "IServiceDiscovery.hpp"

namespace ib {
namespace mw {
namespace service {

class ServiceDiscovery
    : public mw::IIbReceiver<ParticipantDiscoveryEvent, ServiceDiscoveryEvent>
    , public mw::IIbSender<ParticipantDiscoveryEvent, ServiceDiscoveryEvent>
    , public IIbServiceEndpoint
    , public service::IServiceDiscovery
{
public: 

    ServiceDiscovery(IParticipantInternal* participant, const std::string& participantName);
    virtual ~ServiceDiscovery() noexcept;
  
public: //IServiceDiscovery
    //!< Called on creating a service locally; Publish new service to ourselves and all other participants
    void NotifyServiceCreated(const ServiceDescriptor& serviceDescriptor) override;
    //!< Called on removing a service locally; Publish service removal to ourselves to all other participants
    void NotifyServiceRemoved(const ServiceDescriptor& serviceDescriptor) override;
    //!< Register a handler for asynchronous service creation notifications
    void RegisterServiceDiscoveryHandler(ServiceDiscoveryHandlerT handler) override;
    //!< Register a specific handler for asynchronous service creation notifications
    void RegisterSpecificServiceDiscoveryHandler(ServiceDiscoveryHandlerT handler,
                                                 const std::string& controllerTypeName,
                                                 const std::string& supplDataValue) override;

    //!< Get all currently known services, including from ourselves
    std::vector<ServiceDescriptor> GetServices() const override;

    //!< React on a leaving participant, called via RegisterPeerShutdownCallback 
    void OnParticpantRemoval(const std::string& participantName) override;

public: // Interfaces

    // IIbServiceEndpoint
    void SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor) override;
    auto GetServiceDescriptor() const -> const mw::ServiceDescriptor & override;

    // IIbReceiver
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const ParticipantDiscoveryEvent& msg) override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const ServiceDiscoveryEvent& msg) override;

private: //methods

    //!< React on a new participant
    void OnParticpantAddition(const ParticipantDiscoveryEvent& msg);

    //!< React on single service changes
    void OnServiceRemoval(const ServiceDescriptor&);
    void OnServiceAddition(const ServiceDescriptor&);

    //!< When a serciveDiscovery of another participant is discovered, we announce all services from ourselves 
    void AnnounceLocalParticipantTo(const std::string& otherParticipant);

    //!< Inform about service changes
    void CallHandlers(ServiceDiscoveryEvent::Type eventType, const ServiceDescriptor& serviceDescriptor) const;

    void AddService(const ServiceDescriptor& serviceDescriptor);
    void RemoveService(const ServiceDescriptor& serviceDescriptor);

private:
    IParticipantInternal* _participant{nullptr};
    std::string _participantName;
    ServiceDescriptor _serviceDescriptor; //!< for the ServiceDiscovery controller itself
    std::vector<ServiceDiscoveryHandlerT> _handlers;
    //!< a cache for computing additions/removals per participant
    using ServiceMap = std::unordered_map<std::string /*serviceDescriptor*/, ServiceDescriptor>;
    std::unordered_map<std::string /* participant name */, ServiceMap> _servicesByParticipant; 
    SpecificDiscoveryStore _specificDiscoveryStore;
    mutable std::recursive_mutex _discoveryMx;
    std::atomic<bool> _shuttingDown{false};
};

} // namespace service
} // namespace mw
} // namespace ib

