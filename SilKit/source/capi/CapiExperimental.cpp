// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "silkit/capi/SilKit.h"
#include "silkit/SilKit.hpp"
#include "silkit/participant/exception.hpp"
#include "silkit/services/datatypes.hpp"

#include "capi/CapiImpl.hpp"

#include "core/internal/IParticipantInternal.hpp"
#include "core/service/IServiceDiscovery.hpp"
#include "core/internal/ServiceDescriptor.hpp"
#include "core/internal/ServiceConfigKeys.hpp"

#include "config/YamlParser.hpp"

#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace {

namespace Discovery = SilKit::Core::Discovery;

auto GetServiceDiscovery(SilKit_Participant* participant) -> Discovery::IServiceDiscovery*
{
    auto* cppParticipant = reinterpret_cast<SilKit::IParticipant*>(participant);
    auto* participantInternal = dynamic_cast<SilKit::Core::IParticipantInternal*>(cppParticipant);
    if (participantInternal == nullptr)
    {
        throw SilKit::SilKitError{"participant is not a valid SilKit::IParticipant*"};
    }
    return participantInternal->GetServiceDiscovery();
}

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
// (infrastructure / internal services / network links), in which case the handler must not be invoked.
// Connection-specific and simulation-specific fields (numberOfConnections, connectedParticipantName,
// connectedServiceName, isSimulated, simulatingParticipantName) are initialised to safe empty values
// here; callers set them from PendingEmission after this function returns.
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
    out.simulatingParticipantName = "";
    // numberOfConnections = 0, isSimulated = SilKit_False (from memset above)

    // Network links are handled internally (fan-out to bus controllers) and are not exposed directly.
    if (serviceDescriptor.GetServiceType() == SilKit::Core::ServiceType::Link)
    {
        return false;
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

// Identifies a service across the simulation: (participant name, service id).
using ServiceKey = std::pair<std::string, SilKit::Core::EndpointId>;
// Identifies a network: (network name, network type). Used to correlate bus controllers with links.
using NetworkKey = std::pair<std::string, SilKit::Config::NetworkType>;

// Peer identity for pub/sub and RPC connection resolution.
struct PeerInfo
{
    std::string participantName;
    std::string serviceName;
};

// Tracks a user-facing service (DataSubscriber or RpcServer) for connection counting and peer reveal.
// connKeys holds the internal-connection endpoints attached to this parent; its size is the connection
// count. Endpoints may be recorded before the parent's own descriptor is known (out-of-order replay),
// in which case haveDescriptor is false until the parent is discovered.
struct TrackedEntry
{
    SilKit::Core::ServiceDescriptor descriptor;
    bool haveDescriptor{false};
    std::set<ServiceKey> connKeys;
};

// A confirmed connection endpoint (DataSubscriberInternal / RpcServerInternal): links the internal
// endpoint to its parent (user-facing subscriber/server) and remembers the peer's UUID (the
// DataPublisher / RpcClient networkName) so the peer identity can be resolved, possibly later.
struct ConnInfo
{
    ServiceKey parentKey;
    std::string peerUuid;
};

// A synthesized event, queued while the state lock is held and dispatched after it is released.
// All std::string members hold backing storage for the pointer fields set in the emitted struct.
struct PendingEmission
{
    SilKit_Experimental_ServiceDiscoveryEvent_Type type{SilKit_Experimental_ServiceDiscoveryEvent_Type_Invalid};
    SilKit::Core::ServiceDescriptor descriptor;
    uint32_t numberOfConnections{0};
    std::string connectedParticipantName;
    std::string connectedServiceName;
    bool isSimulated{false};
    std::string simulatingParticipantName;
};

// Per-observer bookkeeping. Lives as long as the participant's service discovery, because it is
// captured by the registered handler (which is never unregistered).
struct ObserverState
{
    SilKit_Experimental_ServiceDiscoveryHandler_t handler{};
    void* context{nullptr};

    std::mutex mutex;

    // DataSubscribers and RpcServers tracked for connection counting and peer resolution.
    std::map<ServiceKey, TrackedEntry> tracked;
    // Internal-connection endpoint key -> connection info (parent key + peer UUID).
    std::map<ServiceKey, ConnInfo> connections;
    // Peer UUID -> connection endpoints whose peer identity is not yet known, awaiting the peer's own
    // discovery. Enables revealing the peer of a connection seen before its DataPublisher / RpcClient.
    std::map<std::string, std::set<ServiceKey>> pendingByPeerUuid;
    // DataPublisher / RpcClient UUID (= their networkName) -> peer identity. Keyed by globally unique
    // UUID, so publishers and clients share this map without collision.
    std::map<std::string, PeerInfo> peersByUuid;

    // Bus controllers by ServiceKey -> stored descriptor, for link-event fan-out.
    std::map<ServiceKey, SilKit::Core::ServiceDescriptor> busControllers;
    // NetworkKey -> set of bus controller ServiceKeys (reverse lookup for link events).
    std::map<NetworkKey, std::set<ServiceKey>> controllersByNetwork;

    // Active network simulators: NetworkKey -> simulator's participant name.
    std::map<NetworkKey, std::string> activeLinks;
};

void EmitPending(ObserverState& state, const PendingEmission& e)
{
    SilKit_Experimental_ServiceDescriptor out{};
    LabelStorage storage;
    if (!ClassifyAndFill(e.descriptor, out, storage))
    {
        return;
    }
    out.numberOfConnections = e.numberOfConnections;
    out.connectedParticipantName = e.connectedParticipantName.c_str();
    out.connectedServiceName = e.connectedServiceName.c_str();
    out.isSimulated = e.isSimulated ? SilKit_True : SilKit_False;
    out.simulatingParticipantName = e.simulatingParticipantName.c_str();
    state.handler(state.context, e.type, &out);
}

// ---- Connection bookkeeping helpers. All are called with state.mutex held and append synthesized
//      ServiceUpdated emissions for the *parent* service (subscriber/server); the emissions are always
//      dispatched by the caller after the lock is released. Peer identity is filled in when known, and
//      revealed via a later ServiceUpdated once the peer (publisher/client) is discovered. This keeps
//      the connection graph reconstructable regardless of discovery/replay ordering. ----

auto MakeParentUpdate(const TrackedEntry& entry, const PeerInfo* peer) -> PendingEmission
{
    PendingEmission e;
    e.type = SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceUpdated;
    e.descriptor = entry.descriptor;
    e.numberOfConnections = static_cast<uint32_t>(entry.connKeys.size());
    if (peer != nullptr)
    {
        e.connectedParticipantName = peer->participantName;
        e.connectedServiceName = peer->serviceName;
    }
    return e;
}

// A parent (DataSubscriber / RpcServer) was created: emit its ServiceCreated and reveal the peers of
// any connections already recorded against it (possible when discovery replays out of order).
void OnParentCreated(ObserverState& state, const ServiceKey& parentKey,
                     const SilKit::Core::ServiceDescriptor& descriptor, std::vector<PendingEmission>& emissions)
{
    auto& entry = state.tracked[parentKey];
    entry.descriptor = descriptor;
    entry.haveDescriptor = true;

    PendingEmission created;
    created.type = SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated;
    created.descriptor = descriptor;
    created.numberOfConnections = static_cast<uint32_t>(entry.connKeys.size());
    emissions.push_back(std::move(created));

    for (const auto& connKey : entry.connKeys)
    {
        const auto cit = state.connections.find(connKey);
        if (cit == state.connections.end())
        {
            continue;
        }
        const auto pit = state.peersByUuid.find(cit->second.peerUuid);
        if (pit != state.peersByUuid.end())
        {
            emissions.push_back(MakeParentUpdate(entry, &pit->second));
        }
        else
        {
            state.pendingByPeerUuid[cit->second.peerUuid].insert(connKey);
        }
    }
}

// A parent (DataSubscriber / RpcServer) was removed: drop it and all of its connection bookkeeping.
void OnParentRemoved(ObserverState& state, const ServiceKey& parentKey)
{
    const auto tit = state.tracked.find(parentKey);
    if (tit == state.tracked.end())
    {
        return;
    }
    for (const auto& connKey : tit->second.connKeys)
    {
        const auto cit = state.connections.find(connKey);
        if (cit == state.connections.end())
        {
            continue;
        }
        const auto pit = state.pendingByPeerUuid.find(cit->second.peerUuid);
        if (pit != state.pendingByPeerUuid.end())
        {
            pit->second.erase(connKey);
            if (pit->second.empty())
            {
                state.pendingByPeerUuid.erase(pit);
            }
        }
        state.connections.erase(cit);
    }
    state.tracked.erase(tit);
}

// A confirmed connection endpoint (DataSubscriberInternal / RpcServerInternal) appeared.
void OnConnectionCreated(ObserverState& state, const ServiceKey& connKey, const ServiceKey& parentKey,
                         const std::string& peerUuid, std::vector<PendingEmission>& emissions)
{
    if (state.connections.count(connKey) != 0)
    {
        return; // already counted
    }
    state.connections[connKey] = ConnInfo{parentKey, peerUuid};
    auto& entry = state.tracked[parentKey];
    entry.connKeys.insert(connKey);

    if (!entry.haveDescriptor)
    {
        return; // revealed when the parent itself is discovered
    }
    const auto pit = state.peersByUuid.find(peerUuid);
    if (pit != state.peersByUuid.end())
    {
        emissions.push_back(MakeParentUpdate(entry, &pit->second));
    }
    else
    {
        // Surface the new connection count now; reveal the peer once its publisher/client is known.
        emissions.push_back(MakeParentUpdate(entry, nullptr));
        state.pendingByPeerUuid[peerUuid].insert(connKey);
    }
}

// A confirmed connection endpoint was removed (a match dissolved).
void OnConnectionRemoved(ObserverState& state, const ServiceKey& connKey, std::vector<PendingEmission>& emissions)
{
    const auto cit = state.connections.find(connKey);
    if (cit == state.connections.end())
    {
        return;
    }
    const auto parentKey = cit->second.parentKey;
    const auto peerUuid = cit->second.peerUuid;
    state.connections.erase(cit);

    const auto pit = state.pendingByPeerUuid.find(peerUuid);
    if (pit != state.pendingByPeerUuid.end())
    {
        pit->second.erase(connKey);
        if (pit->second.empty())
        {
            state.pendingByPeerUuid.erase(pit);
        }
    }

    const auto tit = state.tracked.find(parentKey);
    if (tit == state.tracked.end())
    {
        return;
    }
    tit->second.connKeys.erase(connKey);
    if (tit->second.haveDescriptor)
    {
        const auto peerIt = state.peersByUuid.find(peerUuid);
        const PeerInfo* peer = peerIt != state.peersByUuid.end() ? &peerIt->second : nullptr;
        emissions.push_back(MakeParentUpdate(tit->second, peer));
    }
}

// A peer (DataPublisher / RpcClient) appeared: remember its identity and reveal it to any connection
// that was recorded before the peer was known.
void OnPeerCreated(ObserverState& state, const std::string& peerUuid, PeerInfo peerInfo,
                   std::vector<PendingEmission>& emissions)
{
    state.peersByUuid[peerUuid] = std::move(peerInfo);
    const auto pit = state.pendingByPeerUuid.find(peerUuid);
    if (pit == state.pendingByPeerUuid.end())
    {
        return;
    }
    const auto& peer = state.peersByUuid[peerUuid];
    for (const auto& connKey : pit->second)
    {
        const auto cit = state.connections.find(connKey);
        if (cit == state.connections.end())
        {
            continue;
        }
        const auto tit = state.tracked.find(cit->second.parentKey);
        if (tit != state.tracked.end() && tit->second.haveDescriptor)
        {
            emissions.push_back(MakeParentUpdate(tit->second, &peer));
        }
    }
    state.pendingByPeerUuid.erase(pit);
}

// A peer (DataPublisher / RpcClient) was removed: forget its identity. Existing connection edges have
// already been reported; the connection's own removal (if any) reports the decremented count.
void OnPeerRemoved(ObserverState& state, const std::string& peerUuid)
{
    state.peersByUuid.erase(peerUuid);
}

// Handles a single internal discovery event, translating it into zero or more public emissions.
// Emissions are always dispatched outside the state lock so user code never runs while we hold it.
void HandleDiscoveryEvent(ObserverState& state, Discovery::ServiceDiscoveryEvent::Type type,
                          const SilKit::Core::ServiceDescriptor& descriptor)
{
    using EventType = Discovery::ServiceDiscoveryEvent::Type;

    // ---- Network links: track and fan-out isSimulated updates to affected bus controllers ----
    if (descriptor.GetServiceType() == SilKit::Core::ServiceType::Link)
    {
        const NetworkKey networkKey{descriptor.GetNetworkName(), descriptor.GetNetworkType()};
        std::vector<PendingEmission> emissions;
        {
            std::lock_guard<std::mutex> lock{state.mutex};
            if (type == EventType::ServiceCreated)
            {
                state.activeLinks[networkKey] = descriptor.GetParticipantName();
                const auto cit = state.controllersByNetwork.find(networkKey);
                if (cit != state.controllersByNetwork.end())
                {
                    for (const auto& controllerKey : cit->second)
                    {
                        PendingEmission e;
                        e.type = SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceUpdated;
                        e.descriptor = state.busControllers.at(controllerKey);
                        e.isSimulated = true;
                        e.simulatingParticipantName = descriptor.GetParticipantName();
                        emissions.push_back(std::move(e));
                    }
                }
            }
            else if (type == EventType::ServiceRemoved)
            {
                state.activeLinks.erase(networkKey);
                const auto cit = state.controllersByNetwork.find(networkKey);
                if (cit != state.controllersByNetwork.end())
                {
                    for (const auto& controllerKey : cit->second)
                    {
                        PendingEmission e;
                        e.type = SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceUpdated;
                        e.descriptor = state.busControllers.at(controllerKey);
                        e.isSimulated = false;
                        emissions.push_back(std::move(e));
                    }
                }
            }
        }
        for (const auto& e : emissions)
        {
            EmitPending(state, e);
        }
        return;
    }

    std::string controllerType;
    descriptor.GetSupplementalDataItem(Discovery::controllerType, controllerType);

    // ---- Bus controllers: track for link fan-out; emit immediately with current isSimulated state ----
    if (controllerType == Discovery::controllerTypeCan || controllerType == Discovery::controllerTypeEthernet
        || controllerType == Discovery::controllerTypeFlexray || controllerType == Discovery::controllerTypeLin)
    {
        const ServiceKey key{descriptor.GetParticipantName(), descriptor.GetServiceId()};
        const NetworkKey networkKey{descriptor.GetNetworkName(), descriptor.GetNetworkType()};
        PendingEmission emission;
        emission.descriptor = descriptor;
        {
            std::lock_guard<std::mutex> lock{state.mutex};
            if (type == EventType::ServiceCreated)
            {
                state.busControllers[key] = descriptor;
                state.controllersByNetwork[networkKey].insert(key);
                emission.type = SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated;
                const auto linkIt = state.activeLinks.find(networkKey);
                if (linkIt != state.activeLinks.end())
                {
                    emission.isSimulated = true;
                    emission.simulatingParticipantName = linkIt->second;
                }
            }
            else if (type == EventType::ServiceRemoved)
            {
                state.busControllers.erase(key);
                auto& ctrlSet = state.controllersByNetwork[networkKey];
                ctrlSet.erase(key);
                if (ctrlSet.empty())
                {
                    state.controllersByNetwork.erase(networkKey);
                }
                emission.type = SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceRemoved;
            }
        }
        EmitPending(state, emission);
        return;
    }

    // ---- DataPublisher: a user-facing service AND a connection peer. Emit its own event first, then
    //      reveal it on any subscriber connection recorded before this publisher was known. ----
    if (controllerType == Discovery::controllerTypeDataPublisher)
    {
        std::vector<PendingEmission> reveals;
        {
            std::lock_guard<std::mutex> lock{state.mutex};
            if (type == EventType::ServiceCreated)
            {
                OnPeerCreated(state, descriptor.GetNetworkName(),
                              PeerInfo{descriptor.GetParticipantName(), descriptor.GetServiceName()}, reveals);
            }
            else if (type == EventType::ServiceRemoved)
            {
                OnPeerRemoved(state, descriptor.GetNetworkName());
            }
        }
        PendingEmission own;
        own.type = ToC(type);
        own.descriptor = descriptor;
        EmitPending(state, own);
        for (const auto& e : reveals)
        {
            EmitPending(state, e);
        }
        return;
    }

    // ---- DataSubscriber: a tracked parent. OnParentCreated emits ServiceCreated and reveals any
    //      already-recorded connections; removal drops all of its bookkeeping. ----
    if (controllerType == Discovery::controllerTypeDataSubscriber)
    {
        const ServiceKey key{descriptor.GetParticipantName(), descriptor.GetServiceId()};
        std::vector<PendingEmission> emissions;
        {
            std::lock_guard<std::mutex> lock{state.mutex};
            if (type == EventType::ServiceCreated)
            {
                OnParentCreated(state, key, descriptor, emissions);
            }
            else if (type == EventType::ServiceRemoved)
            {
                OnParentRemoved(state, key);
                PendingEmission removed;
                removed.type = SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceRemoved;
                removed.descriptor = descriptor;
                emissions.push_back(std::move(removed));
            }
        }
        for (const auto& e : emissions)
        {
            EmitPending(state, e);
        }
        return;
    }

    // ---- DataSubscriberInternal: a confirmed pub/sub match; drives Service_Updated on the parent
    //      subscriber, with the publisher (peerUuid = networkName) as the peer. ----
    if (controllerType == Discovery::controllerTypeDataSubscriberInternal)
    {
        const ServiceKey internalKey{descriptor.GetParticipantName(), descriptor.GetServiceId()};
        std::string parentIdStr;
        if (!descriptor.GetSupplementalDataItem(Discovery::supplKeyDataSubscriberInternalParentServiceID, parentIdStr))
        {
            return;
        }
        SilKit::Core::EndpointId parentServiceId{0};
        try
        {
            parentServiceId = static_cast<SilKit::Core::EndpointId>(std::stoull(parentIdStr));
        }
        catch (...)
        {
            return;
        }
        const ServiceKey parentKey{descriptor.GetParticipantName(), parentServiceId};
        const std::string publisherUuid = descriptor.GetNetworkName();

        std::vector<PendingEmission> emissions;
        {
            std::lock_guard<std::mutex> lock{state.mutex};
            if (type == EventType::ServiceCreated)
            {
                OnConnectionCreated(state, internalKey, parentKey, publisherUuid, emissions);
            }
            else if (type == EventType::ServiceRemoved)
            {
                OnConnectionRemoved(state, internalKey, emissions);
            }
        }
        for (const auto& e : emissions)
        {
            EmitPending(state, e);
        }
        return;
    }

    // ---- RpcClient: a user-facing service AND a connection peer. Emit its own event first, then
    //      reveal it on any server connection recorded before this client was known. ----
    if (controllerType == Discovery::controllerTypeRpcClient)
    {
        std::vector<PendingEmission> reveals;
        {
            std::lock_guard<std::mutex> lock{state.mutex};
            if (type == EventType::ServiceCreated)
            {
                OnPeerCreated(state, descriptor.GetNetworkName(),
                              PeerInfo{descriptor.GetParticipantName(), descriptor.GetServiceName()}, reveals);
            }
            else if (type == EventType::ServiceRemoved)
            {
                OnPeerRemoved(state, descriptor.GetNetworkName());
            }
        }
        PendingEmission own;
        own.type = ToC(type);
        own.descriptor = descriptor;
        EmitPending(state, own);
        for (const auto& e : reveals)
        {
            EmitPending(state, e);
        }
        return;
    }

    // ---- RpcServer: a tracked parent. OnParentCreated emits ServiceCreated and reveals any
    //      already-recorded connections; removal drops all of its bookkeeping. ----
    if (controllerType == Discovery::controllerTypeRpcServer)
    {
        const ServiceKey key{descriptor.GetParticipantName(), descriptor.GetServiceId()};
        std::vector<PendingEmission> emissions;
        {
            std::lock_guard<std::mutex> lock{state.mutex};
            if (type == EventType::ServiceCreated)
            {
                OnParentCreated(state, key, descriptor, emissions);
            }
            else if (type == EventType::ServiceRemoved)
            {
                OnParentRemoved(state, key);
                PendingEmission removed;
                removed.type = SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceRemoved;
                removed.descriptor = descriptor;
                emissions.push_back(std::move(removed));
            }
        }
        for (const auto& e : emissions)
        {
            EmitPending(state, e);
        }
        return;
    }

    // ---- RpcServerInternal: a confirmed RPC match; emit Service_Updated on the parent server ----
    if (controllerType == Discovery::controllerTypeRpcServerInternal)
    {
        const ServiceKey internalKey{descriptor.GetParticipantName(), descriptor.GetServiceId()};
        std::string parentIdStr;
        if (!descriptor.GetSupplementalDataItem(Discovery::supplKeyRpcServerInternalParentServiceID, parentIdStr))
        {
            return;
        }
        SilKit::Core::EndpointId parentServiceId{0};
        try
        {
            parentServiceId = static_cast<SilKit::Core::EndpointId>(std::stoull(parentIdStr));
        }
        catch (...)
        {
            return;
        }
        const ServiceKey parentKey{descriptor.GetParticipantName(), parentServiceId};
        std::string clientUuid;
        descriptor.GetSupplementalDataItem(Discovery::supplKeyRpcServerInternalClientUUID, clientUuid);

        std::vector<PendingEmission> emissions;
        {
            std::lock_guard<std::mutex> lock{state.mutex};
            if (type == EventType::ServiceCreated)
            {
                OnConnectionCreated(state, internalKey, parentKey, clientUuid, emissions);
            }
            else if (type == EventType::ServiceRemoved)
            {
                OnConnectionRemoved(state, internalKey, emissions);
            }
        }
        for (const auto& e : emissions)
        {
            EmitPending(state, e);
        }
        return;
    }

    // Everything else (infrastructure, system services): forwarded as-is; ClassifyAndFill suppresses them.
    PendingEmission e;
    e.type = ToC(type);
    e.descriptor = descriptor;
    EmitPending(state, e);
}

} // namespace


