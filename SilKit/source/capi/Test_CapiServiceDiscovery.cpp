// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <string>
#include <vector>
#include <tuple>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "silkit/capi/SilKit.h"
#include "silkit/services/datatypes.hpp"

#include "core/mock/participant/MockParticipant.hpp"
#include "core/internal/ServiceDescriptor.hpp"
#include "core/internal/ServiceConfigKeys.hpp"
#include "config/YamlParser.hpp"

namespace {

using SilKit::Core::Tests::DummyParticipant;
using SilKit::Core::ServiceDescriptor;
using SilKit::Core::ServiceType;
using ServiceDiscoveryEvent = SilKit::Core::Discovery::ServiceDiscoveryEvent;
namespace Discovery = SilKit::Core::Discovery;

struct CapturedLabel
{
    std::string key;
    std::string value;
    SilKit_LabelKind kind;
};

// Captures, inside the handler invocation, everything the public struct carries. Copying into owned storage here also
// verifies that the struct and all its borrowed pointers are valid for the duration of the callback.
struct CallbackData
{
    int callCount{0};
    SilKit_Experimental_ServiceDiscoveryEvent_Type lastType{
        SilKit_Experimental_ServiceDiscoveryEvent_Type_Invalid};
    SilKit_Experimental_ServiceKind serviceKind{SilKit_Experimental_ServiceKind_Undefined};
    std::string participantName;
    std::string serviceName;
    std::string primaryIdentifier;
    std::string mediaType;
    std::vector<CapturedLabel> labels;
};

void SilKitCALL CapturingHandler(void* context, SilKit_Experimental_ServiceDiscoveryEvent_Type type,
                                 const SilKit_Experimental_ServiceDescriptor* serviceDescriptor)
{
    auto* data = static_cast<CallbackData*>(context);
    data->callCount += 1;
    data->lastType = type;
    data->serviceKind = serviceDescriptor->serviceKind;
    data->participantName = serviceDescriptor->participantName;
    data->serviceName = serviceDescriptor->serviceName;
    data->primaryIdentifier = serviceDescriptor->primaryIdentifier;
    data->mediaType = serviceDescriptor->mediaType;
    data->labels.clear();
    for (size_t i = 0; i < serviceDescriptor->labelList.numLabels; ++i)
    {
        const auto& label = serviceDescriptor->labelList.labels[i];
        data->labels.push_back({label.key, label.value, label.kind});
    }
}

void SilKitCALL NoopHandler(void* /*context*/, SilKit_Experimental_ServiceDiscoveryEvent_Type /*type*/,
                            const SilKit_Experimental_ServiceDescriptor* /*serviceDescriptor*/)
{
}

auto SerializeLabels(const std::vector<SilKit::Services::MatchingLabel>& labels) -> std::string
{
    return SilKit::Config::Serialize(labels);
}

class Test_CapiServiceDiscovery : public testing::Test
{
public:
    DummyParticipant mockParticipant;

    static ServiceDescriptor MakeController(const std::string& controllerType, const std::string& networkName,
                                            const std::string& serviceName)
    {
        ServiceDescriptor descriptor;
        descriptor.SetParticipantNameAndComputeId("ParticipantA");
        descriptor.SetServiceName(serviceName);
        descriptor.SetNetworkName(networkName);
        descriptor.SetServiceType(ServiceType::Controller);
        descriptor.SetSupplementalDataItem(Discovery::controllerType, controllerType);
        return descriptor;
    }

