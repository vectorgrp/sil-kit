// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <functional>
#include <unordered_map>
#include <mutex>
#include <vector>
#include <atomic>
#include <unordered_set>

#include "SpecificDiscoveryStore.hpp"

#include "IParticipantInternal.hpp"
#include "IServiceEndpoint.hpp"
#include "IReceiver.hpp"
#include "ISender.hpp"
#include "IServiceDiscovery.hpp"

namespace SilKit {
namespace Core {
namespace Discovery {

class ServiceDiscovery
    : public Core::IReceiver<ParticipantDiscoveryEvent, ServiceDiscoveryEvent>
    , public Core::ISender<ParticipantDiscoveryEvent, ServiceDiscoveryEvent>
    , public IServiceEndpoint
    , public Discovery::IServiceDiscovery
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
    void RegisterServiceDiscoveryHandler(ServiceDiscoveryHandler handler) override;
    //!< Register a specific handler for asynchronous service creation notifications
    void RegisterSpecificServiceDiscoveryHandler(ServiceDiscoveryHandler handler, const std::string& controllerType,
                                                 const std::string& topic,
                                                 const std::vector<SilKit::Services::MatchingLabel>& labels) override;

    //!< Get all currently known services, including from ourselves
    std::vector<ServiceDescriptor> GetServices() const override;

    //!< React on a leaving participant, called via RegisterPeerShutdownCallback
    void OnParticpantRemoval(const std::string& participantName) override;

public: // Interfaces
    // IServiceEndpoint
    void SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor) override;
    auto GetServiceDescriptor() const -> const Core::ServiceDescriptor& override;

    // IReceiver
    void ReceiveMsg(const IServiceEndpoint* from, const ParticipantDiscoveryEvent& msg) override;
    void ReceiveMsg(const IServiceEndpoint* from, const ServiceDiscoveryEvent& msg) override;

private: // Methods
    //!< React on a new participant
    void OnParticpantAddition(const ParticipantDiscoveryEvent& msg);

    //!< React on single service changes
    void OnServiceRemoval(const ServiceDescriptor&);
    void OnServiceAddition(const ServiceDescriptor&);

    //!< When a serciveDiscovery of another participant is discovered, we announce all services from ourselves
    void AnnounceLocalParticipantTo(const std::string& otherParticipant);

    //!< Inform about service changes
    void CallHandlers(ServiceDiscoveryEvent::Type eventType, const ServiceDescriptor& serviceDescriptor) const;

private:
    IParticipantInternal* _participant{nullptr};
    std::string _participantName;
    ServiceDescriptor _serviceDescriptor; //!< for the ServiceDiscovery controller itself
    std::vector<ServiceDiscoveryHandler> _handlers;
    //!< a cache for computing additions/removals per participant
    using ServiceMap = std::unordered_map<std::string /*serviceDescriptor*/, ServiceDescriptor>;
    std::unordered_map<std::string /* participant name */, ServiceMap> _servicesByParticipant;
    SpecificDiscoveryStore _specificDiscoveryStore;
    mutable std::recursive_mutex _discoveryMx;
    std::atomic<bool> _shuttingDown{false};
};

} // namespace Discovery
} // namespace Core
} // namespace SilKit
