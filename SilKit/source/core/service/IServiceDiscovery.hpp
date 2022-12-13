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

} // namespace Orchestration
} // namespace Core
} // namespace SilKit
