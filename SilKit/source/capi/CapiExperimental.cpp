// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
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

// Tracks a user-facing service (DataSubscriber or RpcServer) for connection counting.
struct TrackedEntry
{
    SilKit::Core::ServiceDescriptor descriptor;
    bool haveDescriptor{false};
    std::size_t connections{0};
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
    // internal-connection key -> parent (subscriber/server) key
    std::map<ServiceKey, ServiceKey> connectionParent;

    // DataPublisher UUID (= networkName of DataPublisher) -> peer info, for resolving pub/sub connections.
    std::map<std::string, PeerInfo> publishersByUuid;
    // RpcClient UUID (= networkName of RpcClient) -> peer info, for resolving RPC connections.
    std::map<std::string, PeerInfo> rpcClientsByUuid;

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

    // ---- DataPublisher: track UUID for connection peer resolution; emit immediately ----
    if (controllerType == Discovery::controllerTypeDataPublisher)
    {
        {
            std::lock_guard<std::mutex> lock{state.mutex};
            if (type == EventType::ServiceCreated)
            {
                state.publishersByUuid[descriptor.GetNetworkName()] = {descriptor.GetParticipantName(),
                                                                       descriptor.GetServiceName()};
            }
            else if (type == EventType::ServiceRemoved)
            {
                state.publishersByUuid.erase(descriptor.GetNetworkName());
            }
        }
        PendingEmission e;
        e.type = ToC(type);
        e.descriptor = descriptor;
        EmitPending(state, e);
        return;
    }

    // ---- DataSubscriber: emit immediately; tracked for connection counting via DataSubscriberInternal ----
    if (controllerType == Discovery::controllerTypeDataSubscriber)
    {
        const ServiceKey key{descriptor.GetParticipantName(), descriptor.GetServiceId()};
        PendingEmission emission;
        emission.descriptor = descriptor;
        {
            std::lock_guard<std::mutex> lock{state.mutex};
            if (type == EventType::ServiceCreated)
            {
                auto& entry = state.tracked[key];
                entry.descriptor = descriptor;
                entry.haveDescriptor = true;
                emission.type = SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated;
                emission.numberOfConnections = static_cast<uint32_t>(entry.connections);
            }
            else if (type == EventType::ServiceRemoved)
            {
                state.tracked.erase(key);
                for (auto cit = state.connectionParent.begin(); cit != state.connectionParent.end();)
                {
                    cit = (cit->second == key) ? state.connectionParent.erase(cit) : std::next(cit);
                }
                emission.type = SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceRemoved;
            }
        }
        EmitPending(state, emission);
        return;
    }

