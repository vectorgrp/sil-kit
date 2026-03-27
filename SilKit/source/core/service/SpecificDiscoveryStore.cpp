// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "SpecificDiscoveryStore.hpp"
#include "LabelMatching.hpp"
#include "YamlParser.hpp"

namespace {
inline auto MakeFilter(const std::string& type,
                       const std::string& topicOrFunction) -> SilKit::Core::Discovery::FilterType
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
                if (serviceDescriptor.GetSupplementalDataItem(supplKeyRpcClientLabels, labelsStr))
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
                if (serviceDescriptor.GetSupplementalDataItem(supplKeyDataPublisherPubLabels, labelsStr))
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
void SpecificDiscoveryStore::CallHandlerOnHandlerRegistration(
    const ServiceDiscoveryHandler& handler, const std::string& controllerType_, const std::string& key,
    const std::vector<SilKit::Services::MatchingLabel>& labels)
{
    // pre filter controllerType and Topic/Function
    auto& entry = _lookup[MakeFilter(controllerType_, key)];

    auto* greedyLabel = GetLabelWithMinimalNodeSet(entry, labels);

    if (greedyLabel == nullptr)
    {
        // no labels present trigger all
        for (auto&& serviceDescriptor : entry.allCluster.nodes)
        {
            const auto descriptorLabels = GetLabels(serviceDescriptor);
            if(Util::MatchLabels(labels, descriptorLabels))
            {
                handler(ServiceDiscoveryEvent::Type::ServiceCreated, serviceDescriptor);
            }
        }
    }
    else
    {

        if (greedyLabel->kind == SilKit::Services::MatchingLabel::Kind::Optional)
        {
            // Get all services that do not have the same optional label present
            for (auto&& serviceDescriptor : entry.notLabelMap[greedyLabel->key].nodes)
            {
                const auto descriptorLabels = GetLabels(serviceDescriptor);
                if(Util::MatchLabels(labels, descriptorLabels))
                {
                    handler(ServiceDiscoveryEvent::Type::ServiceCreated, serviceDescriptor);
                }
            }
            // Get all services that do not have any labels attached, thus matching our optional label
            for (auto&& serviceDescriptor : entry.noLabelCluster.nodes)
            {
                const auto descriptorLabels = GetLabels(serviceDescriptor);
                if(Util::MatchLabels(labels, descriptorLabels))
                {
                    handler(ServiceDiscoveryEvent::Type::ServiceCreated, serviceDescriptor);
                }
            }
        }
        // trigger label handlers for exact matches (optional and mandatory)
        for (auto&& serviceDescriptor : entry.labelMap[MakeFilter(greedyLabel->key, greedyLabel->value)].nodes)
        {
            const auto descriptorLabels = GetLabels(serviceDescriptor);
            if(Util::MatchLabels(labels, descriptorLabels))
            {
                handler(ServiceDiscoveryEvent::Type::ServiceCreated, serviceDescriptor);
            }
        }
    }
}

// A new publisher shows up -> notify all subscriber handlers
void SpecificDiscoveryStore::CallHandlersOnServiceChange(ServiceDiscoveryEvent::Type eventType,
                                                         const std::string& supplControllerTypeName,
                                                         const std::string& key,
                                                         const std::vector<SilKit::Services::MatchingLabel>& labels,
                                                         const ServiceDescriptor& serviceDescriptor)
{
    // pre filter key and mediaType
    auto& entry = _lookup[MakeFilter(supplControllerTypeName, key)];

    const SilKit::Services::MatchingLabel* greedyLabel = GetLabelWithMinimalHandlerSet(entry, labels);

    if (greedyLabel == nullptr)
    {

        bool skipLabelCheck = supplControllerTypeName == controllerTypeRpcServerInternal;
        // no labels present trigger all
        for (auto&& controllerInfo : entry.allCluster.controllerInfo)
        {
            bool run_handler = skipLabelCheck ? true : Util::MatchLabels(controllerInfo->labels, labels);
            if (controllerInfo->handler && run_handler)
            {
                controllerInfo->handler(eventType, serviceDescriptor);
            }
        }
    }
    else
    {
        if (greedyLabel->kind == SilKit::Services::MatchingLabel::Kind::Optional)
        {
            // trigger handlers that do not have the same optional label
            for (auto&& controllerInfo : entry.notLabelMap[greedyLabel->key].controllerInfo)
            {
                if (controllerInfo->handler && Util::MatchLabels(controllerInfo->labels, labels))
                {
                    controllerInfo->handler(eventType, serviceDescriptor);
                }
            }
            // trigger handlers with no labels attached, thus matching our optional label
            for (auto&& controllerInfo : entry.noLabelCluster.controllerInfo)
            {
                if (controllerInfo->handler && Util::MatchLabels(controllerInfo->labels, labels))
                {
                    controllerInfo->handler(eventType, serviceDescriptor);
                }
            }
        }
        // trigger label handlers with exact matches (optional and mandatory)
        for (auto&& controllerInfo : entry.labelMap[MakeFilter(greedyLabel->key, greedyLabel->value)].controllerInfo)
        {
            if (controllerInfo->handler && Util::MatchLabels(controllerInfo->labels, labels))
            {
                controllerInfo->handler(eventType, serviceDescriptor);
            }
        }
    }
}

void SpecificDiscoveryStore::UpdateLookupOnServiceChange(ServiceDiscoveryEvent::Type eventType,
                                                         const std::string& supplControllerTypeName,
                                                         const std::string& key,
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

auto SpecificDiscoveryStore::GetLabelWithMinimalHandlerSet(DiscoveryKeyNode& keyNode,
                                                           const std::vector<SilKit::Services::MatchingLabel>& labels)
    -> const SilKit::Services::MatchingLabel*
{
    const SilKit::Services::MatchingLabel* outGreedyLabel = nullptr;

    size_t matchCount = keyNode.allCluster.controllerInfo.size();
    // search greedy Cluster guess
    for (auto&& l : labels)
    {
        if (matchCount <= 1)
            break;
        const auto keyTuple = std::make_tuple(l.key, l.value);
        if (l.kind == SilKit::Services::MatchingLabel::Kind::Mandatory)
        {
            const auto& relevantNodeCount = keyNode.labelMap[keyTuple].controllerInfo.size();
            if (relevantNodeCount < matchCount)
            {
                matchCount = relevantNodeCount;
                outGreedyLabel = &l;
            }
        }
        else if (l.kind == SilKit::Services::MatchingLabel::Kind::Optional)
        {
            const auto labeled_matches = keyNode.labelMap[keyTuple].controllerInfo.size();
            const auto distinct_matches = keyNode.notLabelMap[l.key].controllerInfo.size();

            const size_t relevantNodeCount = labeled_matches + distinct_matches;
            if ( relevantNodeCount > 0 && relevantNodeCount < matchCount)
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
            if ( relevantNodeCount > 0 && relevantNodeCount < matchCount)
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
                for (auto& controllerInfo : entry.allCluster.controllerInfo)
                {
                    entry.notLabelMap[l.key].controllerInfo.emplace_back(controllerInfo);
                }
            }
        }

        // insert for every label
        for (auto&& keyval : entry.notLabelMap)
        {
            const auto& labelKey = keyval.first;
            auto foundLabel =
                std::find_if(labels.begin(), labels.end(), [&labelKey](const auto& ml) { return ml.key == labelKey; });
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
    UpdateDiscoveryClusters(controllerType_, key, labels,
                            [&serviceDescriptor](auto& cluster) { cluster.nodes.push_back(serviceDescriptor); });
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

    for (auto&& keyval : entry.notLabelMap)
    {
        auto& cluster = keyval.second;
        cluster.nodes.erase(std::remove(cluster.nodes.begin(), cluster.nodes.end(), serviceDescriptor),
                            cluster.nodes.end());
    }
    for (auto&& keyval : entry.labelMap)
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
    auto controllerInfo = std::make_shared<ControllerCluster>(ControllerCluster(std::move(handler), labels));
    UpdateDiscoveryClusters(controllerType_, key, labels,
                            [controllerInfo](auto& cluster) { cluster.controllerInfo.push_back(controllerInfo); });
}

const std::vector<SilKit::Services::MatchingLabel> SpecificDiscoveryStore::GetLabels(
    const ServiceDescriptor& descriptor)
{
    const auto ctrlType = descriptor.GetSupplementalDataValue(Core::Discovery::controllerType);

    std::string labelsStr;

    if(ctrlType == controllerTypeDataPublisher)
    {
        labelsStr = descriptor.GetSupplementalDataValue(Core::Discovery::supplKeyDataPublisherPubLabels);
    }
    else if(ctrlType == controllerTypeRpcClient)
    {
        labelsStr = descriptor.GetSupplementalDataValue(Core::Discovery::supplKeyRpcClientLabels);
    }
    else
    {
        // Don't need labels return an empty vector
        return std::vector<SilKit::Services::MatchingLabel>();
    }

    const auto descriptorLabels =
        SilKit::Config::Deserialize<std::vector<SilKit::Services::MatchingLabel>>(labelsStr);
    return descriptorLabels;

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
