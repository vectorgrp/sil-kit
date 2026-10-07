// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "capi/ServiceObserver.hpp"

#include "silkit/capi/Orchestration.h"
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

// The bus type of a controller or simulated network; pub/sub, RPC and internal services have none.
auto ToSimulatedNetworkType(SilKit::Config::NetworkType networkType) -> SilKit_Experimental_SimulatedNetworkType
{
    using SilKit::Config::NetworkType;
    switch (networkType)
    {
    case NetworkType::CAN:
        return SilKit_NetworkType_CAN;
    case NetworkType::Ethernet:
        return SilKit_NetworkType_Ethernet;
    case NetworkType::FlexRay:
        return SilKit_NetworkType_FlexRay;
    case NetworkType::LIN:
        return SilKit_NetworkType_LIN;
    default:
        return SilKit_NetworkType_Undefined;
    }
}

// The operation mode stored by the LifecycleService (its numeric SilKit_OperationMode value).
auto ParseOperationMode(const char* value) -> SilKit_OperationMode
{
    const std::string text = value == nullptr ? "" : value;
    if (text == std::to_string(SilKit_OperationMode_Coordinated))
    {
        return SilKit_OperationMode_Coordinated;
    }
    if (text == std::to_string(SilKit_OperationMode_Autonomous))
    {
        return SilKit_OperationMode_Autonomous;
    }
    return SilKit_OperationMode_Invalid;
}

