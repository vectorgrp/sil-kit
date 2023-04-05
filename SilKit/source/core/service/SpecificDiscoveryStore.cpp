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
namespace {
inline auto MakeFilter(const std::string& type, const std::string& topicOrFunction) 
  -> SilKit::Core::Discovery::FilterType
{
     return std::make_tuple(type, topicOrFunction);
}
} // end namespace
namespace SilKit {
namespace Core {
namespace Discovery {

// Service changes get announced here. 
void SpecificDiscoveryStore::ServiceChange(ServiceDiscoveryEvent::Type changeType,
                                           const ServiceDescriptor& serviceDescriptor) 
{
    std::string supplControllerTypeName;
    if (serviceDescriptor.GetSupplementalDataItem(Core::Discovery::controllerType, supplControllerTypeName))
    {
        if (_allowedControllers.count(supplControllerTypeName))
        {
            std::string key;
            std::string mediaType;
            std::vector<SilKit::Services::MatchingLabel> labels;

            // extract relevant information depending on controllerType
            if (supplControllerTypeName == controllerTypeRpcServerInternal)
            {
                serviceDescriptor.GetSupplementalDataItem(supplKeyRpcServerInternalClientUUID, key);
                serviceDescriptor.GetSupplementalDataItem(supplKeyRpcServerMediaType, mediaType);
            }
            else if (supplControllerTypeName == controllerTypeRpcClient)
            {
                serviceDescriptor.GetSupplementalDataItem(supplKeyRpcClientFunctionName, key);
                serviceDescriptor.GetSupplementalDataItem(supplKeyRpcClientMediaType, mediaType);

                // Add labels
                std::string labelsStr;
                if(serviceDescriptor.GetSupplementalDataItem(supplKeyRpcClientLabels, labelsStr))
                {
                    labels = SilKit::Config::Deserialize<decltype(labels)>(labelsStr);
                }
            }
            else if (supplControllerTypeName == controllerTypeDataPublisher)
            {
                serviceDescriptor.GetSupplementalDataItem(supplKeyDataPublisherTopic, key);
                serviceDescriptor.GetSupplementalDataItem(supplKeyDataPublisherMediaType, mediaType);

                // Add labels
                std::string labelsStr;
                if(serviceDescriptor.GetSupplementalDataItem(supplKeyDataPublisherPubLabels, labelsStr))
                {
                    labels = SilKit::Config::Deserialize<decltype(labels)>(labelsStr);
                }
            }

            CallHandlersOnServiceChange(changeType, supplControllerTypeName, key, labels, serviceDescriptor);
            if (changeType == ServiceDiscoveryEvent::Type::ServiceCreated)
            {
                InsertLookupNode(supplControllerTypeName, key, labels, serviceDescriptor);
            }
            else if (changeType == ServiceDiscoveryEvent::Type::ServiceRemoved)
            {
                RemoveLookupNode(supplControllerTypeName, key, serviceDescriptor);
            }
        }
    }
}

// A new subscriber shows up -> notify of all earlier services
void SpecificDiscoveryStore::CallHandlerOnHandlerRegistration(const ServiceDiscoveryHandler& handler,
                                                              const std::string& controllerType_, const std::string& key,
                                                              const std::vector<SilKit::Services::MatchingLabel>& labels)
{
    // pre filter key and mediaType
    auto& entry = _lookup[MakeFilter(controllerType_, key)];

    auto* greedyLabel = GetLabelWithMinimalNodeSet(entry, labels);

    if (greedyLabel == nullptr)
    {
        // no labels present trigger all
        for (auto&& serviceDescriptor : entry.allCluster.nodes)
        {
            handler(ServiceDiscoveryEvent::Type::ServiceCreated, serviceDescriptor);
        }
    }
    else
    {
        if (greedyLabel->kind == SilKit::Services::MatchingLabel::Kind::Optional)
        {
            // trigger notlabel handlers
            for (auto&& serviceDescriptor : entry.notLabelMap[greedyLabel->key].nodes)
            {
                handler(ServiceDiscoveryEvent::Type::ServiceCreated, serviceDescriptor);
            }
            for (auto&& serviceDescriptor : entry.noLabelCluster.nodes)
            {
                handler(ServiceDiscoveryEvent::Type::ServiceCreated, serviceDescriptor);
            }
        }
        // trigger label handlers
        for (auto&& serviceDescriptor : entry.labelMap[MakeFilter(greedyLabel->key, greedyLabel->value)].nodes)
        {
            handler(ServiceDiscoveryEvent::Type::ServiceCreated, serviceDescriptor);
        }
    }
}

// A new publisher shows up -> notify all subscriber handlers
void SpecificDiscoveryStore::CallHandlersOnServiceChange(ServiceDiscoveryEvent::Type eventType,
                                                         const std::string& supplControllerTypeName, const std::string& key,
                                                         const std::vector<SilKit::Services::MatchingLabel>& labels,
                                                         const ServiceDescriptor& serviceDescriptor)
{
    // pre filter key and mediaType
    auto& entry = _lookup[MakeFilter(supplControllerTypeName, key)];
    
    const SilKit::Services::MatchingLabel* greedyLabel = GetLabelWithMinimalHandlerSet(entry, labels);

    if (greedyLabel == nullptr)
    {
        // no labels present trigger all
        for (auto&& handler : entry.allCluster.handlers)
        {
            if (handler)
            {
                (*handler)(eventType, serviceDescriptor);
            }
        }
    }
    else
    {
        if (greedyLabel->kind == SilKit::Services::MatchingLabel::Kind::Optional)
        {
            // trigger notlabel handlers
            for (auto&& handler : entry.notLabelMap[greedyLabel->key].handlers)
            {
                if (handler)
                {
                    (*handler)(eventType, serviceDescriptor);
                }
            }
            for (auto&& handler : entry.noLabelCluster.handlers)
            {
                if (handler)
                {
                    (*handler)(eventType, serviceDescriptor);
                }
            }
        }
        // trigger label handlers
        for (auto&& handler : entry.labelMap[MakeFilter(greedyLabel->key, greedyLabel->value)].handlers)
        {
            if (handler)
            {
                (*handler)(eventType, serviceDescriptor);
            }
        }
    }
}

void SpecificDiscoveryStore::UpdateLookupOnServiceChange(ServiceDiscoveryEvent::Type eventType,
                                                         const std::string& supplControllerTypeName, const std::string& key,
                                                         const std::vector<SilKit::Services::MatchingLabel>& labels,
                                                         const ServiceDescriptor& serviceDescriptor)
{    
    if (eventType == ServiceDiscoveryEvent::Type::ServiceCreated)
    {
        InsertLookupNode(supplControllerTypeName, key, labels, serviceDescriptor);
    }
    else if (eventType == ServiceDiscoveryEvent::Type::ServiceRemoved)
    {
        RemoveLookupNode(supplControllerTypeName, key, serviceDescriptor);
    }
}

auto SpecificDiscoveryStore::GetLabelWithMinimalHandlerSet(
    DiscoveryKeyNode& keyNode, const std::vector<SilKit::Services::MatchingLabel>& labels)
    -> const SilKit::Services::MatchingLabel*
{
    const SilKit::Services::MatchingLabel* outGreedyLabel = nullptr;

    size_t matchCount = keyNode.allCluster.handlers.size();
    // search greedy Cluster guess
    for (auto&& l : labels)
    {
        if (matchCount <= 1)
            break;
        const auto keyTuple = std::make_tuple(l.key, l.value);
        if (l.kind == SilKit::Services::MatchingLabel::Kind::Mandatory)
        {
            auto& handlers = keyNode.labelMap[keyTuple].handlers;
            const auto relevantNodeCount = handlers.size();
            if (relevantNodeCount < matchCount)
            {
                matchCount = relevantNodeCount;
                outGreedyLabel = &l;
            }
        }
        else if (l.kind == SilKit::Services::MatchingLabel::Kind::Optional)
        {
            auto& fit_handlers = keyNode.labelMap[keyTuple].handlers;
            auto& not_label_handlers = keyNode.notLabelMap[l.key].handlers;

            size_t relevantNodeCount = fit_handlers.size() + not_label_handlers.size();
            if (relevantNodeCount < matchCount)
            {
                matchCount = relevantNodeCount;
                outGreedyLabel = &l;
            }
        }
    }

    return outGreedyLabel;
}

auto SpecificDiscoveryStore::GetLabelWithMinimalNodeSet(DiscoveryKeyNode& keyNode, 
                                                           const std::vector<SilKit::Services::MatchingLabel>& labels)
    -> const SilKit::Services::MatchingLabel*
{
    const SilKit::Services::MatchingLabel* outGreedyLabel = nullptr;

    size_t matchCount = keyNode.allCluster.nodes.size();
    // search greedy Cluster guess
    for (auto&& l : labels)
    {
        const auto keyTuple = std::make_tuple(l.key, l.value);
        if (matchCount <= 1)
            break;
        if (l.kind == SilKit::Services::MatchingLabel::Kind::Mandatory)
        {
            auto& services = keyNode.labelMap[keyTuple].nodes;
            size_t relevantNodeCount = services.size();
            if (relevantNodeCount < matchCount)
            {
                matchCount = relevantNodeCount;
                outGreedyLabel = &l;
            }
        }
        else if (l.kind == SilKit::Services::MatchingLabel::Kind::Optional)
        {
            auto& fit_nodes = keyNode.labelMap[keyTuple].nodes;
            auto& not_label_nodes = keyNode.notLabelMap[l.key].nodes;

            size_t relevantNodeCount = fit_nodes.size() + not_label_nodes.size();
            if (relevantNodeCount < matchCount)
            {
                matchCount = relevantNodeCount;
                outGreedyLabel = &l;
            }
        }
    }

    return outGreedyLabel;
}

void SpecificDiscoveryStore::UpdateDiscoveryClusters(const std::string& controllerType_, const std::string& key,
                                                     const std::vector<SilKit::Services::MatchingLabel>& labels,
    std::function<void(DiscoveryCluster&)> updater)
{
    auto& entry = _lookup[MakeFilter(controllerType_, key)];
    updater(entry.allCluster);

    if (labels.empty())
    {
        updater(entry.noLabelCluster);
    }
    else
    {
        // make sure all labels are represented
        for (auto&& l : labels)
        {
            // create entry if it does not exist
            entry.labelMap[MakeFilter(l.key, l.value)];


            bool firstTimeLabel = entry.notLabelMap.find(l.key) == entry.notLabelMap.end();
            entry.notLabelMap[l.key];
            if (firstTimeLabel)
            {
                // label is seen for the first time (add all earlier serviceDescriptors to notLabelEntry
                for (auto& serviceDescriptor : entry.allCluster.nodes)
                {
                    entry.notLabelMap[l.key].nodes.emplace_back(serviceDescriptor);
                }
                // label is seen for the first time (add all earlier handlers to notLabelEntry
                for (auto& handler : entry.allCluster.handlers)
                {
                    entry.notLabelMap[l.key].handlers.emplace_back(handler);
                }
            }
        }

        // insert for every label
        for (auto&& keyval: entry.notLabelMap)
        {
            const auto& labelKey = keyval.first;
            auto foundLabel =
                std::find_if(labels.begin(), labels.end(), [&labelKey](const auto& ml) {
                    return ml.key == labelKey;
                });
            if (foundLabel != labels.end())
            {
                auto& labelEntry = entry.labelMap[MakeFilter(labelKey, foundLabel->value)];
                updater(labelEntry);
            }
            else
            {
                auto& labelEntry = entry.notLabelMap[labelKey];
                updater(labelEntry);
            }
        }
    }
}

void SpecificDiscoveryStore::InsertLookupNode(const std::string& controllerType_, const std::string& key, 
                                            const std::vector<SilKit::Services::MatchingLabel>& labels,
                                            const ServiceDescriptor& serviceDescriptor)
{
    UpdateDiscoveryClusters(controllerType_, key, labels, [&serviceDescriptor](auto& cluster) {
        cluster.nodes.push_back(serviceDescriptor);
    });
}

void SpecificDiscoveryStore::RemoveLookupNode(const std::string& controllerType_, const std::string& key,
                                              const ServiceDescriptor& serviceDescriptor)
{

    auto& entry = _lookup[MakeFilter(controllerType_, key)];
    entry.allCluster.nodes.erase(
        std::remove(entry.allCluster.nodes.begin(), entry.allCluster.nodes.end(), serviceDescriptor),
                                 entry.allCluster.nodes.end());
    entry.noLabelCluster.nodes.erase(
        std::remove(entry.noLabelCluster.nodes.begin(), entry.noLabelCluster.nodes.end(), serviceDescriptor),
        entry.noLabelCluster.nodes.end());

    for (auto&& keyval: entry.notLabelMap) 
    {
        auto& cluster = keyval.second;
        cluster.nodes.erase(std::remove(cluster.nodes.begin(), cluster.nodes.end(), serviceDescriptor),
                            cluster.nodes.end());
    }
    for (auto&& keyval: entry.labelMap)
    {
        auto& cluster = keyval.second;
        cluster.nodes.erase(std::remove(cluster.nodes.begin(), cluster.nodes.end(), serviceDescriptor),
                            cluster.nodes.end());
    }
}

void SpecificDiscoveryStore::InsertLookupHandler(const std::string& controllerType_, const std::string& key,
                                                 const std::vector<SilKit::Services::MatchingLabel>& labels,
                                                ServiceDiscoveryHandler handler)
{
    auto handlerPtr = std::make_shared<decltype(handler)>(std::move(handler));
    UpdateDiscoveryClusters(controllerType_, key, labels, [handlerPtr](auto& cluster) {
        cluster.handlers.push_back(handlerPtr);
    });
}

void SpecificDiscoveryStore::RegisterSpecificServiceDiscoveryHandler(
    ServiceDiscoveryHandler handler, const std::string& controllerType_, const std::string& key, 
    const std::vector<SilKit::Services::MatchingLabel>& labels)
{
    CallHandlerOnHandlerRegistration(handler, controllerType_, key, labels);
    InsertLookupHandler(controllerType_, key, labels, handler);
}

} // namespace Discovery
} // namespace Core
} // namespace SilKit

