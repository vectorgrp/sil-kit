// Copyright (c) Vector Informatik GmbH. All rights reserved.

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
    void RegisterSpecificServiceDiscoveryHandler(ServiceDiscoveryHandlerT handler,
                                                 const std::string& controllerTypeName,
                                                 const std::string& supplDataValue);

private:

    //!< Inform about service changes
    void CallHandlers(ServiceDiscoveryEvent::Type eventType, const std::string& controllerTypeName,
                      const std::string& associatedSupplDataKey, const std::string& supplDataValue,
                      const ServiceDescriptor& serviceDescriptor);

    using ServiceMap = std::unordered_map<std::string /*serviceDescriptor*/, ServiceDescriptor>;

    // Storage of specific handlers on this participant by [controllerTypeName/supplDataKey/supplDataValue]
    std::unordered_map<std::string /* unique_key */, std::vector<ServiceDiscoveryHandlerT>> _specificHandlers;

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