    // ---- DataSubscriberInternal: a confirmed pub/sub match; emit Service_Updated on the parent subscriber ----
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
                if (state.connectionParent.count(internalKey) == 0)
                {
                    state.connectionParent[internalKey] = parentKey;
                    auto& entry = state.tracked[parentKey];
                    ++entry.connections;
                    if (entry.haveDescriptor)
                    {
                        PendingEmission e;
                        e.type = SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceUpdated;
                        e.descriptor = entry.descriptor;
                        e.numberOfConnections = static_cast<uint32_t>(entry.connections);
                        const auto pubIt = state.publishersByUuid.find(publisherUuid);
                        if (pubIt != state.publishersByUuid.end())
                        {
                            e.connectedParticipantName = pubIt->second.participantName;
                            e.connectedServiceName = pubIt->second.serviceName;
                        }
                        emissions.push_back(std::move(e));
                    }
                }
            }
            else if (type == EventType::ServiceRemoved)
            {
                const auto cit = state.connectionParent.find(internalKey);
                if (cit != state.connectionParent.end())
                {
                    const auto storedParentKey = cit->second;
                    state.connectionParent.erase(cit);
                    const auto sit = state.tracked.find(storedParentKey);
                    if (sit != state.tracked.end() && sit->second.connections > 0)
                    {
                        --sit->second.connections;
                        if (sit->second.haveDescriptor)
                        {
                            PendingEmission e;
                            e.type = SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceUpdated;
                            e.descriptor = sit->second.descriptor;
                            e.numberOfConnections = static_cast<uint32_t>(sit->second.connections);
                            const auto pubIt = state.publishersByUuid.find(publisherUuid);
                            if (pubIt != state.publishersByUuid.end())
                            {
                                e.connectedParticipantName = pubIt->second.participantName;
                                e.connectedServiceName = pubIt->second.serviceName;
                            }
                            emissions.push_back(std::move(e));
                        }
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

    // ---- RpcClient: track UUID for RPC connection peer resolution; emit immediately ----
    if (controllerType == Discovery::controllerTypeRpcClient)
    {
        {
            std::lock_guard<std::mutex> lock{state.mutex};
            if (type == EventType::ServiceCreated)
            {
                state.rpcClientsByUuid[descriptor.GetNetworkName()] = {descriptor.GetParticipantName(),
                                                                       descriptor.GetServiceName()};
            }
            else if (type == EventType::ServiceRemoved)
            {
                state.rpcClientsByUuid.erase(descriptor.GetNetworkName());
            }
        }
        PendingEmission e;
        e.type = ToC(type);
        e.descriptor = descriptor;
        EmitPending(state, e);
        return;
    }

    // ---- RpcServer: emit immediately; tracked for connection counting via RpcServerInternal ----
    if (controllerType == Discovery::controllerTypeRpcServer)
    {
        const ServiceKey key{descriptor.GetParticipantName(), descriptor.GetServiceId()};
        PendingEmission emission;
        emission.descriptor = descriptor;
        {
            std::lock_guard<std::mutex> lock{state.mutex};
            if (type == EventType::ServiceCreated)
            {
                auto& entry = state.tracked[key];
                entry.descriptor = descriptor;
                entry.haveDescriptor = true;
                emission.type = SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated;
                emission.numberOfConnections = static_cast<uint32_t>(entry.connections);
            }
            else if (type == EventType::ServiceRemoved)
            {
                state.tracked.erase(key);
                for (auto cit = state.connectionParent.begin(); cit != state.connectionParent.end();)
                {
                    cit = (cit->second == key) ? state.connectionParent.erase(cit) : std::next(cit);
                }
                emission.type = SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceRemoved;
            }
        }
        EmitPending(state, emission);
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
                if (state.connectionParent.count(internalKey) == 0)
                {
                    state.connectionParent[internalKey] = parentKey;
                    auto& entry = state.tracked[parentKey];
                    ++entry.connections;
                    if (entry.haveDescriptor)
                    {
                        PendingEmission e;
                        e.type = SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceUpdated;
                        e.descriptor = entry.descriptor;
                        e.numberOfConnections = static_cast<uint32_t>(entry.connections);
                        const auto clientIt = state.rpcClientsByUuid.find(clientUuid);
                        if (clientIt != state.rpcClientsByUuid.end())
                        {
                            e.connectedParticipantName = clientIt->second.participantName;
                            e.connectedServiceName = clientIt->second.serviceName;
                        }
                        emissions.push_back(std::move(e));
                    }
                }
            }
            else if (type == EventType::ServiceRemoved)
            {
                const auto cit = state.connectionParent.find(internalKey);
                if (cit != state.connectionParent.end())
                {
                    const auto storedParentKey = cit->second;
                    state.connectionParent.erase(cit);
                    const auto sit = state.tracked.find(storedParentKey);
                    if (sit != state.tracked.end() && sit->second.connections > 0)
                    {
                        --sit->second.connections;
                        if (sit->second.haveDescriptor)
                        {
                            PendingEmission e;
                            e.type = SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceUpdated;
                            e.descriptor = sit->second.descriptor;
                            e.numberOfConnections = static_cast<uint32_t>(sit->second.connections);
                            const auto clientIt = state.rpcClientsByUuid.find(clientUuid);
                            if (clientIt != state.rpcClientsByUuid.end())
                            {
                                e.connectedParticipantName = clientIt->second.participantName;
                                e.connectedServiceName = clientIt->second.serviceName;
                            }
                            emissions.push_back(std::move(e));
                        }
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
        // This runs on the SIL Kit IO worker thread. Exceptions must never propagate into SIL Kit internals.
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