    // Registers the capturing C handler and returns the internal handler the C API installed on the service discovery.
    SilKit::Core::Discovery::ServiceDiscoveryHandler InstallHandler(CallbackData& data)
    {
        SilKit::Core::Discovery::ServiceDiscoveryHandler capturedHandler;
        EXPECT_CALL(mockParticipant.mockServiceDiscovery, RegisterServiceDiscoveryHandler(testing::_))
            .WillOnce(testing::SaveArg<0>(&capturedHandler));

        SilKit_Experimental_ServiceDiscovery* serviceDiscovery = nullptr;
        EXPECT_EQ(SilKit_Experimental_ServiceDiscovery_Create(&serviceDiscovery, (SilKit_Participant*)&mockParticipant),
                  SilKit_ReturnCode_SUCCESS);
        EXPECT_EQ(SilKit_Experimental_ServiceDiscovery_SetServiceDiscoveryHandler(serviceDiscovery, &data,
                                                                                  &CapturingHandler),
                  SilKit_ReturnCode_SUCCESS);
        EXPECT_TRUE(static_cast<bool>(capturedHandler));
        return capturedHandler;
    }
};

TEST_F(Test_CapiServiceDiscovery, create_returns_service_discovery)
{
    SilKit_Experimental_ServiceDiscovery* serviceDiscovery = nullptr;
    EXPECT_EQ(SilKit_Experimental_ServiceDiscovery_Create(&serviceDiscovery, (SilKit_Participant*)&mockParticipant),
              SilKit_ReturnCode_SUCCESS);
    EXPECT_NE(serviceDiscovery, nullptr);
}

TEST_F(Test_CapiServiceDiscovery, create_bad_parameters)
{
    SilKit_Experimental_ServiceDiscovery* serviceDiscovery = nullptr;
    EXPECT_EQ(SilKit_Experimental_ServiceDiscovery_Create(nullptr, (SilKit_Participant*)&mockParticipant),
              SilKit_ReturnCode_BADPARAMETER);
    EXPECT_EQ(SilKit_Experimental_ServiceDiscovery_Create(&serviceDiscovery, nullptr), SilKit_ReturnCode_BADPARAMETER);
}

TEST_F(Test_CapiServiceDiscovery, set_handler_bad_parameters)
{
    SilKit_Experimental_ServiceDiscovery* serviceDiscovery = nullptr;
    ASSERT_EQ(SilKit_Experimental_ServiceDiscovery_Create(&serviceDiscovery, (SilKit_Participant*)&mockParticipant),
              SilKit_ReturnCode_SUCCESS);
    EXPECT_EQ(SilKit_Experimental_ServiceDiscovery_SetServiceDiscoveryHandler(nullptr, nullptr, &NoopHandler),
              SilKit_ReturnCode_BADPARAMETER);
    EXPECT_EQ(SilKit_Experimental_ServiceDiscovery_SetServiceDiscoveryHandler(serviceDiscovery, nullptr, nullptr),
              SilKit_ReturnCode_BADPARAMETER);
}

TEST_F(Test_CapiServiceDiscovery, reports_can_controller)
{
    CallbackData data;
    auto handler = InstallHandler(data);

    handler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeController(Discovery::controllerTypeCan, "CAN1", "Can1"));

    EXPECT_EQ(data.callCount, 1);
    EXPECT_EQ(data.lastType, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated);
    EXPECT_EQ(data.serviceKind, SilKit_Experimental_ServiceKind_CanController);
    EXPECT_EQ(data.participantName, "ParticipantA");
    EXPECT_EQ(data.serviceName, "Can1");
    EXPECT_EQ(data.primaryIdentifier, "CAN1"); // bus controller: identifier is the network name
    EXPECT_EQ(data.mediaType, "");
    EXPECT_TRUE(data.labels.empty());
}

TEST_F(Test_CapiServiceDiscovery, reports_data_publisher_with_decoded_metadata)
{
    CallbackData data;
    auto handler = InstallHandler(data);

    auto descriptor = MakeController(Discovery::controllerTypeDataPublisher, "pub-uuid-1234", "MyPublisher");
    descriptor.SetSupplementalDataItem(Discovery::supplKeyDataPublisherTopic, "TopicA");
    descriptor.SetSupplementalDataItem(Discovery::supplKeyDataPublisherMediaType, "application/json");
    descriptor.SetSupplementalDataItem(
        Discovery::supplKeyDataPublisherPubLabels,
        SerializeLabels({{"kA", "vA", SilKit::Services::MatchingLabel::Kind::Mandatory},
                         {"kB", "vB", SilKit::Services::MatchingLabel::Kind::Optional}}));

    handler(ServiceDiscoveryEvent::Type::ServiceCreated, descriptor);

    EXPECT_EQ(data.callCount, 1);
    EXPECT_EQ(data.serviceKind, SilKit_Experimental_ServiceKind_DataPublisher);
    EXPECT_EQ(data.primaryIdentifier, "TopicA"); // pub/sub: identifier is the topic
    EXPECT_EQ(data.mediaType, "application/json");
    ASSERT_EQ(data.labels.size(), 2u);
    EXPECT_EQ(data.labels[0].key, "kA");
    EXPECT_EQ(data.labels[0].value, "vA");
    EXPECT_EQ(data.labels[0].kind, SilKit_LabelKind_Mandatory);
    EXPECT_EQ(data.labels[1].key, "kB");
    EXPECT_EQ(data.labels[1].kind, SilKit_LabelKind_Optional);
}

