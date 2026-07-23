// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "capi/ServiceObserver.hpp"

#include "silkit/services/datatypes.hpp"

#include "core/internal/ServiceConfigKeys.hpp"

#include "config/YamlParser.hpp"

#include <algorithm>

namespace {

namespace Discovery = SilKit::Core::Discovery;

auto ToC(Discovery::ServiceDiscoveryEvent::Type type) -> SilKit_Experimental_ServiceDiscoveryEvent_Type
{
    using Type = Discovery::ServiceDiscoveryEvent::Type;
    switch (type)
    {
    case Type::ServiceCreated:
        return SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated;
    case Type::ServiceRemoved:
        return SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceRemoved;
    default:
        return SilKit_Experimental_ServiceDiscoveryEvent_Type_Invalid;
    }
}

// Owns the backing storage for the label list so that the borrowed c-string pointers in the
// SilKit_Experimental_ServiceDescriptor remain valid for the duration of the handler invocation.
struct LabelStorage
{
    std::vector<SilKit::Services::MatchingLabel> labels;
    std::vector<SilKit_Label> cLabels;
};

// Maps the internal ServiceDescriptor to the public struct. Returns false if the service is not user-facing
// (infrastructure / internal endpoints), in which case the handler must not be invoked. The connectedParticipantName
// and connectedServiceName fields are initialised to empty strings here; the link path sets them (and overrides
// serviceKind to Link) after this function returns.
auto ClassifyAndFill(const SilKit::Core::ServiceDescriptor& serviceDescriptor,
                     SilKit_Experimental_ServiceDescriptor& out, LabelStorage& storage) -> bool
{
    const auto& supplementalData = serviceDescriptor.GetSupplementalDataRef();

    const auto findValue = [&supplementalData](const std::string& key) -> const char* {
        const auto it = supplementalData.find(key);
        return it == supplementalData.end() ? nullptr : it->second.c_str();
    };

    SilKit_Struct_Init(SilKit_Experimental_ServiceDescriptor, out);
    out.participantName = serviceDescriptor.GetParticipantName().c_str();
    out.serviceName = serviceDescriptor.GetServiceName().c_str();
    out.primaryIdentifier = serviceDescriptor.GetNetworkName().c_str();
    out.mediaType = "";
    out.simulationName = serviceDescriptor.GetSimulationName().c_str();
    out.connectedParticipantName = "";
    out.connectedServiceName = "";

    // Network-simulator links are user-facing: they are reported as a Link service whose primaryIdentifier is
    // the simulated network name (matching the affected bus controllers' primaryIdentifier).
    if (serviceDescriptor.GetServiceType() == SilKit::Core::ServiceType::Link)
    {
        out.serviceKind = SilKit_Experimental_ServiceKind_Link;
        return true;
    }

    std::string controllerType;
    if (!serviceDescriptor.GetSupplementalDataItem(Discovery::controllerType, controllerType))
    {
        return false;
    }

    // Decode the YAML-encoded matching labels into the public label list. A malformed value must not propagate
    // an exception: the service is still reported, just without labels.
    const auto decodeLabels = [&](const std::string& labelsKey) {
        const char* labelsStr = findValue(labelsKey);
        if (labelsStr == nullptr || *labelsStr == '\0')
        {
            return;
        }
        try
        {
            storage.labels = SilKit::Config::Deserialize<std::vector<SilKit::Services::MatchingLabel>>(labelsStr);
        }
        catch (...)
        {
            return;
        }
        storage.cLabels.reserve(storage.labels.size());
        for (const auto& label : storage.labels)
        {
            SilKit_Label cLabel;
            cLabel.key = label.key.c_str();
            cLabel.value = label.value.c_str();
            cLabel.kind = static_cast<SilKit_LabelKind>(label.kind);
            storage.cLabels.push_back(cLabel);
        }
        out.labelList.numLabels = storage.cLabels.size();
        out.labelList.labels = storage.cLabels.data();
    };

    if (controllerType == Discovery::controllerTypeCan)
    {
        out.serviceKind = SilKit_Experimental_ServiceKind_CanController;
    }
    else if (controllerType == Discovery::controllerTypeEthernet)
    {
        out.serviceKind = SilKit_Experimental_ServiceKind_EthernetController;
    }
    else if (controllerType == Discovery::controllerTypeFlexray)
    {
        out.serviceKind = SilKit_Experimental_ServiceKind_FlexrayController;
    }
    else if (controllerType == Discovery::controllerTypeLin)
    {
        out.serviceKind = SilKit_Experimental_ServiceKind_LinController;
    }
    else if (controllerType == Discovery::controllerTypeDataPublisher)
    {
        out.serviceKind = SilKit_Experimental_ServiceKind_DataPublisher;
        if (const char* topic = findValue(Discovery::supplKeyDataPublisherTopic))
        {
            out.primaryIdentifier = topic;
        }
        if (const char* mediaType = findValue(Discovery::supplKeyDataPublisherMediaType))
        {
            out.mediaType = mediaType;
        }
        decodeLabels(Discovery::supplKeyDataPublisherPubLabels);
    }
    else if (controllerType == Discovery::controllerTypeDataSubscriber)
    {
        out.serviceKind = SilKit_Experimental_ServiceKind_DataSubscriber;
        if (const char* topic = findValue(Discovery::supplKeyDataSubscriberTopic))
        {
            out.primaryIdentifier = topic;
        }
        if (const char* mediaType = findValue(Discovery::supplKeyDataSubscriberMediaType))
        {
            out.mediaType = mediaType;
        }
        decodeLabels(Discovery::supplKeyDataSubscriberSubLabels);
    }
    else if (controllerType == Discovery::controllerTypeRpcClient)
    {
        out.serviceKind = SilKit_Experimental_ServiceKind_RpcClient;
        if (const char* functionName = findValue(Discovery::supplKeyRpcClientFunctionName))
        {
            out.primaryIdentifier = functionName;
        }
        if (const char* mediaType = findValue(Discovery::supplKeyRpcClientMediaType))
        {
            out.mediaType = mediaType;
        }
        decodeLabels(Discovery::supplKeyRpcClientLabels);
    }
    else if (controllerType == Discovery::controllerTypeRpcServer)
    {
        out.serviceKind = SilKit_Experimental_ServiceKind_RpcServer;
        if (const char* functionName = findValue(Discovery::supplKeyRpcServerFunctionName))
        {
            out.primaryIdentifier = functionName;
        }
        if (const char* mediaType = findValue(Discovery::supplKeyRpcServerMediaType))
        {
            out.mediaType = mediaType;
        }
        decodeLabels(Discovery::supplKeyRpcServerLabels);
    }
    else
    {
        // Infrastructure / internal controllers (ServiceDiscovery, SystemMonitor, lifecycle, metrics,
        // DataSubscriberInternal, RpcServerInternal, ...) are not user-facing and are not reported.
        return false;
    }

    return true;
}

auto TryParseEndpointId(const std::string& text, SilKit::Core::EndpointId& out) -> bool
{
    try
    {
        out = static_cast<SilKit::Core::EndpointId>(std::stoull(text));
        return true;
    }
    catch (...)
    {
        return false;
    }
}

} // namespace