// Maps the internal ServiceDescriptor to the public struct. Returns false if the service is not user-facing
// (infrastructure / internal endpoints), in which case the handler must not be invoked. The connected... fields are
// initialised to empty values here; the pub/sub and RPC match path sets them (and overrides serviceKind) after this
// function returns.
auto ClassifyAndFill(const SilKit::Core::ServiceDescriptor& serviceDescriptor, bool isSnapshot,
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
    out.serviceId = serviceDescriptor.GetServiceId();
    out.primaryIdentifier = serviceDescriptor.GetNetworkName().c_str();
    out.networkType = ToSimulatedNetworkType(serviceDescriptor.GetNetworkType());
    out.mediaType = "";
    out.operationMode = SilKit_OperationMode_Invalid;
    out.timeSyncActive = SilKit_False;
    out.connectedParticipantName = "";
    out.connectedServiceName = "";
    out.connectedServiceId = 0;
    out.isSnapshot = isSnapshot ? SilKit_True : SilKit_False;

    // Network-simulator links: primaryIdentifier and networkType are those of the simulated network, matching the
    // affected bus controllers.
    if (serviceDescriptor.GetServiceType() == SilKit::Core::ServiceType::Link)
    {
        out.serviceKind = SilKit_Experimental_ServiceKind_NetworkSimulatorLink;
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
    else if (controllerType == Discovery::controllerTypeLifecycleService)
    {
        // Announced by StartLifecycle, after the operation mode has been stored in the descriptor.
        out.serviceKind = SilKit_Experimental_ServiceKind_LifecycleService;
        out.primaryIdentifier = "";
        out.operationMode = ParseOperationMode(findValue(Discovery::lifecycleIsCoordinated));
    }
    else if (controllerType == Discovery::controllerTypeTimeSyncService)
    {
        out.serviceKind = SilKit_Experimental_ServiceKind_TimeSyncService;
        out.primaryIdentifier = "";
        const char* active = findValue(Discovery::timeSyncActive);
        out.timeSyncActive = (active != nullptr && std::string{active} == "1") ? SilKit_True : SilKit_False;
    }
    else
    {
        // Infrastructure / internal controllers (ServiceDiscovery, SystemMonitor, metrics,
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

auto ServiceObserver::MakeMatch(const SilKit::Core::ServiceDescriptor& parent,
                               const SilKit::Core::ServiceDescriptor& peer, SilKit_Experimental_ServiceKind kind)
    -> MatchEmission
{
    return MatchEmission{parent, peer.GetParticipantName(), peer.GetServiceName(), peer.GetServiceId(), kind};
}

void ServiceObserver::EmitService(SilKit_Experimental_ServiceDiscoveryEvent_Type type,
                                  const SilKit::Core::ServiceDescriptor& descriptor)
{
    SilKit_Experimental_ServiceDescriptor out{};
    LabelStorage storage;
    if (!ClassifyAndFill(descriptor, _isSnapshot, out, storage))
    {
        return;
    }
    Invoke(type, out);
}

void ServiceObserver::EmitMatch(SilKit_Experimental_ServiceDiscoveryEvent_Type type, const MatchEmission& emission)
{
    SilKit_Experimental_ServiceDescriptor out{};
    LabelStorage storage;
    if (!ClassifyAndFill(emission.parentDescriptor, _isSnapshot, out, storage))
    {
        return;
    }
    out.serviceKind = emission.kind;
    out.connectedParticipantName = emission.connectedParticipantName.c_str();
    out.connectedServiceName = emission.connectedServiceName.c_str();
    out.connectedServiceId = emission.connectedServiceId;
    Invoke(type, out);
}

// A throwing handler must not cost the remaining emissions of the same event.
void ServiceObserver::Invoke(SilKit_Experimental_ServiceDiscoveryEvent_Type type,
                             const SilKit_Experimental_ServiceDescriptor& descriptor)
{
    try
    {
        _handler(_context, type, &descriptor);
    }
    catch (...)
    {
    }
}

void ServiceObserver::EmitAll(SilKit_Experimental_ServiceDiscoveryEvent_Type type,
                              const std::vector<MatchEmission>& emissions)
{
    for (const auto& emission : emissions)
    {
        EmitMatch(type, emission);
    }
}

auto ServiceObserver::TryResolve(const PendingMatch& match, std::vector<MatchEmission>& created) -> bool
{
    const auto parentIt = _parents.find(match.parentKey);
    const auto peerIt = _peersByUuid.find(match.peerUuid);
    if (parentIt == _parents.end() || peerIt == _peersByUuid.end())
    {
        return false;
    }
    const MatchKey key{match.parentKey, match.peerUuid};
    if (_emitted.count(key) == 0)
    {
        auto emission = MakeMatch(parentIt->second, peerIt->second, match.kind);
        _emitted.emplace(key, emission);
        created.push_back(std::move(emission));
    }
    return true;
}

void ServiceObserver::DrainResolvablePending(std::vector<MatchEmission>& created)
{
    auto it = _pending.begin();
    while (it != _pending.end())
    {
        it = TryResolve(*it, created) ? _pending.erase(it) : std::next(it);
    }
}

template <typename Predicate>
void ServiceObserver::TakeEmitted(Predicate predicate, std::vector<MatchEmission>& removed)
{
    for (auto it = _emitted.begin(); it != _emitted.end();)
    {
        if (predicate(it->first))
        {
            removed.push_back(std::move(it->second));
            it = _emitted.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

// An internal-match endpoint (DataSubscriberInternal / RpcServerInternal) appeared or disappeared. On creation it
// becomes a PubSubMatch / RpcMatch once its parent (subscriber/server) and peer (publisher/client) are both known;
// until then it is remembered in the pending list. Its removal removes the match.
void ServiceObserver::HandleInternalMatch(Discovery::ServiceDiscoveryEvent::Type type,
                                          const SilKit::Core::ServiceDescriptor& descriptor,
                                          const std::string& parentIdKey, const std::string& peerUuid,
                                          SilKit_Experimental_ServiceKind kind)
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
    const MatchKey matchKey{parentKey, peerUuid};

    std::vector<MatchEmission> created;
    std::vector<MatchEmission> removed;
    {
        std::lock_guard<std::mutex> lock{_mutex};
        const auto isThisMatch = [&](const PendingMatch& p) {
            return p.parentKey == parentKey && p.peerUuid == peerUuid;
        };
        if (type == EventType::ServiceCreated)
        {
            const PendingMatch match{parentKey, peerUuid, kind};
            if (!TryResolve(match, created) && std::none_of(_pending.begin(), _pending.end(), isThisMatch))
            {
                _pending.push_back(match);
            }
        }
        else if (type == EventType::ServiceRemoved)
        {
            _pending.erase(std::remove_if(_pending.begin(), _pending.end(), isThisMatch), _pending.end());
            TakeEmitted([&](const MatchKey& key) { return key == matchKey; }, removed);
        }
    }
    EmitAll(SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceRemoved, removed);
    EmitAll(SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated, created);
}

// A peer (DataPublisher / RpcClient) appeared or disappeared. Its identity (keyed by its networkName / UUID) enables
// resolving matches. Its matches are created after and removed before the peer's own ServiceCreated/ServiceRemoved.
void ServiceObserver::HandlePeer(Discovery::ServiceDiscoveryEvent::Type type,
                                 const SilKit::Core::ServiceDescriptor& descriptor)
{
    using EventType = Discovery::ServiceDiscoveryEvent::Type;

    const std::string peerUuid = descriptor.GetNetworkName();
    std::vector<MatchEmission> created;
    std::vector<MatchEmission> removed;
    {
        std::lock_guard<std::mutex> lock{_mutex};
        if (type == EventType::ServiceCreated)
        {
            _peersByUuid[peerUuid] = descriptor;
            DrainResolvablePending(created);
        }
        else if (type == EventType::ServiceRemoved)
        {
            _peersByUuid.erase(peerUuid);
            _pending.erase(std::remove_if(_pending.begin(), _pending.end(),
                                          [&](const PendingMatch& p) { return p.peerUuid == peerUuid; }),
                           _pending.end());
            TakeEmitted([&](const MatchKey& key) { return key.second == peerUuid; }, removed);
        }
    }
    EmitAll(SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceRemoved, removed);
    EmitService(ToC(type), descriptor);
    EmitAll(SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated, created);
}

// A parent (DataSubscriber / RpcServer) appeared or disappeared. Its identity (keyed by participant + serviceId)
// enables resolving matches. Its matches are created after and removed before the parent's own
// ServiceCreated/ServiceRemoved.
void ServiceObserver::HandleParent(Discovery::ServiceDiscoveryEvent::Type type,
                                   const SilKit::Core::ServiceDescriptor& descriptor)
{
    using EventType = Discovery::ServiceDiscoveryEvent::Type;

    const ServiceKey key{descriptor.GetParticipantName(), descriptor.GetServiceId()};
    std::vector<MatchEmission> created;
    std::vector<MatchEmission> removed;
    {
        std::lock_guard<std::mutex> lock{_mutex};
        if (type == EventType::ServiceCreated)
        {
            _parents[key] = descriptor;
            DrainResolvablePending(created);
        }
        else if (type == EventType::ServiceRemoved)
        {
            _parents.erase(key);
            _pending.erase(std::remove_if(_pending.begin(), _pending.end(),
                                          [&](const PendingMatch& p) { return p.parentKey == key; }),
                           _pending.end());
            TakeEmitted([&](const MatchKey& matchKey) { return matchKey.first == key; }, removed);
        }
    }
    EmitAll(SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceRemoved, removed);
    EmitService(ToC(type), descriptor);
    EmitAll(SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated, created);
}

void ServiceObserver::HandleEvent(Discovery::ServiceDiscoveryEvent::Type type,
                                  const SilKit::Core::ServiceDescriptor& descriptor, bool isSnapshot)
{
    // Everything emitted for this event (the service itself and matches it resolves) carries the flag.
    _isSnapshot = isSnapshot;

    // Network-simulator links are reported directly as a NetworkSimulatorLink service.
    if (descriptor.GetServiceType() == SilKit::Core::ServiceType::Link)
    {
        EmitService(ToC(type), descriptor);
        return;
    }

    std::string controllerType;
    descriptor.GetSupplementalDataItem(Discovery::controllerType, controllerType);

    // Internal-match endpoints: a confirmed pub/sub or RPC match, surfaced as a PubSubMatch / RpcMatch.
    if (controllerType == Discovery::controllerTypeDataSubscriberInternal)
    {
        HandleInternalMatch(type, descriptor, Discovery::supplKeyDataSubscriberInternalParentServiceID,
                            descriptor.GetNetworkName(), SilKit_Experimental_ServiceKind_PubSubMatch);
        return;
    }
    if (controllerType == Discovery::controllerTypeRpcServerInternal)
    {
        std::string clientUuid;
        descriptor.GetSupplementalDataItem(Discovery::supplKeyRpcServerInternalClientUUID, clientUuid);
        HandleInternalMatch(type, descriptor, Discovery::supplKeyRpcServerInternalParentServiceID, clientUuid,
                            SilKit_Experimental_ServiceKind_RpcMatch);
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
