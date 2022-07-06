// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <functional>
#include <vector>

#include "ServiceDatatypes.hpp"

namespace SilKit {
namespace Core {
namespace Discovery {

using ServiceDiscoveryHandlerT =
    std::function<void(ServiceDiscoveryEvent::Type discoveryType, const ServiceDescriptor&)>;

class IServiceDiscovery
{
public:

    virtual ~IServiceDiscovery() = default;
    //!< Publish a locally created new ServiceDescriptor to all other participants
    virtual void NotifyServiceCreated(const ServiceDescriptor& serviceDescriptor) = 0;
    //!< Publish a participant-local service removal to all other participants
    virtual void NotifyServiceRemoved(const ServiceDescriptor& serviceDescriptor) = 0;
    //!< Register a handler for asynchronous service creation notifications
    virtual void RegisterServiceDiscoveryHandler(ServiceDiscoveryHandlerT handler) = 0;
    //!< Register a handler for service creation notifications for a specific controllerTypeName, 
    //!< associated supplDataKey and given supplDataValue 
    virtual void RegisterSpecificServiceDiscoveryHandler(ServiceDiscoveryHandlerT handler,
                                                         const std::string& controllerTypeName,
                                                         const std::string& supplDataValue) = 0;
    //!< Get the currently known created services on other participants
    virtual std::vector<ServiceDescriptor> GetServices() const = 0;
    //!< React on a participant shutdown
    virtual void OnParticpantRemoval(const std::string& participantName) = 0;

};

} // namespace Orchestration
} // namespace Core
} // namespace SilKit
