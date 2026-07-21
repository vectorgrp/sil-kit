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

// Captures everything the public struct carries inside the handler invocation. Copying into owned
// storage also verifies that the struct and all its borrowed pointers are valid for the duration of
// the callback.
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
    std::string simulationName;
    uint32_t numberOfConnections{0};
    std::string connectedParticipantName;
    std::string connectedServiceName;
    bool isSimulated{false};
    std::string simulatingParticipantName;
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
    data->simulationName = serviceDescriptor->simulationName;
    data->numberOfConnections = serviceDescriptor->numberOfConnections;
    data->connectedParticipantName = serviceDescriptor->connectedParticipantName;
    data->connectedServiceName = serviceDescriptor->connectedServiceName;
    data->isSimulated = serviceDescriptor->isSimulated != SilKit_False;
    data->simulatingParticipantName = serviceDescriptor->simulatingParticipantName;
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
    EXPECT_EQ(data.numberOfConnections, 0u);
    EXPECT_FALSE(data.isSimulated);
    EXPECT_EQ(data.simulatingParticipantName, "");
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

TEST_F(Test_CapiServiceDiscovery, network_link_events_are_suppressed)
{
    // Network links are not reported directly at the public API level. Instead they drive
    // isSimulated / Service_Updated events on bus controllers (see is_simulated_* tests below).
    CallbackData data;
    auto handler = InstallHandler(data);

    ServiceDescriptor descriptor;
    descriptor.SetParticipantNameAndComputeId("ParticipantA");
    descriptor.SetServiceName("LinkService");
    descriptor.SetNetworkName("CAN1");
    descriptor.SetServiceType(ServiceType::Link);

    handler(ServiceDiscoveryEvent::Type::ServiceCreated, descriptor);

    EXPECT_EQ(data.callCount, 0);
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
    // DataSubscriberInternal with no parent id is silently dropped.
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
// DataSubscriber: reported immediately on creation; Service_Updated fires on each connection change.
// --------------------------------------------------------------------------------------------------

namespace {

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

// DataSubscriberInternal: a confirmed match between a publisher (identified by its UUID = networkName)
// and the parent DataSubscriber.
auto MakeInternalConnection(SilKit::Core::EndpointId serviceId, SilKit::Core::EndpointId parentServiceId,
                            const std::string& publisherUuid = "pub-uuid") -> ServiceDescriptor
{
    ServiceDescriptor descriptor;
    descriptor.SetParticipantNameAndComputeId("SubParticipant");
    descriptor.SetServiceName("SubInternal");
    descriptor.SetNetworkName(publisherUuid);
    descriptor.SetServiceType(ServiceType::Controller);
    descriptor.SetServiceId(serviceId);
    descriptor.SetSupplementalDataItem(Discovery::controllerType, Discovery::controllerTypeDataSubscriberInternal);
    descriptor.SetSupplementalDataItem(Discovery::supplKeyDataSubscriberInternalParentServiceID,
                                       std::to_string(parentServiceId));
    return descriptor;
}

auto MakeDataPublisher(const std::string& publisherUuid, const std::string& participantName,
                       const std::string& serviceName) -> ServiceDescriptor
{
    ServiceDescriptor descriptor;
    descriptor.SetParticipantNameAndComputeId(participantName);
    descriptor.SetServiceName(serviceName);
    descriptor.SetNetworkName(publisherUuid);
    descriptor.SetServiceType(ServiceType::Controller);
    descriptor.SetSupplementalDataItem(Discovery::controllerType, Discovery::controllerTypeDataPublisher);
    descriptor.SetSupplementalDataItem(Discovery::supplKeyDataPublisherTopic, "T");
    return descriptor;
}

auto MakeRpcServer(SilKit::Core::EndpointId serviceId, const std::string& functionName) -> ServiceDescriptor
{
    ServiceDescriptor descriptor;
    descriptor.SetParticipantNameAndComputeId("ServerParticipant");
    descriptor.SetServiceName("RpcSrv");
    descriptor.SetNetworkName("srv-net");
    descriptor.SetServiceType(ServiceType::Controller);
    descriptor.SetServiceId(serviceId);
    descriptor.SetSupplementalDataItem(Discovery::controllerType, Discovery::controllerTypeRpcServer);
    descriptor.SetSupplementalDataItem(Discovery::supplKeyRpcServerFunctionName, functionName);
    return descriptor;
}

auto MakeRpcClient(const std::string& clientUuid, const std::string& participantName,
                   const std::string& serviceName) -> ServiceDescriptor
{
    ServiceDescriptor descriptor;
    descriptor.SetParticipantNameAndComputeId(participantName);
    descriptor.SetServiceName(serviceName);
    descriptor.SetNetworkName(clientUuid);
    descriptor.SetServiceType(ServiceType::Controller);
    descriptor.SetSupplementalDataItem(Discovery::controllerType, Discovery::controllerTypeRpcClient);
    descriptor.SetSupplementalDataItem(Discovery::supplKeyRpcClientFunctionName, "Add");
    return descriptor;
}

auto MakeRpcServerInternal(SilKit::Core::EndpointId serviceId, SilKit::Core::EndpointId parentServiceId,
                           const std::string& clientUuid) -> ServiceDescriptor
{
    ServiceDescriptor descriptor;
    descriptor.SetParticipantNameAndComputeId("ServerParticipant");
    descriptor.SetServiceName("RpcSrvInternal");
    descriptor.SetNetworkName("srv-internal-net");
    descriptor.SetServiceType(ServiceType::Controller);
    descriptor.SetServiceId(serviceId);
    descriptor.SetSupplementalDataItem(Discovery::controllerType, Discovery::controllerTypeRpcServerInternal);
    descriptor.SetSupplementalDataItem(Discovery::supplKeyRpcServerInternalParentServiceID,
                                       std::to_string(parentServiceId));
    descriptor.SetSupplementalDataItem(Discovery::supplKeyRpcServerInternalClientUUID, clientUuid);
    return descriptor;
}

auto MakeBusController(const std::string& ctrlType, const std::string& networkName, const std::string& serviceName,
                       SilKit::Core::EndpointId serviceId,
                       const std::string& participantName = "CtrlParticipant") -> ServiceDescriptor
{
    ServiceDescriptor descriptor;
    descriptor.SetParticipantNameAndComputeId(participantName);
    descriptor.SetServiceName(serviceName);
    descriptor.SetNetworkName(networkName);
    descriptor.SetServiceType(ServiceType::Controller);
    descriptor.SetServiceId(serviceId);
    descriptor.SetSupplementalDataItem(Discovery::controllerType, ctrlType);
    return descriptor;
}

auto MakeNetworkLink(const std::string& networkName, const std::string& simulatorParticipant) -> ServiceDescriptor
{
    ServiceDescriptor descriptor;
    descriptor.SetParticipantNameAndComputeId(simulatorParticipant);
    descriptor.SetServiceName(networkName + "_link");
    descriptor.SetNetworkName(networkName);
    descriptor.SetServiceType(ServiceType::Link);
    return descriptor;
}

} // namespace

TEST_F(Test_CapiServiceDiscovery, subscriber_reported_immediately_on_creation)
{
    // A DataSubscriber is visible as soon as it is created, even without any connections.
    CallbackData data;
    auto handler = InstallHandler(data);

    handler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeUserSubscriber(100, "T"));

    EXPECT_EQ(data.callCount, 1);
    EXPECT_EQ(data.lastType, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated);
    EXPECT_EQ(data.serviceKind, SilKit_Experimental_ServiceKind_DataSubscriber);
    EXPECT_EQ(data.primaryIdentifier, "T");
    EXPECT_EQ(data.numberOfConnections, 0u);
    EXPECT_EQ(data.connectedParticipantName, "");
    EXPECT_EQ(data.connectedServiceName, "");
}

TEST_F(Test_CapiServiceDiscovery, subscriber_service_updated_on_connection)
{
    // When a DataSubscriberInternal confirms a match, a Service_Updated with the new count fires.
    CallbackData data;
    auto handler = InstallHandler(data);

    handler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeUserSubscriber(100, "T"));
    ASSERT_EQ(data.callCount, 1);

    handler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeInternalConnection(200, 100));

    EXPECT_EQ(data.callCount, 2);
    EXPECT_EQ(data.lastType, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceUpdated);
    EXPECT_EQ(data.serviceKind, SilKit_Experimental_ServiceKind_DataSubscriber);
    EXPECT_EQ(data.primaryIdentifier, "T");
    EXPECT_EQ(data.numberOfConnections, 1u);
}

TEST_F(Test_CapiServiceDiscovery, subscriber_service_updated_shows_peer_info)
{
    // The Service_Updated event for a DataSubscriber connection names the publisher as the peer.
    CallbackData data;
    auto handler = InstallHandler(data);

    const std::string uuid = "pub-uuid-abc";
    handler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeDataPublisher(uuid, "PubParticipant", "MyPub"));
    handler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeUserSubscriber(100, "T"));
    handler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeInternalConnection(200, 100, uuid));

    ASSERT_EQ(data.callCount, 3); // publisher + subscriber + service_updated
    EXPECT_EQ(data.lastType, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceUpdated);
    EXPECT_EQ(data.connectedParticipantName, "PubParticipant");
    EXPECT_EQ(data.connectedServiceName, "MyPub");
    EXPECT_EQ(data.numberOfConnections, 1u);
}