TEST_F(Test_CapiServiceDiscovery, reports_rpc_client_with_function_name)
{
    CallbackData data;
    auto handler = InstallHandler(data);

    auto descriptor = MakeController(Discovery::controllerTypeRpcClient, "rpc-uuid-9", "MyClient");
    descriptor.SetSupplementalDataItem(Discovery::supplKeyRpcClientFunctionName, "Add");
    descriptor.SetSupplementalDataItem(Discovery::supplKeyRpcClientMediaType, "application/octet-stream");

    handler(ServiceDiscoveryEvent::Type::ServiceCreated, descriptor);

    EXPECT_EQ(data.serviceKind, SilKit_Experimental_ServiceKind_RpcClient);
    EXPECT_EQ(data.primaryIdentifier, "Add"); // rpc: identifier is the function name
    EXPECT_EQ(data.mediaType, "application/octet-stream");
}

TEST_F(Test_CapiServiceDiscovery, reports_network_link)
{
    CallbackData data;
    auto handler = InstallHandler(data);

    ServiceDescriptor descriptor;
    descriptor.SetParticipantNameAndComputeId("ParticipantA");
    descriptor.SetServiceName("LinkService");
    descriptor.SetNetworkName("CAN1");
    descriptor.SetServiceType(ServiceType::Link);

    handler(ServiceDiscoveryEvent::Type::ServiceCreated, descriptor);

    EXPECT_EQ(data.callCount, 1);
    EXPECT_EQ(data.serviceKind, SilKit_Experimental_ServiceKind_NetworkLink);
    EXPECT_EQ(data.primaryIdentifier, "CAN1");
}

TEST_F(Test_CapiServiceDiscovery, reports_removal)
{
    CallbackData data;
    auto handler = InstallHandler(data);

    handler(ServiceDiscoveryEvent::Type::ServiceRemoved, MakeController(Discovery::controllerTypeLin, "LIN1", "Lin1"));

    EXPECT_EQ(data.callCount, 1);
    EXPECT_EQ(data.lastType, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceRemoved);
    EXPECT_EQ(data.serviceKind, SilKit_Experimental_ServiceKind_LinController);
}

TEST_F(Test_CapiServiceDiscovery, filters_infrastructure_and_internal_services)
{
    CallbackData data;
    auto handler = InstallHandler(data);

    // Infrastructure controller (SystemMonitor) is not user-facing.
    handler(ServiceDiscoveryEvent::Type::ServiceCreated,
            MakeController(Discovery::controllerTypeSystemMonitor, "default", "SystemMonitor"));
    // Internal pub/sub helper is not user-facing.
    handler(ServiceDiscoveryEvent::Type::ServiceCreated,
            MakeController(Discovery::controllerTypeDataSubscriberInternal, "sub-uuid", "SubInternal"));

    // A controller without a controller.type supplemental entry is skipped, too.
    ServiceDescriptor noControllerType;
    noControllerType.SetParticipantNameAndComputeId("ParticipantA");
    noControllerType.SetServiceName("Mystery");
    noControllerType.SetServiceType(ServiceType::Controller);
    handler(ServiceDiscoveryEvent::Type::ServiceCreated, noControllerType);

    EXPECT_EQ(data.callCount, 0);
}

TEST_F(Test_CapiServiceDiscovery, malformed_labels_are_swallowed)
{
    CallbackData data;
    auto handler = InstallHandler(data);

    auto descriptor = MakeController(Discovery::controllerTypeDataPublisher, "pub-uuid", "MyPublisher");
    descriptor.SetSupplementalDataItem(Discovery::supplKeyDataPublisherTopic, "TopicA");
    descriptor.SetSupplementalDataItem(Discovery::supplKeyDataPublisherPubLabels, "{ this is : not [ valid");

    // Must not throw across the FFI boundary; the service is still reported.
    EXPECT_NO_THROW(handler(ServiceDiscoveryEvent::Type::ServiceCreated, descriptor));
    EXPECT_EQ(data.callCount, 1);
    EXPECT_EQ(data.serviceKind, SilKit_Experimental_ServiceKind_DataPublisher);
    EXPECT_EQ(data.primaryIdentifier, "TopicA");
}

// --------------------------------------------------------------------------------------------------
// Connection-gated DataSubscriber synthesis: a user-facing DataSubscriber is reported only while it
// has at least one confirmed connection (a DataSubscriberInternal announcement whose parent-service
// id points back at it). Internal services and the transport UUID are never exposed.
// --------------------------------------------------------------------------------------------------

