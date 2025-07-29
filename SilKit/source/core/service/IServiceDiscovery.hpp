// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <functional>
#include <vector>

#include "ServiceDatatypes.hpp"

namespace SilKit {
namespace Core {
namespace Discovery {

using ServiceDiscoveryHandler =
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
    virtual void RegisterServiceDiscoveryHandler(ServiceDiscoveryHandler handler) = 0;
    //!< Register a handler for service creation notifications for a specific controllerTypeName,
    //!< associated supplDataKey and given supplDataValue
    virtual void RegisterSpecificServiceDiscoveryHandler(
        ServiceDiscoveryHandler handler, const std::string& controllerType, const std::string& topic,
        const std::vector<SilKit::Services::MatchingLabel>& labels) = 0;
    //!< Get the currently known created services on other participants
    virtual std::vector<ServiceDescriptor> GetServices() const = 0;
    //!< React on a participant shutdown
    virtual void OnParticpantRemoval(const std::string& participantName) = 0;
};

} // namespace Discovery
} // namespace Core
} // namespace SilKit