TEST_F(Test_CapiServiceDiscovery, subscriber_service_updated_on_disconnection)
{
    // When the DataSubscriberInternal is removed the count decrements via another Service_Updated.
    CallbackData data;
    auto handler = InstallHandler(data);

    handler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeUserSubscriber(100, "T"));
    handler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeInternalConnection(200, 100));
    ASSERT_EQ(data.callCount, 2);

    handler(ServiceDiscoveryEvent::Type::ServiceRemoved, MakeInternalConnection(200, 100));

    EXPECT_EQ(data.callCount, 3);
    EXPECT_EQ(data.lastType, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceUpdated);
    EXPECT_EQ(data.numberOfConnections, 0u);
}

TEST_F(Test_CapiServiceDiscovery, subscriber_service_removed_on_removal)
{
    // ServiceRemoved fires when the DataSubscriber itself is removed, not on last-connection loss.
    CallbackData data;
    auto handler = InstallHandler(data);

    handler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeUserSubscriber(100, "T"));
    handler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeInternalConnection(200, 100));
    ASSERT_EQ(data.callCount, 2);

    handler(ServiceDiscoveryEvent::Type::ServiceRemoved, MakeUserSubscriber(100, "T"));

    EXPECT_EQ(data.callCount, 3);
    EXPECT_EQ(data.lastType, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceRemoved);
    EXPECT_EQ(data.serviceKind, SilKit_Experimental_ServiceKind_DataSubscriber);
}

TEST_F(Test_CapiServiceDiscovery, subscriber_connection_arrives_before_subscriber)
{
    // When the DataSubscriberInternal arrives before the DataSubscriber, the subscriber is reported
    // as ServiceCreated (with connections already counted) once the subscriber itself arrives.
    CallbackData data;
    auto handler = InstallHandler(data);

    handler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeInternalConnection(200, 100));
    EXPECT_EQ(data.callCount, 0); // subscriber not known yet

    handler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeUserSubscriber(100, "T"));
    EXPECT_EQ(data.callCount, 1);
    EXPECT_EQ(data.lastType, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated);
    EXPECT_EQ(data.serviceKind, SilKit_Experimental_ServiceKind_DataSubscriber);
    EXPECT_EQ(data.numberOfConnections, 1u); // connection already counted
}

