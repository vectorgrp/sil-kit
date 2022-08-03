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

#include "SpecificDiscoveryStore.hpp"

namespace SilKit {
namespace Core {
namespace Discovery {

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
    if (serviceDescriptor.GetSupplementalDataItem(Core::Discovery::controllerType, supplControllerTypeName))
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


void SpecificDiscoveryStore::RegisterSpecificServiceDiscoveryHandler(ServiceDiscoveryHandler handler,
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

} // namespace Discovery
} // namespace Core
} // namespace SilKit

