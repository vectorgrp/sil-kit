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

#include <string>
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
    out.networkName = serviceDescriptor.GetNetworkName().c_str();
    out.networkOrTopic = serviceDescriptor.GetNetworkName().c_str();
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
            out.networkOrTopic = topic;
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
            out.networkOrTopic = topic;
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
            out.networkOrTopic = functionName;
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
            out.networkOrTopic = functionName;
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

    cppServiceDiscovery->RegisterServiceDiscoveryHandler(
        [handler, context](Discovery::ServiceDiscoveryEvent::Type type,
                           const SilKit::Core::ServiceDescriptor& serviceDescriptor) {
        // This runs on the SIL Kit IO worker thread. Exceptions must never propagate into SIL Kit internals.
        try
        {
            SilKit_Experimental_ServiceDescriptor cDescriptor;
            LabelStorage storage;
            if (!ClassifyAndFill(serviceDescriptor, cDescriptor, storage))
            {
                return;
            }
            handler(context, ToC(type), &cDescriptor);
        }
        catch (...)
        {
        }
    });

    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS
