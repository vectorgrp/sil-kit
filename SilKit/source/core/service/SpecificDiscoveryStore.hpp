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

    SpecificDiscoveryStore(const std::string& fromParticipant );

    //!< Service addition/removal
    void ServiceChange(ServiceDiscoveryEvent::Type changeType, const ServiceDescriptor& serviceDescriptor);

    //!< React on single service changes
    void RegisterSpecificServiceDiscoveryHandler(ServiceDiscoveryHandler handler,
                                                 const std::string& controllerTypeName,
                                                 const std::string& supplDataValue);

private:

    //!< Inform about service changes
    void CallHandlers(ServiceDiscoveryEvent::Type eventType, const std::string& controllerTypeName,
                      const std::string& associatedSupplDataKey, const std::string& supplDataValue,
                      const ServiceDescriptor& serviceDescriptor);

    using ServiceMap = std::unordered_map<std::string /*serviceDescriptor*/, ServiceDescriptor>;

    // Storage of specific handlers on this participant by [controllerTypeName/supplDataKey/supplDataValue]
    std::unordered_map<std::string /* unique_key */, std::vector<ServiceDiscoveryHandler>> _specificHandlers;

    // Storage of incoming serviceDescriptors by [participantName][controllerTypeName/supplDataKey/supplDataValue]
    using ServiceMapByUniqueKey = std::unordered_map<std::string /* unique_key */, ServiceMap>;
    std::unordered_map<std::string /* participant name */, ServiceMapByUniqueKey> _specificAnnouncementsByParticipant;

    // We only allow specific handlers for fixed controllerTypeName, supplDataKey are associated
    const std::unordered_map<std::string, std::string> _allowedSpecificDiscovery = {
        {controllerTypeDataPublisher, supplKeyDataPublisherTopic},
        {controllerTypeRpcServerInternal, supplKeyRpcServerInternalClientUUID},
        {controllerTypeRpcClient, supplKeyRpcClientFunctionName}};

    std::string _participantName;

};

} // namespace Discovery
} // namespace Core
} // namespace SilKit