namespace VSilKit {

ServiceObserver::ServiceObserver(SilKit_Experimental_ServiceDiscoveryHandler_t handler, void* context)
    : _handler{handler}
    , _context{context}
{
}

auto ServiceObserver::MakeLink(const SilKit::Core::ServiceDescriptor& parent,
                               const SilKit::Core::ServiceDescriptor& peer) -> LinkEmission
{
    return LinkEmission{parent, peer.GetParticipantName(), peer.GetServiceName()};
}

void ServiceObserver::EmitService(SilKit_Experimental_ServiceDiscoveryEvent_Type type,
                                  const SilKit::Core::ServiceDescriptor& descriptor)
{
    SilKit_Experimental_ServiceDescriptor out{};
    LabelStorage storage;
    if (!ClassifyAndFill(descriptor, out, storage))
    {
        return;
    }
    _handler(_context, type, &out);
}

void ServiceObserver::EmitLink(const LinkEmission& emission)
{
    SilKit_Experimental_ServiceDescriptor out{};
    LabelStorage storage;
    if (!ClassifyAndFill(emission.parentDescriptor, out, storage))
    {
        return;
    }
    out.serviceKind = SilKit_Experimental_ServiceKind_Link;
    out.connectedParticipantName = emission.connectedParticipantName.c_str();
    out.connectedServiceName = emission.connectedServiceName.c_str();
    _handler(_context, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated, &out);
}

void ServiceObserver::DrainResolvablePending(std::vector<LinkEmission>& emissions)
{
    auto it = _pending.begin();
    while (it != _pending.end())
    {
        const auto parentIt = _parents.find(it->parentKey);
        const auto peerIt = _peersByUuid.find(it->peerUuid);
        if (parentIt != _parents.end() && peerIt != _peersByUuid.end())
        {
            emissions.push_back(MakeLink(parentIt->second, peerIt->second));
            it = _pending.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

// An internal-match endpoint (DataSubscriberInternal / RpcServerInternal) appeared or disappeared. On creation it is
// turned into a Link event once its parent (subscriber/server) and peer (publisher/client) are both known; if either
// is still missing it is remembered in the pending list. No Link removal event is emitted: a match teardown always
// coincides with a ServiceRemoved of one of its endpoints, from which the link's disappearance is inferred.
void ServiceObserver::HandleInternalMatch(Discovery::ServiceDiscoveryEvent::Type type,
                                          const SilKit::Core::ServiceDescriptor& descriptor,
                                          const std::string& parentIdKey, const std::string& peerUuid)
{
    using EventType = Discovery::ServiceDiscoveryEvent::Type;

    std::string parentIdStr;
    if (!descriptor.GetSupplementalDataItem(parentIdKey, parentIdStr))
    {
        return;
    }
    SilKit::Core::EndpointId parentServiceId{0};
    if (!TryParseEndpointId(parentIdStr, parentServiceId))
    {
        return;
    }
    const ServiceKey parentKey{descriptor.GetParticipantName(), parentServiceId};

    std::vector<LinkEmission> emissions;
    {
        std::lock_guard<std::mutex> lock{_mutex};
        if (type == EventType::ServiceCreated)
        {
            const auto parentIt = _parents.find(parentKey);
            const auto peerIt = _peersByUuid.find(peerUuid);
            if (parentIt != _parents.end() && peerIt != _peersByUuid.end())
            {
                emissions.push_back(MakeLink(parentIt->second, peerIt->second));
            }
            else
            {
                _pending.push_back(PendingMatch{parentKey, peerUuid});
            }
        }
        else if (type == EventType::ServiceRemoved)
        {
            _pending.erase(std::remove_if(_pending.begin(), _pending.end(),
                                          [&](const PendingMatch& p) {
                return p.parentKey == parentKey && p.peerUuid == peerUuid;
            }),
                           _pending.end());
        }
    }
    for (const auto& e : emissions)
    {
        EmitLink(e);
    }
}

// A peer (DataPublisher / RpcClient) appeared or disappeared. Its identity (keyed by its networkName / UUID) enables
// resolving matches. The peer's own ServiceCreated/ServiceRemoved is always emitted.
void ServiceObserver::HandlePeer(Discovery::ServiceDiscoveryEvent::Type type,
                                 const SilKit::Core::ServiceDescriptor& descriptor)
{
    using EventType = Discovery::ServiceDiscoveryEvent::Type;

    const std::string peerUuid = descriptor.GetNetworkName();
    std::vector<LinkEmission> emissions;
    {
        std::lock_guard<std::mutex> lock{_mutex};
        if (type == EventType::ServiceCreated)
        {
            _peersByUuid[peerUuid] = descriptor;
            DrainResolvablePending(emissions);
        }
        else if (type == EventType::ServiceRemoved)
        {
            _peersByUuid.erase(peerUuid);
            _pending.erase(std::remove_if(_pending.begin(), _pending.end(),
                                          [&](const PendingMatch& p) { return p.peerUuid == peerUuid; }),
                           _pending.end());
        }
    }
    EmitService(ToC(type), descriptor);
    for (const auto& e : emissions)
    {
        EmitLink(e);
    }
}

// A parent (DataSubscriber / RpcServer) appeared or disappeared. Its identity (keyed by participant + serviceId)
// enables resolving matches. The parent's own ServiceCreated/ServiceRemoved is always emitted.
void ServiceObserver::HandleParent(Discovery::ServiceDiscoveryEvent::Type type,
                                   const SilKit::Core::ServiceDescriptor& descriptor)
{
    using EventType = Discovery::ServiceDiscoveryEvent::Type;

    const ServiceKey key{descriptor.GetParticipantName(), descriptor.GetServiceId()};
    std::vector<LinkEmission> emissions;
    {
        std::lock_guard<std::mutex> lock{_mutex};
        if (type == EventType::ServiceCreated)
        {
            _parents[key] = descriptor;
            DrainResolvablePending(emissions);
        }
        else if (type == EventType::ServiceRemoved)
        {
            _parents.erase(key);
            _pending.erase(std::remove_if(_pending.begin(), _pending.end(),
                                          [&](const PendingMatch& p) { return p.parentKey == key; }),
                           _pending.end());
        }
    }
    EmitService(ToC(type), descriptor);
    for (const auto& e : emissions)
    {
        EmitLink(e);
    }
}

void ServiceObserver::HandleEvent(Discovery::ServiceDiscoveryEvent::Type type,
                                  const SilKit::Core::ServiceDescriptor& descriptor)
{
    // Network-simulator links are reported directly as a Link service (created and removed: a detaching simulator
    // leaves its controllers alive, so its removal is not otherwise observable).
    if (descriptor.GetServiceType() == SilKit::Core::ServiceType::Link)
    {
        EmitService(ToC(type), descriptor);
        return;
    }

    std::string controllerType;
    descriptor.GetSupplementalDataItem(Discovery::controllerType, controllerType);

    // Internal-match endpoints: a confirmed pub/sub or RPC match, surfaced as a Link event.
    if (controllerType == Discovery::controllerTypeDataSubscriberInternal)
    {
        HandleInternalMatch(type, descriptor, Discovery::supplKeyDataSubscriberInternalParentServiceID,
                            descriptor.GetNetworkName());
        return;
    }
    if (controllerType == Discovery::controllerTypeRpcServerInternal)
    {
        std::string clientUuid;
        descriptor.GetSupplementalDataItem(Discovery::supplKeyRpcServerInternalClientUUID, clientUuid);
        HandleInternalMatch(type, descriptor, Discovery::supplKeyRpcServerInternalParentServiceID, clientUuid);
        return;
    }

    // Peers: DataPublisher / RpcClient (the connecting side of a match).
    if (controllerType == Discovery::controllerTypeDataPublisher
        || controllerType == Discovery::controllerTypeRpcClient)
    {
        HandlePeer(type, descriptor);
        return;
    }

    // Parents: DataSubscriber / RpcServer (the receiving side of a match).
    if (controllerType == Discovery::controllerTypeDataSubscriber
        || controllerType == Discovery::controllerTypeRpcServer)
    {
        HandleParent(type, descriptor);
        return;
    }

    // Bus controllers are emitted as-is; infrastructure / internal services are suppressed by ClassifyAndFill.
    EmitService(ToC(type), descriptor);
}

} // namespace VSilKit