TEST_F(Test_CapiServiceDiscovery, subscriber_multiple_connections_counted)
{
    // Each new DataSubscriberInternal fires a Service_Updated with an incremented count.
    CallbackData data;
    auto handler = InstallHandler(data);

    handler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeUserSubscriber(100, "T"));
    handler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeInternalConnection(200, 100));
    EXPECT_EQ(data.numberOfConnections, 1u);

    handler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeInternalConnection(201, 100));
    EXPECT_EQ(data.callCount, 3);
    EXPECT_EQ(data.numberOfConnections, 2u);

    handler(ServiceDiscoveryEvent::Type::ServiceRemoved, MakeInternalConnection(200, 100));
    EXPECT_EQ(data.numberOfConnections, 1u);

    handler(ServiceDiscoveryEvent::Type::ServiceRemoved, MakeInternalConnection(201, 100));
    EXPECT_EQ(data.numberOfConnections, 0u);
    EXPECT_EQ(data.lastType, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceUpdated);
    EXPECT_EQ(data.callCount, 5);
}

// --------------------------------------------------------------------------------------------------
// RpcServer: reported immediately on creation; Service_Updated fires on each RpcServerInternal event.
// --------------------------------------------------------------------------------------------------

TEST_F(Test_CapiServiceDiscovery, rpc_server_reported_immediately_on_creation)
{
    CallbackData data;
    auto handler = InstallHandler(data);

    handler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeRpcServer(50, "Add"));

    EXPECT_EQ(data.callCount, 1);
    EXPECT_EQ(data.lastType, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated);
    EXPECT_EQ(data.serviceKind, SilKit_Experimental_ServiceKind_RpcServer);
    EXPECT_EQ(data.primaryIdentifier, "Add");
    EXPECT_EQ(data.numberOfConnections, 0u);
}

TEST_F(Test_CapiServiceDiscovery, rpc_server_service_updated_on_connection)
{
    // When a RpcServerInternal confirms a call match, a Service_Updated fires on the parent server.
    CallbackData data;
    auto handler = InstallHandler(data);

    handler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeRpcServer(50, "Add"));
    ASSERT_EQ(data.callCount, 1);

    handler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeRpcServerInternal(60, 50, "client-uuid-1"));

    EXPECT_EQ(data.callCount, 2);
    EXPECT_EQ(data.lastType, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceUpdated);
    EXPECT_EQ(data.serviceKind, SilKit_Experimental_ServiceKind_RpcServer);
    EXPECT_EQ(data.numberOfConnections, 1u);
}

TEST_F(Test_CapiServiceDiscovery, rpc_server_service_updated_shows_peer_info)
{
    // The Service_Updated for an RPC connection names the client as the peer.
    CallbackData data;
    auto handler = InstallHandler(data);

    const std::string clientUuid = "client-uuid-xyz";
    handler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeRpcClient(clientUuid, "ClientParticipant", "MyClient"));
    handler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeRpcServer(50, "Add"));
    handler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeRpcServerInternal(60, 50, clientUuid));

    ASSERT_EQ(data.callCount, 3);
    EXPECT_EQ(data.lastType, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceUpdated);
    EXPECT_EQ(data.connectedParticipantName, "ClientParticipant");
    EXPECT_EQ(data.connectedServiceName, "MyClient");
    EXPECT_EQ(data.numberOfConnections, 1u);
}

TEST_F(Test_CapiServiceDiscovery, rpc_server_service_updated_on_disconnection)
{
    CallbackData data;
    auto handler = InstallHandler(data);

    handler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeRpcServer(50, "Add"));
    handler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeRpcServerInternal(60, 50, "client-uuid"));
    ASSERT_EQ(data.callCount, 2);

    handler(ServiceDiscoveryEvent::Type::ServiceRemoved, MakeRpcServerInternal(60, 50, "client-uuid"));

    EXPECT_EQ(data.callCount, 3);
    EXPECT_EQ(data.lastType, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceUpdated);
    EXPECT_EQ(data.numberOfConnections, 0u);
}

// --------------------------------------------------------------------------------------------------
// Network simulator detection: bus controllers carry isSimulated / simulatingParticipantName.
// No NetworkLink event type is exposed at the public API level.
// --------------------------------------------------------------------------------------------------