namespace {
// User-facing DataSubscriber with an explicit service id and topic (topic drives primaryIdentifier).
auto MakeUserSubscriber(SilKit::Core::EndpointId serviceId, const std::string& topic) -> ServiceDescriptor
{
    ServiceDescriptor descriptor;
    descriptor.SetParticipantNameAndComputeId("SubParticipant");
    descriptor.SetServiceName("Sub");
    descriptor.SetNetworkName("default");
    descriptor.SetServiceType(ServiceType::Controller);
    descriptor.SetServiceId(serviceId);
    descriptor.SetSupplementalDataItem(Discovery::controllerType, Discovery::controllerTypeDataSubscriber);
    descriptor.SetSupplementalDataItem(Discovery::supplKeyDataSubscriberTopic, topic);
    return descriptor;
}

// DataSubscriberInternal (a confirmed connection) whose parent is the user-facing subscriber above.
auto MakeInternalConnection(SilKit::Core::EndpointId serviceId, SilKit::Core::EndpointId parentServiceId)
    -> ServiceDescriptor
{
    ServiceDescriptor descriptor;
    descriptor.SetParticipantNameAndComputeId("SubParticipant");
    descriptor.SetServiceName("SubInternal");
    descriptor.SetNetworkName("pub-uuid");
    descriptor.SetServiceType(ServiceType::Controller);
    descriptor.SetServiceId(serviceId);
    descriptor.SetSupplementalDataItem(Discovery::controllerType, Discovery::controllerTypeDataSubscriberInternal);
    descriptor.SetSupplementalDataItem(Discovery::supplKeyDataSubscriberInternalParentServiceID,
                                       std::to_string(parentServiceId));
    return descriptor;
}
} // namespace

TEST_F(Test_CapiServiceDiscovery, subscriber_without_connection_is_not_reported)
{
    CallbackData data;
    auto handler = InstallHandler(data);

    handler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeUserSubscriber(100, "T"));

    EXPECT_EQ(data.callCount, 0);
}

TEST_F(Test_CapiServiceDiscovery, subscriber_reported_when_connection_appears)
{
    CallbackData data;
    auto handler = InstallHandler(data);

    handler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeUserSubscriber(100, "T"));
    handler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeInternalConnection(200, 100));

    EXPECT_EQ(data.callCount, 1);
    EXPECT_EQ(data.lastType, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated);
    EXPECT_EQ(data.serviceKind, SilKit_Experimental_ServiceKind_DataSubscriber);
    EXPECT_EQ(data.primaryIdentifier, "T");
}

TEST_F(Test_CapiServiceDiscovery, subscriber_reported_when_connection_arrives_before_subscriber)
{
    CallbackData data;
    auto handler = InstallHandler(data);

    // Reversed order: the connection is discovered before the user-facing subscriber.
    handler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeInternalConnection(200, 100));
    EXPECT_EQ(data.callCount, 0);

    handler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeUserSubscriber(100, "T"));
    EXPECT_EQ(data.callCount, 1);
    EXPECT_EQ(data.lastType, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated);
    EXPECT_EQ(data.serviceKind, SilKit_Experimental_ServiceKind_DataSubscriber);
}

TEST_F(Test_CapiServiceDiscovery, subscriber_removed_when_connection_removed)
{
    CallbackData data;
    auto handler = InstallHandler(data);

    handler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeUserSubscriber(100, "T"));
    handler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeInternalConnection(200, 100));
    ASSERT_EQ(data.callCount, 1);

    handler(ServiceDiscoveryEvent::Type::ServiceRemoved, MakeInternalConnection(200, 100));

    EXPECT_EQ(data.callCount, 2);
    EXPECT_EQ(data.lastType, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceRemoved);
    EXPECT_EQ(data.serviceKind, SilKit_Experimental_ServiceKind_DataSubscriber);
}

TEST_F(Test_CapiServiceDiscovery, subscriber_reported_once_for_multiple_connections)
{
    CallbackData data;
    auto handler = InstallHandler(data);

    handler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeUserSubscriber(100, "T"));

    // Two publishers => two internal connections for the same subscriber: only one ServiceCreated.
    handler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeInternalConnection(200, 100));
    handler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeInternalConnection(201, 100));
    EXPECT_EQ(data.callCount, 1);

    // Removing one connection keeps the subscriber connected (no event).
    handler(ServiceDiscoveryEvent::Type::ServiceRemoved, MakeInternalConnection(200, 100));
    EXPECT_EQ(data.callCount, 1);

    // Removing the last connection reports the subscriber removed.
    handler(ServiceDiscoveryEvent::Type::ServiceRemoved, MakeInternalConnection(201, 100));
    EXPECT_EQ(data.callCount, 2);
    EXPECT_EQ(data.lastType, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceRemoved);
}

} // namespace
