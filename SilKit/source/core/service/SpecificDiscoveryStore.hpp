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
#include <memory>
#include <string>
#include <functional>
#include <map>

#include "IServiceDiscovery.hpp"
#include "Hash.hpp"

namespace SilKit {
namespace Core {
namespace Discovery {

// Internal Data Types
using ControllerType = std::string;
using TopicOrKey = std::string;
using FilterType = std::tuple<ControllerType, TopicOrKey>;
struct FilterTypeHash
{
    std::size_t operator()(const FilterType& ft) const
    {
        return static_cast<size_t>(
          SilKit::Util::Hash::HashCombine(std::hash<ControllerType>()(std::get<0>(ft)),
                                                                   (std::hash<TopicOrKey>()(std::get<1>(ft))))
          );
    }
};

using HandlerValue = std::shared_ptr<ServiceDiscoveryHandler>;

//! Stores all potential nodes (service descriptors) and handlers to call for a specific data matching branch
class DiscoveryCluster
{
public:
    std::vector<ServiceDescriptor> nodes;
    std::vector<HandlerValue> handlers;
};

//! Holds all relevant information for a controllerType and key (topic/functionName/clientUUID)
class DiscoveryKeyNode
{
public:
    //!< Stores all handlers/nodes for a specific label key (first tuple parameter) and value (second tuple parameter)
    std::unordered_map<FilterType, DiscoveryCluster, FilterTypeHash> labelMap;
    //!< Stores all handlers/nodes that do not have a specific label
    std::unordered_map<TopicOrKey, DiscoveryCluster> notLabelMap;
    //!< Stores all handlers/nodes that do not have any label
    DiscoveryCluster noLabelCluster;
    //!< Stores all handlers/nodes for a controllerType and key
    DiscoveryCluster allCluster;
};

//! Store to prevent quadratic lookup of services
class SpecificDiscoveryStore
{
public: 
   
    /*! \brief Service addition/removal is called on service discovery events
    * 
    *   Note: Implementation is not thread safe, all public API interactions must be secured with a common mutex
    */
    void ServiceChange(ServiceDiscoveryEvent::Type changeType, const ServiceDescriptor& serviceDescriptor);

    /*! \brief Register a specific service discovery handler, that is optimized to pre-filter relevant service discovery events
    *   \parameter handler a callback that is called for pre-filtered service discovery events
    *   \parameter controllerType service discovery controller type to pre-filter
    *   \parameter key used to pre filter the service discovery events; semantics depend on controllerType 
    *      (DataPublisher -> topic, RpcServer -> FunctionName, RpcServerInternal -> clientUUID)
    *   \parameter labels that should match for the filtered service discovery events
    *
    *   Note: handler might be called for service discovery events that only if a subset of the parameter constraints
    *   Implementation is not thread safe, all public API interactions must be secured with a common mutex
    */ 
    void RegisterSpecificServiceDiscoveryHandler(ServiceDiscoveryHandler handler, const std::string& controllerType,
                                                 const std::string& key,
                                                 const std::vector<SilKit::Services::MatchingLabel>& labels);

private: //methods

    //!< Trigger relevant handler calls when a service has changed
    void CallHandlersOnServiceChange(ServiceDiscoveryEvent::Type eventType, const std::string& controllerType,
                                     const std::string& topic, const std::vector<SilKit::Services::MatchingLabel>& labels,
                                     const ServiceDescriptor& serviceDescriptor);

    //!< Trigger handler for past events that happened before registration
    void CallHandlerOnHandlerRegistration(const ServiceDiscoveryHandler& handler, const std::string& controllerType,
                                          const std::string& topic,
                                          const std::vector<SilKit::Services::MatchingLabel>& labels);

    //!< Update the internal lookup structure when a service discovery event happened
    void UpdateLookupOnServiceChange(ServiceDiscoveryEvent::Type eventType, const std::string& supplControllerTypeName,
                                     const std::string& topic,
                                     const std::vector<SilKit::Services::MatchingLabel>& labels,
                                     const ServiceDescriptor& serviceDescriptor);

    //!< Update all DiscoveryClusters within the internal lookup structure
    void UpdateDiscoveryClusters(const std::string& controllerType, const std::string& key,
                                 const std::vector<SilKit::Services::MatchingLabel>& labels,
                                                         std::function<void(DiscoveryCluster&)>);

    //!< Looks for the label that returns a minimal handler set
    auto GetLabelWithMinimalHandlerSet(DiscoveryKeyNode& keyNode,
                                       const std::vector<SilKit::Services::MatchingLabel>& labels)
        -> const SilKit::Services::MatchingLabel*;

    //!< Looks for the label that returns a minimal ServiceDescriptor set
    auto GetLabelWithMinimalNodeSet(DiscoveryKeyNode& keyNode,
                                    const std::vector<SilKit::Services::MatchingLabel>& labels)
        -> const SilKit::Services::MatchingLabel*;

    //!< Insert a new lookup node from internal lookup structure
    void InsertLookupNode(const std::string& controllerType, const std::string& key, 
                           const std::vector<SilKit::Services::MatchingLabel>& labels,
                           const ServiceDescriptor& serviceDescriptor);
    
    //!< Remove a new lookup node from internal lookup structure
    void RemoveLookupNode(const std::string& controllerType, const std::string& key,
                          const ServiceDescriptor& serviceDescriptor);

    //!< Insert a new lookup handler
    void InsertLookupHandler(const std::string& controllerType, const std::string& key, 
                          const std::vector<SilKit::Services::MatchingLabel>& labels,
                             ServiceDiscoveryHandler handler);

private: //member

    //!< SpecificDiscoveryStore is only available to a a sub set of controllers
    const std::unordered_set<std::string> _allowedControllers = {
        controllerTypeDataPublisher, controllerTypeRpcServerInternal, controllerTypeRpcClient};

protected:
    //! NB: container is not thread safe, all public API interactions must be secured with a common mutex
    std::unordered_map<FilterType, DiscoveryKeyNode, FilterTypeHash> _lookup;
};

} // namespace Discovery
} // namespace Core
} // namespace SilKit

