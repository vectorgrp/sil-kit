// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "SpecificDiscoveryStore.hpp"

namespace ib {
namespace mw {
namespace service {

SpecificDiscoveryStore::SpecificDiscoveryStore(const std::string& participantName) 
    : _participantName{ participantName }
{
}

void SpecificDiscoveryStore::ServiceChange(ServiceDiscoveryEvent::Type changeType,
                                           const ServiceDescriptor& serviceDescriptor)
{
    std::string participantName = serviceDescriptor.GetParticipantName();

    // Store serviceDescriptors (for specific handlers) by participantName | controllerTypeName | supplDataKey | supplDataValue
    std::string supplControllerTypeName;
    if (serviceDescriptor.GetSupplementalDataItem(mw::service::controllerType, supplControllerTypeName))
    {
        // Specific handlers are only allowed for fixed controller type names
        if (_allowedSpecificDiscovery.count(supplControllerTypeName))
        {
            // Suppl data keys are fixed for a given controller type name
            auto&& associatedSupplDataKey = _allowedSpecificDiscovery.at(supplControllerTypeName);
            std::string supplDataValue;
            if (serviceDescriptor.GetSupplementalDataItem(associatedSupplDataKey, supplDataValue))
            {
                const auto uniqueKey = supplControllerTypeName + "/" + associatedSupplDataKey + "/" + supplDataValue;

                CallHandlers(changeType, supplControllerTypeName, associatedSupplDataKey, supplDataValue,
                             serviceDescriptor);

                if (changeType == ServiceDiscoveryEvent::Type::ServiceCreated)
                {
                    _specificAnnouncementsByParticipant[participantName][uniqueKey].emplace(
                        serviceDescriptor.to_string(), serviceDescriptor);
                }
                else if (changeType == ServiceDiscoveryEvent::Type::ServiceRemoved)
                {
                    _specificAnnouncementsByParticipant[participantName][uniqueKey].erase(
                        serviceDescriptor.to_string());
                }
            }
        }
    }
}

void SpecificDiscoveryStore::CallHandlers(ServiceDiscoveryEvent::Type eventType, const std::string& controllerTypeName,
                                          const std::string& associatedSupplDataKey, const std::string& supplDataValue,
                                          const ServiceDescriptor& serviceDescriptor)
{
    const auto uniqueKey = controllerTypeName + "/" + associatedSupplDataKey + "/" + supplDataValue;
    auto&& handlersForUniqueKey = _specificHandlers.find(uniqueKey);
    if (handlersForUniqueKey != _specificHandlers.end())
    {
        // Call registered specific handlers
        auto handlers = handlersForUniqueKey->second;
        for (auto&& handler : handlers)
        {
            handler(eventType, serviceDescriptor);
        }
    }
}


void SpecificDiscoveryStore::RegisterSpecificServiceDiscoveryHandler(ServiceDiscoveryHandlerT handler,
                                                                     const std::string& controllerTypeName,
                                                                     const std::string& supplDataValue)
{
    auto&& supplDataKey = _allowedSpecificDiscovery.at(controllerTypeName);
    const auto uniqueKey = controllerTypeName + "/" + supplDataKey + "/" + supplDataValue;

    // The handler might create a new service and alters _specificAnnouncements while iterating here. Use a copy.
    auto specificAnnouncementsCopy = _specificAnnouncementsByParticipant;

    // We might have discovered participants (including ourselves) that announced services with suppl data we are interested in
    for (auto&& participantServices : specificAnnouncementsCopy)
    {
        // Check for the relevant entry
        auto&& serviceMapByUniqueKey = participantServices.second;
        auto serviceForUniqueKey = serviceMapByUniqueKey.find(uniqueKey);
        if (serviceForUniqueKey != serviceMapByUniqueKey.end())
        {
            // Call the handlers we are interested in
            for (auto&& serviceDescriptorMapEntry : serviceForUniqueKey->second)
            {
                handler(ServiceDiscoveryEvent::Type::ServiceCreated, serviceDescriptorMapEntry.second);
            }
        }
    }
    // Store the handler as we might not have seen relevant participants yet
    _specificHandlers[uniqueKey].emplace_back(std::move(handler));
}


} // namespace service
} // namespace mw
} // namespace ib

