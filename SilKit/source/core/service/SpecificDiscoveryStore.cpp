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
#include "YamlParser.hpp"

namespace SilKit {
namespace Core {
namespace Discovery {

// Controllers register their discovery handler here. 
void SpecificDiscoveryStore::RegisterSpecificServiceDiscoveryHandler(ServiceDiscoveryHandler handler,
                                                                     const std::vector<std::string>& lookupKeys)
{
    // If related services already got announced, call the new handler 
    CallHandlerOnHandlerRegistration(handler, lookupKeys);
    // Add to lookup for later added handlers.
    UpdateLookupOnHandlerRegistration(std::move(handler), lookupKeys);
}

// Service changes get announced here. 
void SpecificDiscoveryStore::ServiceChange(ServiceDiscoveryEvent::Type changeType,
                                           const ServiceDescriptor& serviceDescriptor) 
{
    std::string supplControllerTypeName;
    if (serviceDescriptor.GetSupplementalDataItem(Core::Discovery::controllerType, supplControllerTypeName))
    {
        if (_allowedControllers.count(supplControllerTypeName))
        {
            const std::vector<std::string>& lookupKeys = ConstructLookupKeys(supplControllerTypeName, serviceDescriptor);
            for (const auto& lookupKey : lookupKeys)
            {
                // Call handlers interested in the new service.
                CallHandlersOnServiceChange(changeType, lookupKey, serviceDescriptor);
                // Add to lookup for later added handlers.
                UpdateLookupOnServiceChange(changeType, lookupKey, serviceDescriptor);
            }
        }
    }
}

std::vector<std::string> SpecificDiscoveryStore::ConstructLookupKeys(
    const std::string& supplControllerTypeName, const ServiceDescriptor& serviceDescriptor) const
{
    if (supplControllerTypeName == controllerTypeDataPublisher)
    {
        return ConstructLookupKeysDataPublisher(serviceDescriptor);
    }
    else if (supplControllerTypeName == controllerTypeRpcServerInternal)
    {
        return ConstructLookupKeysRpcServerInternal(serviceDescriptor);
    }
    else if (supplControllerTypeName == controllerTypeRpcClient)
    {
        return ConstructLookupKeysRpcClient(serviceDescriptor);
    }

    return {};
}

std::vector<std::string> SpecificDiscoveryStore::ConstructLookupKeysDataPublisher(
    const ServiceDescriptor& serviceDescriptor) const
{
    std::vector<std::string> lookupKeys{};

    std::string lookupKeyBase = controllerTypeDataPublisher + "/" + supplKeyDataPublisherTopic + "/";
    // Add topic
    std::string topic;
    serviceDescriptor.GetSupplementalDataItem(supplKeyDataPublisherTopic, topic);
    lookupKeyBase += topic + "/" + supplKeyDataPublisherPubLabels + "/";

    // Add labels
    std::string labelsStr;
    serviceDescriptor.GetSupplementalDataItem(supplKeyDataPublisherPubLabels, labelsStr);
    std::vector<SilKit::Services::MatchingLabel> labels =
        SilKit::Config::Deserialize<std::vector<SilKit::Services::MatchingLabel>>(labelsStr);
    std::string allLabelsStr = lookupKeyBase;
    std::string mandatoryLabelsStr = lookupKeyBase;
    bool hasOptional = false;
    
    if (labels.empty())
    {
        lookupKeys.push_back(lookupKeyBase);
    }
    else
    {
        for (auto l : labels)
        {
            allLabelsStr += l.key + "/" + l.value + "/";
            if (l.kind == Services::MatchingLabel::Kind::Mandatory)
            {
                mandatoryLabelsStr += l.key + "/" + l.value + "/";
            }
            else
            {
                hasOptional = true;
            }
        }
        if (hasOptional)
        {
            lookupKeys.push_back(allLabelsStr);
        }
        // Add entry only with mandatory labels. Might boil down to no labels and just topic to find optional <-> empty
        lookupKeys.push_back(mandatoryLabelsStr);

    }

    return lookupKeys;
}

std::vector<std::string> SpecificDiscoveryStore::ConstructLookupKeysRpcServerInternal(
    const ServiceDescriptor& serviceDescriptor) const
{
    std::string lookupKey = controllerTypeRpcServerInternal + "/" + supplKeyRpcServerInternalClientUUID + "/";
    std::string clientUUID;
    serviceDescriptor.GetSupplementalDataItem(supplKeyRpcServerInternalClientUUID, clientUUID);
    lookupKey += clientUUID;
    return {std::move(lookupKey)};
}

std::vector<std::string> SpecificDiscoveryStore::ConstructLookupKeysRpcClient(
    const ServiceDescriptor& serviceDescriptor) const
{
    std::string lookupKey = controllerTypeRpcClient + "/" + supplKeyRpcClientFunctionName + "/";
    std::string functionName;
    serviceDescriptor.GetSupplementalDataItem(supplKeyRpcClientFunctionName, functionName);
    lookupKey += functionName;
    return {std::move(lookupKey)};
}

void SpecificDiscoveryStore::CallHandlersOnServiceChange(ServiceDiscoveryEvent::Type eventType,
                                                         const std::string& lookupKey,
                                          const ServiceDescriptor& serviceDescriptor) const
{
    auto&& handlersForUniqueKey = _specificDiscoveryHandlers.find(lookupKey);
    if (handlersForUniqueKey != _specificDiscoveryHandlers.end())
    {
        auto handlers = handlersForUniqueKey->second;
        for (auto&& handler : handlers)
        {
            handler(eventType, serviceDescriptor);
        }
    }
}

void SpecificDiscoveryStore::UpdateLookupOnServiceChange(ServiceDiscoveryEvent::Type eventType,
                                                      const std::string& lookupKey,
                                                      const ServiceDescriptor& serviceDescriptor)
{
    const auto& participantName = serviceDescriptor.GetParticipantName();
    if (eventType == ServiceDiscoveryEvent::Type::ServiceCreated)
    {
        _serviceDescriptorsByParticipant[participantName][lookupKey].emplace(serviceDescriptor.to_string(),
                                                                             serviceDescriptor);
    }
    else if (eventType == ServiceDiscoveryEvent::Type::ServiceRemoved)
    {
        _serviceDescriptorsByParticipant[participantName][lookupKey].erase(serviceDescriptor.to_string());
    }
}

void SpecificDiscoveryStore::CallHandlerOnHandlerRegistration(const ServiceDiscoveryHandler& handler,
                                                        const std::vector<std::string>& lookupKeys) const
{
    // Call the handler with the serviceDescriptors we are interested in
    // This might create (internal) controllers and alter _serviceDescriptorsByParticipant while iterating. 
    // Iterators for std::map are valid though.
    for (auto&& participantServices : _serviceDescriptorsByParticipant)
    {
        auto&& serviceMapByUniqueKey = participantServices.second;
        for (const auto& key : lookupKeys)
        {
            auto serviceForUniqueKey = serviceMapByUniqueKey.find(key);
            if (serviceForUniqueKey != serviceMapByUniqueKey.end())
            {
                for (auto&& serviceDescriptorMapEntry : serviceForUniqueKey->second)
                {
                    handler(ServiceDiscoveryEvent::Type::ServiceCreated, serviceDescriptorMapEntry.second);
                }
            }
        }
    }
}

// Store the handler as we might not have seen relevant participants yet.
void SpecificDiscoveryStore::UpdateLookupOnHandlerRegistration(ServiceDiscoveryHandler handler,
                                                      const std::vector<std::string>& lookupKeys)
{
    for (const auto& key : lookupKeys)
    {
        _specificDiscoveryHandlers[key].emplace_back(handler);
    }
}

} // namespace Discovery
} // namespace Core
} // namespace SilKit