SilKit_ReturnCode SilKitCALL SilKit_Experimental_ServiceDiscovery_Create(
    SilKit_Experimental_ServiceDiscovery** outServiceDiscovery, SilKit_Participant* participant)
try
{
    ASSERT_VALID_OUT_PARAMETER(outServiceDiscovery);
    ASSERT_VALID_POINTER_PARAMETER(participant);

    auto* serviceDiscovery = GetServiceDiscovery(participant);
    *outServiceDiscovery = reinterpret_cast<SilKit_Experimental_ServiceDiscovery*>(serviceDiscovery);

    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_Experimental_ServiceDiscovery_SetServiceDiscoveryHandler(
    SilKit_Experimental_ServiceDiscovery* serviceDiscovery, void* context,
    SilKit_Experimental_ServiceDiscoveryHandler_t handler)
try
{
    ASSERT_VALID_POINTER_PARAMETER(serviceDiscovery);
    ASSERT_VALID_HANDLER_PARAMETER(handler);

    auto* cppServiceDiscovery = reinterpret_cast<Discovery::IServiceDiscovery*>(serviceDiscovery);

    auto state = std::make_shared<ObserverState>();
    state->handler = handler;
    state->context = context;

    cppServiceDiscovery->RegisterServiceDiscoveryHandler(
        [state](Discovery::ServiceDiscoveryEvent::Type type,
                const SilKit::Core::ServiceDescriptor& serviceDescriptor) {
        // Invoked by the internal service discovery with its lock held, serialized, on an unspecified
        // thread (an IO worker for remote events, or the caller's thread for locally created/removed
        // services). Exceptions must never propagate into SIL Kit internals.
        try
        {
            HandleDiscoveryEvent(*state, type, serviceDescriptor);
        }
        catch (...)
        {
        }
    });

    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS
