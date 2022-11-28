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

#include <unordered_map>
#include <unordered_set>

#include "IServiceDiscovery.hpp"

namespace SilKit {
namespace Core {
namespace Discovery {

//!< Store to prevent quadratic lookup of services
class SpecificDiscoveryStore
{
public: 

    //!< Service addition/removal
    void ServiceChange(ServiceDiscoveryEvent::Type changeType, const ServiceDescriptor& serviceDescriptor);

    //!< React on single service changes
    void RegisterSpecificServiceDiscoveryHandler(ServiceDiscoveryHandler handler,
                                                 const std::vector<std::string>& lookupKeys);

private:

    //!< Inform about service changes
    void CallHandlersOnServiceChange(ServiceDiscoveryEvent::Type eventType, const std::string& lookupKey,
                                     const ServiceDescriptor& serviceDescriptor) const;
    void UpdateLookupOnServiceChange(ServiceDiscoveryEvent::Type eventType, const std::string& lookupKey,
                                   const ServiceDescriptor& serviceDescriptor);

    void CallHandlerOnHandlerRegistration(const ServiceDiscoveryHandler& handler,
                                          const std::vector<std::string>& lookupKeys) const;
    void UpdateLookupOnHandlerRegistration(ServiceDiscoveryHandler handler, const std::vector<std::string>& lookupKeys);

    std::vector<std::string> ConstructLookupKeys(const std::string& supplControllerTypeName,
                                                  const ServiceDescriptor& serviceDescriptor) const;
    std::vector<std::string> ConstructLookupKeysDataPublisher(const ServiceDescriptor& serviceDescriptor) const;
    std::vector<std::string> ConstructLookupKeysRpcServerInternal(const ServiceDescriptor& serviceDescriptor) const;
    std::vector<std::string> ConstructLookupKeysRpcClient(const ServiceDescriptor& serviceDescriptor) const;

    using ServiceMap = std::map<std::string /*serviceDescriptor*/, ServiceDescriptor>;

    // Storage of specific handlers on this participant by [controllerTypeName/supplDataKey/supplDataValue]
    std::map<std::string /* unique_key */, std::vector<ServiceDiscoveryHandler>> _specificDiscoveryHandlers;

    // Storage of incoming serviceDescriptors by [participantName][controllerTypeName/supplDataKey/supplDataValue]
    using ServiceMapByUniqueKey = std::map<std::string /* unique_key */, ServiceMap>;
    std::map<std::string /* participant name */, ServiceMapByUniqueKey> _serviceDescriptorsByParticipant;

    // We only allow specific handlers for fixed controllerTypeNames
    const std::unordered_set<std::string> _allowedControllers = {
        controllerTypeDataPublisher, controllerTypeRpcServerInternal, controllerTypeRpcClient};
};

} // namespace Discovery
} // namespace Core
} // namespace SilKit