TEST_F(Test_CapiServiceDiscovery, is_simulated_false_for_controller_without_link)
{
    CallbackData data;
    auto handler = InstallHandler(data);

    handler(ServiceDiscoveryEvent::Type::ServiceCreated,
            MakeBusController(Discovery::controllerTypeCan, "CAN1", "Can1", 1));

    EXPECT_EQ(data.callCount, 1);
    EXPECT_FALSE(data.isSimulated);
    EXPECT_EQ(data.simulatingParticipantName, "");
}

TEST_F(Test_CapiServiceDiscovery, is_simulated_set_when_link_arrives_after_controller)
{
    // When a network simulator announces itself after the bus controller, a Service_Updated fires.
    CallbackData data;
    auto handler = InstallHandler(data);

    handler(ServiceDiscoveryEvent::Type::ServiceCreated,
            MakeBusController(Discovery::controllerTypeCan, "CAN1", "Can1", 1));
    ASSERT_EQ(data.callCount, 1);
    ASSERT_FALSE(data.isSimulated);

    handler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeNetworkLink("CAN1", "SimParticipant"));

    EXPECT_EQ(data.callCount, 2);
    EXPECT_EQ(data.lastType, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceUpdated);
    EXPECT_EQ(data.serviceKind, SilKit_Experimental_ServiceKind_CanController);
    EXPECT_TRUE(data.isSimulated);
    EXPECT_EQ(data.simulatingParticipantName, "SimParticipant");
}

TEST_F(Test_CapiServiceDiscovery, is_simulated_set_on_service_created_when_link_precedes_controller)
{
    // When the network link is already active before the bus controller arrives, the ServiceCreated
    // event for that controller already carries isSimulated = true — no separate Service_Updated needed.
    CallbackData data;
    auto handler = InstallHandler(data);

    handler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeNetworkLink("CAN1", "SimParticipant"));
    EXPECT_EQ(data.callCount, 0); // link itself not reported

    handler(ServiceDiscoveryEvent::Type::ServiceCreated,
            MakeBusController(Discovery::controllerTypeCan, "CAN1", "Can1", 1));

    EXPECT_EQ(data.callCount, 1);
    EXPECT_EQ(data.lastType, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated);
    EXPECT_TRUE(data.isSimulated);
    EXPECT_EQ(data.simulatingParticipantName, "SimParticipant");
}

TEST_F(Test_CapiServiceDiscovery, is_simulated_cleared_on_link_removal)
{
    // When the network simulator disappears, a Service_Updated clears isSimulated.
    CallbackData data;
    auto handler = InstallHandler(data);

    handler(ServiceDiscoveryEvent::Type::ServiceCreated,
            MakeBusController(Discovery::controllerTypeCan, "CAN1", "Can1", 1));
    handler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeNetworkLink("CAN1", "SimParticipant"));
    ASSERT_EQ(data.callCount, 2);
    ASSERT_TRUE(data.isSimulated);

    handler(ServiceDiscoveryEvent::Type::ServiceRemoved, MakeNetworkLink("CAN1", "SimParticipant"));

    EXPECT_EQ(data.callCount, 3);
    EXPECT_EQ(data.lastType, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceUpdated);
    EXPECT_FALSE(data.isSimulated);
    EXPECT_EQ(data.simulatingParticipantName, "");
}

TEST_F(Test_CapiServiceDiscovery, link_fan_out_reaches_all_controllers_on_network)
{
    // A single link event fans out to every bus controller on that network.
    CallbackData data;
    auto handler = InstallHandler(data);

    handler(ServiceDiscoveryEvent::Type::ServiceCreated,
            MakeBusController(Discovery::controllerTypeCan, "CAN1", "Can1", 1));
    handler(ServiceDiscoveryEvent::Type::ServiceCreated,
            MakeBusController(Discovery::controllerTypeCan, "CAN1", "Can2", 2));
    ASSERT_EQ(data.callCount, 2);

    handler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeNetworkLink("CAN1", "SimParticipant"));

    // Both controllers receive a Service_Updated.
    EXPECT_EQ(data.callCount, 4);
}

} // namespace
