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
// (infrastructure / internal services), in which case the handler must not be invoked.
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

    // Network links carry no controller type and are reported as-is.
    if (serviceDescriptor.GetServiceType() == SilKit::Core::ServiceType::Link)
    {
        out.serviceKind = SilKit_Experimental_ServiceKind_NetworkLink;
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

// Identifies a service across the simulation: (participant name, service id).
using ServiceKey = std::pair<std::string, SilKit::Core::EndpointId>;

struct SubscriberEntry
{
    SilKit::Core::ServiceDescriptor descriptor;
    bool haveDescriptor{false};
    std::size_t connections{0};
    bool announced{false};
};

// A synthesized event, queued while the state lock is held and dispatched after it is released.
struct PendingEmission
{
    SilKit_Experimental_ServiceDiscoveryEvent_Type type;
    SilKit::Core::ServiceDescriptor descriptor;
};

// Per-observer bookkeeping. Lives as long as the participant's service discovery, because it is
// captured by the registered handler (which is never unregistered).
//
// Semantics: a user-facing DataSubscriber is reported to the observer only while it has at least one
// confirmed connection to a publisher. SIL Kit creates exactly one internal DataSubscriberInternal
// service per matched publisher - its existence IS the confirmed match - carrying the parent
// DataSubscriber's service id. We ref-count these connections and synthesize a ServiceCreated when
// the first connection appears and a ServiceRemoved when the last one disappears. Internal services
// and the transport UUID are never exposed. All other kinds (publishers, RPC, bus, links) are
// reported on creation, unchanged.
//
// (RPC has an analogous RpcServerInternal edge but is intentionally out of scope here: RPC
// client/server are still reported on creation.)
struct ObserverState
{
    SilKit_Experimental_ServiceDiscoveryHandler_t handler{};
    void* context{nullptr};

    std::mutex mutex;
    std::map<ServiceKey, SubscriberEntry> subscribers;
    std::map<ServiceKey, ServiceKey> connectionParent; // internal-subscriber id -> parent subscriber id
};

void Emit(ObserverState& state, SilKit_Experimental_ServiceDiscoveryEvent_Type type,
          const SilKit::Core::ServiceDescriptor& descriptor)
{
    SilKit_Experimental_ServiceDescriptor reportedDescriptor{};
    LabelStorage storage;
    if (ClassifyAndFill(descriptor, reportedDescriptor, storage))
    {
        state.handler(state.context, type, &reportedDescriptor);
    }
}

// Handles a single internal discovery event. Subscriber-related events drive the connection
// bookkeeping and may synthesize user-facing DataSubscriber events; every other kind is forwarded
// as-is. Emissions are dispatched outside the lock so user code never runs while we hold it.
void HandleDiscoveryEvent(ObserverState& state, Discovery::ServiceDiscoveryEvent::Type type,
                          const SilKit::Core::ServiceDescriptor& descriptor)
{
    using EventType = Discovery::ServiceDiscoveryEvent::Type;

    std::string controllerType;
    descriptor.GetSupplementalDataItem(Discovery::controllerType, controllerType);

    if (controllerType == Discovery::controllerTypeDataSubscriber)
    {
        const ServiceKey key{descriptor.GetParticipantName(), descriptor.GetServiceId()};
        std::vector<PendingEmission> emissions;
        {
            std::lock_guard<std::mutex> lock{state.mutex};
            if (type == EventType::ServiceCreated)
            {
                auto& entry = state.subscribers[key];
                entry.descriptor = descriptor;
                entry.haveDescriptor = true;
                if (entry.connections > 0 && !entry.announced)
                {
                    entry.announced = true;
                    emissions.push_back(
                        {SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated, entry.descriptor});
                }
            }
            else if (type == EventType::ServiceRemoved)
            {
                const auto it = state.subscribers.find(key);
                if (it != state.subscribers.end())
                {
                    if (it->second.announced)
                    {
                        emissions.push_back(
                            {SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceRemoved, it->second.descriptor});
                    }
                    state.subscribers.erase(it);
                    // Drop any still-open connections that referenced this subscriber.
                    for (auto cit = state.connectionParent.begin(); cit != state.connectionParent.end();)
                    {
                        cit = (cit->second == key) ? state.connectionParent.erase(cit) : std::next(cit);
                    }
                }
            }
        }
        for (const auto& emission : emissions)
        {
            Emit(state, emission.type, emission.descriptor);
        }
        return;
    }

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

        std::vector<PendingEmission> emissions;
        {
            std::lock_guard<std::mutex> lock{state.mutex};
            if (type == EventType::ServiceCreated)
            {
                if (state.connectionParent.count(internalKey) == 0)
                {
                    state.connectionParent[internalKey] = parentKey;
                    auto& entry = state.subscribers[parentKey];
                    ++entry.connections;
                    if (entry.haveDescriptor && !entry.announced)
                    {
                        entry.announced = true;
                        emissions.push_back(
                            {SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated, entry.descriptor});
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
                    const auto sit = state.subscribers.find(storedParentKey);
                    if (sit != state.subscribers.end() && sit->second.connections > 0)
                    {
                        --sit->second.connections;
                        if (sit->second.connections == 0 && sit->second.announced)
                        {
                            sit->second.announced = false;
                            emissions.push_back(
                                {SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceRemoved, sit->second.descriptor});
                        }
                    }
                }
            }
        }
        for (const auto& emission : emissions)
        {
            Emit(state, emission.type, emission.descriptor);
        }
        return;
    }

    // Publishers, RPC clients/servers, bus controllers, network links, and anything else: reported on
    // creation/removal exactly as before.
    SilKit_Experimental_ServiceDescriptor reportedDescriptor{};
    LabelStorage storage;
    if (ClassifyAndFill(descriptor, reportedDescriptor, storage))
    {
        state.handler(state.context, ToC(type), &reportedDescriptor);
    }
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

    // The observer state is captured by (and thus owned by) the registered handler, which lives as
    // long as the participant's service discovery. It buffers pub/sub matches and synthesizes
    // connection-gated DataSubscriber events (see ObserverState / HandleDiscoveryEvent).
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
