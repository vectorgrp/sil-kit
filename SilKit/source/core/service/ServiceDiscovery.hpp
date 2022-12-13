/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

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
    auto GetServiceDescriptor() const -> const Core::ServiceDescriptor & override;

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

