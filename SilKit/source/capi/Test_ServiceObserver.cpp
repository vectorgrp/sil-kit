// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

#include "gtest/gtest.h"

#include "silkit/capi/SilKit.h"
#include "silkit/services/datatypes.hpp"

#include "capi/ServiceObserver.hpp"

#include "core/internal/ServiceDescriptor.hpp"
#include "core/internal/ServiceConfigKeys.hpp"
#include "config/YamlParser.hpp"

namespace {

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

// A single captured event. Copying into owned storage also verifies that the struct and all its
// borrowed pointers are valid for the duration of the callback.
struct CapturedEvent
{
    SilKit_Experimental_ServiceDiscoveryEvent_Type type{SilKit_Experimental_ServiceDiscoveryEvent_Type_Invalid};
    SilKit_Experimental_ServiceKind serviceKind{SilKit_Experimental_ServiceKind_Undefined};
    std::string participantName;
    std::string serviceName;
    uint64_t serviceId{0};
    std::string primaryIdentifier;
    SilKit_Experimental_SimulatedNetworkType networkType{SilKit_NetworkType_Undefined};
    std::string mediaType;
    std::vector<CapturedLabel> labels;
    SilKit_OperationMode operationMode{SilKit_OperationMode_Invalid};
    bool timeSyncActive{false};
    std::string connectedParticipantName;
    std::string connectedServiceName;
    uint64_t connectedServiceId{0};
    bool isSnapshot{false};
};

// Captures every event the public handler receives, plus the latest one for convenience.
struct CallbackData
{
    int callCount{0};
    std::vector<CapturedEvent> events;

    const CapturedEvent& last() const
    {
        return events.back();
    }
};

void SilKitCALL CapturingHandler(void* context, SilKit_Experimental_ServiceDiscoveryEvent_Type type,
                                 const SilKit_Experimental_ServiceDescriptor* serviceDescriptor)
{
    auto* data = static_cast<CallbackData*>(context);
    data->callCount += 1;

    CapturedEvent event;
    event.type = type;
    event.serviceKind = serviceDescriptor->serviceKind;
    event.participantName = serviceDescriptor->participantName;
    event.serviceName = serviceDescriptor->serviceName;
    event.serviceId = serviceDescriptor->serviceId;
    event.networkType = serviceDescriptor->networkType;
    event.primaryIdentifier = serviceDescriptor->primaryIdentifier;
    event.mediaType = serviceDescriptor->mediaType;
    for (size_t i = 0; i < serviceDescriptor->labelList.numLabels; ++i)
    {
        const auto& label = serviceDescriptor->labelList.labels[i];
        event.labels.push_back({label.key, label.value, label.kind});
    }
    event.operationMode = serviceDescriptor->operationMode;
    event.timeSyncActive = serviceDescriptor->timeSyncActive == SilKit_True;
    event.connectedParticipantName = serviceDescriptor->connectedParticipantName;
    event.connectedServiceName = serviceDescriptor->connectedServiceName;
    event.connectedServiceId = serviceDescriptor->connectedServiceId;
    event.isSnapshot = serviceDescriptor->isSnapshot == SilKit_True;

    data->events.push_back(std::move(event));
}

auto SerializeLabels(const std::vector<SilKit::Services::MatchingLabel>& labels) -> std::string
{
    return SilKit::Config::Serialize(labels);
}

auto MakeController(const std::string& controllerType, const std::string& networkName,
                    const std::string& serviceName) -> ServiceDescriptor
{
    ServiceDescriptor descriptor;
    descriptor.SetParticipantNameAndComputeId("ParticipantA");
    descriptor.SetServiceName(serviceName);
    descriptor.SetNetworkName(networkName);
    descriptor.SetServiceType(ServiceType::Controller);
    descriptor.SetSupplementalDataItem(Discovery::controllerType, controllerType);
    return descriptor;
}

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
                       const std::string& serviceName, SilKit::Core::EndpointId serviceId = 7) -> ServiceDescriptor
{
    ServiceDescriptor descriptor;
    descriptor.SetParticipantNameAndComputeId(participantName);
    descriptor.SetServiceName(serviceName);
    descriptor.SetServiceId(serviceId);
    descriptor.SetNetworkName(publisherUuid);
    descriptor.SetNetworkType(SilKit::Config::NetworkType::Data);
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
                   const std::string& serviceName, SilKit::Core::EndpointId serviceId = 8) -> ServiceDescriptor
{
    ServiceDescriptor descriptor;
    descriptor.SetParticipantNameAndComputeId(participantName);
    descriptor.SetServiceName(serviceName);
    descriptor.SetServiceId(serviceId);
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
    descriptor.SetNetworkType(SilKit::Config::NetworkType::CAN);
    descriptor.SetServiceType(ServiceType::Link);
    return descriptor;
}

// Drives the ServiceObserver directly: no CAPI wrapper and no mock participant. The observer emits
// through the C handler into `data`.
class Test_ServiceObserver : public testing::Test
{
public:
    CallbackData data;
    VSilKit::ServiceObserver observer{&CapturingHandler, &data};
};

// --------------------------------------------------------------------------------------------------
// Basic classification.
// --------------------------------------------------------------------------------------------------

TEST_F(Test_ServiceObserver, reports_can_controller)
{
    auto descriptor = MakeController(Discovery::controllerTypeCan, "CAN1", "Can1");
    descriptor.SetServiceId(42);
    descriptor.SetNetworkType(SilKit::Config::NetworkType::CAN);
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated, descriptor);

    ASSERT_EQ(data.callCount, 1);
    EXPECT_EQ(data.last().type, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated);
    EXPECT_EQ(data.last().serviceKind, SilKit_Experimental_ServiceKind_CanController);
    EXPECT_EQ(data.last().participantName, "ParticipantA");
    EXPECT_EQ(data.last().serviceName, "Can1");
    EXPECT_EQ(data.last().serviceId, 42u);
    EXPECT_EQ(data.last().primaryIdentifier, "CAN1"); // bus controller: identifier is the network name
    EXPECT_EQ(data.last().networkType, SilKit_NetworkType_CAN);
    EXPECT_EQ(data.last().mediaType, "");
    EXPECT_TRUE(data.last().labels.empty());
    EXPECT_EQ(data.last().operationMode, SilKit_OperationMode_Invalid);
    EXPECT_FALSE(data.last().timeSyncActive);
    EXPECT_EQ(data.last().connectedParticipantName, "");
    EXPECT_EQ(data.last().connectedServiceName, "");
    EXPECT_EQ(data.last().connectedServiceId, 0u);
    EXPECT_FALSE(data.last().isSnapshot);
}

TEST_F(Test_ServiceObserver, reports_data_publisher_with_decoded_metadata)
{
    auto descriptor = MakeController(Discovery::controllerTypeDataPublisher, "pub-uuid-1234", "MyPublisher");
    descriptor.SetSupplementalDataItem(Discovery::supplKeyDataPublisherTopic, "TopicA");
    descriptor.SetSupplementalDataItem(Discovery::supplKeyDataPublisherMediaType, "application/json");
    descriptor.SetSupplementalDataItem(
        Discovery::supplKeyDataPublisherPubLabels,
        SerializeLabels({{"kA", "vA", SilKit::Services::MatchingLabel::Kind::Mandatory},
                         {"kB", "vB", SilKit::Services::MatchingLabel::Kind::Optional}}));

    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated, descriptor);

    ASSERT_EQ(data.callCount, 1);
    EXPECT_EQ(data.last().serviceKind, SilKit_Experimental_ServiceKind_DataPublisher);
    EXPECT_EQ(data.last().primaryIdentifier, "TopicA"); // pub/sub: identifier is the topic
    EXPECT_EQ(data.last().mediaType, "application/json");
    ASSERT_EQ(data.last().labels.size(), 2u);
    EXPECT_EQ(data.last().labels[0].key, "kA");
    EXPECT_EQ(data.last().labels[0].value, "vA");
    EXPECT_EQ(data.last().labels[0].kind, SilKit_LabelKind_Mandatory);
    EXPECT_EQ(data.last().labels[1].key, "kB");
    EXPECT_EQ(data.last().labels[1].kind, SilKit_LabelKind_Optional);
}

TEST_F(Test_ServiceObserver, reports_rpc_client_with_function_name)
{
    auto descriptor = MakeController(Discovery::controllerTypeRpcClient, "rpc-uuid-9", "MyClient");
    descriptor.SetSupplementalDataItem(Discovery::supplKeyRpcClientFunctionName, "Add");
    descriptor.SetSupplementalDataItem(Discovery::supplKeyRpcClientMediaType, "application/octet-stream");

    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated, descriptor);

    ASSERT_EQ(data.callCount, 1);
    EXPECT_EQ(data.last().serviceKind, SilKit_Experimental_ServiceKind_RpcClient);
    EXPECT_EQ(data.last().primaryIdentifier, "Add"); // rpc: identifier is the function name
    EXPECT_EQ(data.last().mediaType, "application/octet-stream");
}

TEST_F(Test_ServiceObserver, reports_removal)
{
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceRemoved,
                         MakeController(Discovery::controllerTypeLin, "LIN1", "Lin1"));

    ASSERT_EQ(data.callCount, 1);
    EXPECT_EQ(data.last().type, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceRemoved);
    EXPECT_EQ(data.last().serviceKind, SilKit_Experimental_ServiceKind_LinController);
}

TEST_F(Test_ServiceObserver, reports_lifecycle_service_with_operation_mode)
{
    auto coordinated = MakeController(Discovery::controllerTypeLifecycleService, "default", "LifecycleService");
    coordinated.SetSupplementalDataItem(Discovery::lifecycleIsCoordinated,
                                        std::to_string(SilKit_OperationMode_Coordinated));
    auto autonomous = MakeController(Discovery::controllerTypeLifecycleService, "default", "LifecycleService");
    autonomous.SetSupplementalDataItem(Discovery::lifecycleIsCoordinated,
                                       std::to_string(SilKit_OperationMode_Autonomous));
    const auto unknown = MakeController(Discovery::controllerTypeLifecycleService, "default", "LifecycleService");

    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated, coordinated);
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated, autonomous);
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceRemoved, unknown);

    ASSERT_EQ(data.callCount, 3);
    EXPECT_EQ(data.events[0].serviceKind, SilKit_Experimental_ServiceKind_LifecycleService);
    EXPECT_EQ(data.events[0].participantName, "ParticipantA");
    EXPECT_EQ(data.events[0].primaryIdentifier, "");
    EXPECT_EQ(data.events[0].operationMode, SilKit_OperationMode_Coordinated);
    EXPECT_EQ(data.events[1].operationMode, SilKit_OperationMode_Autonomous);
    EXPECT_EQ(data.events[2].type, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceRemoved);
    EXPECT_EQ(data.events[2].operationMode, SilKit_OperationMode_Invalid);
}

TEST_F(Test_ServiceObserver, reports_time_sync_service_with_synchronization_state)
{
    auto synchronized = MakeController(Discovery::controllerTypeTimeSyncService, "default", "TimeSyncService");
    synchronized.SetSupplementalDataItem(Discovery::timeSyncActive, "1");
    auto unsynchronized = MakeController(Discovery::controllerTypeTimeSyncService, "default", "TimeSyncService");
    unsynchronized.SetSupplementalDataItem(Discovery::timeSyncActive, "0");

    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated, synchronized);
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated, unsynchronized);

    ASSERT_EQ(data.callCount, 2);
    EXPECT_EQ(data.events[0].serviceKind, SilKit_Experimental_ServiceKind_TimeSyncService);
    EXPECT_EQ(data.events[0].primaryIdentifier, "");
    EXPECT_TRUE(data.events[0].timeSyncActive);
    EXPECT_FALSE(data.events[1].timeSyncActive);
    EXPECT_EQ(data.events[1].operationMode, SilKit_OperationMode_Invalid);
}

TEST_F(Test_ServiceObserver, filters_infrastructure_and_internal_services)
{
    // Infrastructure controller (SystemMonitor) is not user-facing.
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated,
                         MakeController(Discovery::controllerTypeSystemMonitor, "default", "SystemMonitor"));
    // DataSubscriberInternal with no parent id is silently dropped (no match to report).
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated,
                         MakeController(Discovery::controllerTypeDataSubscriberInternal, "sub-uuid", "SubInternal"));

    // A controller without a controller.type supplemental entry is skipped, too.
    ServiceDescriptor noControllerType;
    noControllerType.SetParticipantNameAndComputeId("ParticipantA");
    noControllerType.SetServiceName("Mystery");
    noControllerType.SetServiceType(ServiceType::Controller);
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated, noControllerType);

    EXPECT_EQ(data.callCount, 0);
}

TEST_F(Test_ServiceObserver, malformed_labels_are_swallowed)
{
    auto descriptor = MakeController(Discovery::controllerTypeDataPublisher, "pub-uuid", "MyPublisher");
    descriptor.SetSupplementalDataItem(Discovery::supplKeyDataPublisherTopic, "TopicA");
    descriptor.SetSupplementalDataItem(Discovery::supplKeyDataPublisherPubLabels, "{ this is : not [ valid");

    // Must not throw across the FFI boundary; the service is still reported.
    EXPECT_NO_THROW(observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated, descriptor));
    ASSERT_EQ(data.callCount, 1);
    EXPECT_EQ(data.last().serviceKind, SilKit_Experimental_ServiceKind_DataPublisher);
    EXPECT_EQ(data.last().primaryIdentifier, "TopicA");
}

TEST_F(Test_ServiceObserver, reports_data_subscriber_with_decoded_labels)
{
    auto descriptor = MakeController(Discovery::controllerTypeDataSubscriber, "default", "MySubscriber");
    descriptor.SetSupplementalDataItem(Discovery::supplKeyDataSubscriberTopic, "TopicS");
    descriptor.SetSupplementalDataItem(
        Discovery::supplKeyDataSubscriberSubLabels,
        SerializeLabels({{"sk", "sv", SilKit::Services::MatchingLabel::Kind::Mandatory}}));

    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated, descriptor);

    ASSERT_EQ(data.callCount, 1);
    EXPECT_EQ(data.last().serviceKind, SilKit_Experimental_ServiceKind_DataSubscriber);
    EXPECT_EQ(data.last().primaryIdentifier, "TopicS");
    ASSERT_EQ(data.last().labels.size(), 1u);
    EXPECT_EQ(data.last().labels[0].key, "sk");
    EXPECT_EQ(data.last().labels[0].value, "sv");
    EXPECT_EQ(data.last().labels[0].kind, SilKit_LabelKind_Mandatory);
}

TEST_F(Test_ServiceObserver, reports_ethernet_and_flexray_controllers)
{
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated,
                         MakeController(Discovery::controllerTypeEthernet, "ETH1", "Eth1"));
    EXPECT_EQ(data.last().serviceKind, SilKit_Experimental_ServiceKind_EthernetController);
    EXPECT_EQ(data.last().primaryIdentifier, "ETH1");

    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated,
                         MakeController(Discovery::controllerTypeFlexray, "FR1", "Fr1"));
    EXPECT_EQ(data.last().serviceKind, SilKit_Experimental_ServiceKind_FlexrayController);
    EXPECT_EQ(data.last().primaryIdentifier, "FR1");
}

// --------------------------------------------------------------------------------------------------
// Pub/sub matches: reported as a PubSubMatch once both endpoints are known.
// --------------------------------------------------------------------------------------------------

TEST_F(Test_ServiceObserver, subscriber_reported_immediately_on_creation)
{
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated, MakeUserSubscriber(100, "T"));

    ASSERT_EQ(data.callCount, 1);
    EXPECT_EQ(data.last().type, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated);
    EXPECT_EQ(data.last().serviceKind, SilKit_Experimental_ServiceKind_DataSubscriber);
    EXPECT_EQ(data.last().primaryIdentifier, "T");
    EXPECT_EQ(data.last().connectedParticipantName, "");
    EXPECT_EQ(data.last().connectedServiceName, "");
}

TEST_F(Test_ServiceObserver, pubsub_match_reported_as_match)
{
    const std::string uuid = "pub-uuid-abc";
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated,
                         MakeDataPublisher(uuid, "PubParticipant", "MyPub"));
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated, MakeUserSubscriber(100, "T"));
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated, MakeInternalConnection(200, 100, uuid));

    ASSERT_EQ(data.callCount, 3); // publisher + subscriber + match
    EXPECT_EQ(data.last().type, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated);
    EXPECT_EQ(data.last().serviceKind, SilKit_Experimental_ServiceKind_PubSubMatch);
    EXPECT_EQ(data.last().participantName, "SubParticipant"); // receiving side
    EXPECT_EQ(data.last().serviceName, "Sub");
    EXPECT_EQ(data.last().serviceId, 100u);
    EXPECT_EQ(data.last().primaryIdentifier, "T");
    EXPECT_EQ(data.last().networkType, SilKit_NetworkType_Undefined);
    EXPECT_EQ(data.last().connectedParticipantName, "PubParticipant"); // peer
    EXPECT_EQ(data.last().connectedServiceName, "MyPub");
    EXPECT_EQ(data.last().connectedServiceId, 7u);
}

TEST_F(Test_ServiceObserver, pubsub_match_deferred_until_publisher_arrives)
{
    const std::string uuid = "pub-uuid-late";
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated, MakeUserSubscriber(100, "T"));
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated, MakeInternalConnection(200, 100, uuid));
    EXPECT_EQ(data.callCount, 1); // only the subscriber so far; the match is pending

    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated,
                         MakeDataPublisher(uuid, "PubParticipant", "MyPub"));

    ASSERT_EQ(data.callCount, 3); // publisher event + the now-resolvable match
    EXPECT_EQ(data.last().serviceKind, SilKit_Experimental_ServiceKind_PubSubMatch);
    EXPECT_EQ(data.last().connectedParticipantName, "PubParticipant");
    EXPECT_EQ(data.last().connectedServiceName, "MyPub");
}

TEST_F(Test_ServiceObserver, pubsub_match_deferred_until_subscriber_arrives)
{
    const std::string uuid = "pub-uuid-replay";
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated, MakeInternalConnection(200, 100, uuid));
    EXPECT_EQ(data.callCount, 0); // nothing user-facing yet

    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated,
                         MakeDataPublisher(uuid, "PubParticipant", "MyPub"));
    EXPECT_EQ(data.callCount, 1); // publisher, but subscriber still unknown

    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated, MakeUserSubscriber(100, "T"));

    ASSERT_EQ(data.callCount, 3); // subscriber event + the now-resolvable match
    EXPECT_EQ(data.last().serviceKind, SilKit_Experimental_ServiceKind_PubSubMatch);
    EXPECT_EQ(data.last().connectedParticipantName, "PubParticipant");
    EXPECT_EQ(data.last().connectedServiceName, "MyPub");
}

// --------------------------------------------------------------------------------------------------
// Match removal: a pub/sub or RPC match is removed by its internal endpoint going away, or right before
// either of its endpoints is reported as removed.
// --------------------------------------------------------------------------------------------------

class Test_ServiceObserverMatchRemoval : public Test_ServiceObserver
{
public:
    const std::string uuid{"pub-uuid-teardown"};

    void SetUp() override
    {
        observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated,
                             MakeDataPublisher(uuid, "PubParticipant", "MyPub"));
        observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated, MakeUserSubscriber(100, "T"));
        observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated, MakeInternalConnection(200, 100, uuid));
        ASSERT_EQ(data.callCount, 3);
        ASSERT_EQ(data.last().serviceKind, SilKit_Experimental_ServiceKind_PubSubMatch);
    }

    void ExpectMatchRemovedAt(size_t index)
    {
        ASSERT_GT(data.events.size(), index);
        const auto& event = data.events[index];
        EXPECT_EQ(event.type, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceRemoved);
        EXPECT_EQ(event.serviceKind, SilKit_Experimental_ServiceKind_PubSubMatch);
        EXPECT_EQ(event.participantName, "SubParticipant");
        EXPECT_EQ(event.serviceName, "Sub");
        EXPECT_EQ(event.primaryIdentifier, "T");
        EXPECT_EQ(event.connectedParticipantName, "PubParticipant");
        EXPECT_EQ(event.connectedServiceName, "MyPub");
    }
};

TEST_F(Test_ServiceObserverMatchRemoval, removed_with_internal_endpoint)
{
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceRemoved, MakeInternalConnection(200, 100, uuid));

    ASSERT_EQ(data.callCount, 4);
    ExpectMatchRemovedAt(3);
}

TEST_F(Test_ServiceObserverMatchRemoval, removed_before_publisher)
{
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceRemoved,
                         MakeDataPublisher(uuid, "PubParticipant", "MyPub"));

    ASSERT_EQ(data.callCount, 5);
    ExpectMatchRemovedAt(3);
    EXPECT_EQ(data.events[4].type, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceRemoved);
    EXPECT_EQ(data.events[4].serviceKind, SilKit_Experimental_ServiceKind_DataPublisher);
}

TEST_F(Test_ServiceObserverMatchRemoval, removed_before_subscriber)
{
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceRemoved, MakeUserSubscriber(100, "T"));

    ASSERT_EQ(data.callCount, 5);
    ExpectMatchRemovedAt(3);
    EXPECT_EQ(data.events[4].type, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceRemoved);
    EXPECT_EQ(data.events[4].serviceKind, SilKit_Experimental_ServiceKind_DataSubscriber);
}

TEST_F(Test_ServiceObserverMatchRemoval, removed_only_once)
{
    // The internal endpoint goes first; the later endpoint removals must not report the match again.
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceRemoved, MakeInternalConnection(200, 100, uuid));
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceRemoved,
                         MakeDataPublisher(uuid, "PubParticipant", "MyPub"));
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceRemoved, MakeUserSubscriber(100, "T"));

    ASSERT_EQ(data.callCount, 6);
    ExpectMatchRemovedAt(3);
    EXPECT_EQ(data.events[4].serviceKind, SilKit_Experimental_ServiceKind_DataPublisher);
    EXPECT_EQ(data.events[5].serviceKind, SilKit_Experimental_ServiceKind_DataSubscriber);
}

TEST_F(Test_ServiceObserverMatchRemoval, recreated_after_endpoint_returns)
{
    // A publisher leaving and coming back with a new internal endpoint yields a new match.
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceRemoved,
                         MakeDataPublisher(uuid, "PubParticipant", "MyPub"));
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceRemoved, MakeInternalConnection(200, 100, uuid));
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated,
                         MakeDataPublisher(uuid, "PubParticipant", "MyPub"));
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated, MakeInternalConnection(201, 100, uuid));

    ASSERT_EQ(data.callCount, 7); // match removed, publisher removed, publisher created, match created
    EXPECT_EQ(data.last().type, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated);
    EXPECT_EQ(data.last().serviceKind, SilKit_Experimental_ServiceKind_PubSubMatch);
}

TEST_F(Test_ServiceObserver, pending_match_removal_reports_nothing)
{
    // The match was never reported (publisher unknown), so its removal is not observable.
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated, MakeUserSubscriber(100, "T"));
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated, MakeInternalConnection(200, 100, "unknown"));
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceRemoved, MakeInternalConnection(200, 100, "unknown"));

    EXPECT_EQ(data.callCount, 1); // the subscriber only
}

TEST_F(Test_ServiceObserver, duplicate_internal_announcement_reports_match_once)
{
    const std::string uuid = "pub-uuid-dup";
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated,
                         MakeDataPublisher(uuid, "PubParticipant", "MyPub"));
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated, MakeUserSubscriber(100, "T"));
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated, MakeInternalConnection(200, 100, uuid));
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated, MakeInternalConnection(200, 100, uuid));

    EXPECT_EQ(data.callCount, 3);
}

TEST_F(Test_ServiceObserver, subscriber_service_removed_on_removal)
{
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated, MakeUserSubscriber(100, "T"));
    ASSERT_EQ(data.callCount, 1);

    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceRemoved, MakeUserSubscriber(100, "T"));

    ASSERT_EQ(data.callCount, 2);
    EXPECT_EQ(data.last().type, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceRemoved);
    EXPECT_EQ(data.last().serviceKind, SilKit_Experimental_ServiceKind_DataSubscriber);
}

TEST_F(Test_ServiceObserver, pubsub_multiple_matches_each_reported)
{
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated, MakeUserSubscriber(100, "T"));
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated, MakeDataPublisher("uuid-1", "PubA", "PubCtrlA"));
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated, MakeDataPublisher("uuid-2", "PubB", "PubCtrlB"));
    const int before = data.callCount;

    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated, MakeInternalConnection(200, 100, "uuid-1"));
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated, MakeInternalConnection(201, 100, "uuid-2"));

    ASSERT_EQ(data.callCount, before + 2);
    EXPECT_EQ(data.events[before].serviceKind, SilKit_Experimental_ServiceKind_PubSubMatch);
    EXPECT_EQ(data.events[before].connectedParticipantName, "PubA");
    EXPECT_EQ(data.events[before + 1].serviceKind, SilKit_Experimental_ServiceKind_PubSubMatch);
    EXPECT_EQ(data.events[before + 1].connectedParticipantName, "PubB");
}

// --------------------------------------------------------------------------------------------------
// RPC matches: reported as an RpcMatch once both endpoints are known.
// --------------------------------------------------------------------------------------------------

TEST_F(Test_ServiceObserver, rpc_server_reported_immediately_on_creation)
{
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated, MakeRpcServer(50, "Add"));

    ASSERT_EQ(data.callCount, 1);
    EXPECT_EQ(data.last().type, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated);
    EXPECT_EQ(data.last().serviceKind, SilKit_Experimental_ServiceKind_RpcServer);
    EXPECT_EQ(data.last().primaryIdentifier, "Add");
}

TEST_F(Test_ServiceObserver, rpc_match_reported_as_match)
{
    const std::string clientUuid = "client-uuid-xyz";
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated,
                         MakeRpcClient(clientUuid, "ClientParticipant", "MyClient"));
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated, MakeRpcServer(50, "Add"));
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated, MakeRpcServerInternal(60, 50, clientUuid));

    ASSERT_EQ(data.callCount, 3);
    EXPECT_EQ(data.last().type, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated);
    EXPECT_EQ(data.last().serviceKind, SilKit_Experimental_ServiceKind_RpcMatch);
    EXPECT_EQ(data.last().participantName, "ServerParticipant"); // receiving side
    EXPECT_EQ(data.last().serviceName, "RpcSrv");
    EXPECT_EQ(data.last().serviceId, 50u);
    EXPECT_EQ(data.last().primaryIdentifier, "Add");
    EXPECT_EQ(data.last().connectedParticipantName, "ClientParticipant"); // peer
    EXPECT_EQ(data.last().connectedServiceName, "MyClient");
    EXPECT_EQ(data.last().connectedServiceId, 8u);
}

TEST_F(Test_ServiceObserver, rpc_match_deferred_until_client_arrives)
{
    const std::string clientUuid = "client-uuid-late";
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated, MakeRpcServer(50, "Add"));
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated, MakeRpcServerInternal(60, 50, clientUuid));
    EXPECT_EQ(data.callCount, 1); // only the server; the match is pending

    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated,
                         MakeRpcClient(clientUuid, "ClientParticipant", "MyClient"));

    ASSERT_EQ(data.callCount, 3); // client event + the now-resolvable match
    EXPECT_EQ(data.last().serviceKind, SilKit_Experimental_ServiceKind_RpcMatch);
    EXPECT_EQ(data.last().connectedParticipantName, "ClientParticipant");
    EXPECT_EQ(data.last().connectedServiceName, "MyClient");
}

TEST_F(Test_ServiceObserver, rpc_match_removed_before_client)
{
    const std::string clientUuid = "client-uuid-gone";
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated,
                         MakeRpcClient(clientUuid, "ClientParticipant", "MyClient"));
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated, MakeRpcServer(50, "Add"));
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated, MakeRpcServerInternal(60, 50, clientUuid));
    ASSERT_EQ(data.callCount, 3);

    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceRemoved,
                         MakeRpcClient(clientUuid, "ClientParticipant", "MyClient"));

    ASSERT_EQ(data.callCount, 5);
    EXPECT_EQ(data.events[3].type, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceRemoved);
    EXPECT_EQ(data.events[3].serviceKind, SilKit_Experimental_ServiceKind_RpcMatch);
    EXPECT_EQ(data.events[3].connectedParticipantName, "ClientParticipant");
    EXPECT_EQ(data.events[4].type, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceRemoved);
    EXPECT_EQ(data.events[4].serviceKind, SilKit_Experimental_ServiceKind_RpcClient);
}

// --------------------------------------------------------------------------------------------------
// Network-simulator links: reported as a NetworkSimulatorLink (created and removed).
// --------------------------------------------------------------------------------------------------

TEST_F(Test_ServiceObserver, network_link_reported_as_link_service)
{
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated, MakeNetworkLink("CAN1", "SimParticipant"));

    ASSERT_EQ(data.callCount, 1);
    EXPECT_EQ(data.last().type, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated);
    EXPECT_EQ(data.last().serviceKind, SilKit_Experimental_ServiceKind_NetworkSimulatorLink);
    EXPECT_EQ(data.last().participantName, "SimParticipant");
    EXPECT_EQ(data.last().serviceName, "CAN1_link");
    EXPECT_EQ(data.last().primaryIdentifier, "CAN1"); // network name, joins to bus controllers' identifier
    EXPECT_EQ(data.last().networkType, SilKit_NetworkType_CAN); // bus type of the simulated network
    EXPECT_EQ(data.last().connectedParticipantName, "");
    EXPECT_EQ(data.last().connectedServiceName, "");
}

TEST_F(Test_ServiceObserver, network_link_removal_reported)
{
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated, MakeNetworkLink("CAN1", "SimParticipant"));
    ASSERT_EQ(data.callCount, 1);

    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceRemoved, MakeNetworkLink("CAN1", "SimParticipant"));

    ASSERT_EQ(data.callCount, 2);
    EXPECT_EQ(data.last().type, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceRemoved);
    EXPECT_EQ(data.last().serviceKind, SilKit_Experimental_ServiceKind_NetworkSimulatorLink);
    EXPECT_EQ(data.last().primaryIdentifier, "CAN1");
}

TEST_F(Test_ServiceObserver, bus_controller_reported_independently_of_link)
{
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated,
                         MakeBusController(Discovery::controllerTypeCan, "CAN1", "Can1", 1));
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated, MakeNetworkLink("CAN1", "SimParticipant"));

    ASSERT_EQ(data.callCount, 2);
    EXPECT_EQ(data.events[0].serviceKind, SilKit_Experimental_ServiceKind_CanController);
    EXPECT_EQ(data.events[0].primaryIdentifier, "CAN1");
    EXPECT_EQ(data.events[1].serviceKind, SilKit_Experimental_ServiceKind_NetworkSimulatorLink);
    EXPECT_EQ(data.events[1].primaryIdentifier, "CAN1"); // same network -> the controller is simulated
}

void SilKitCALL ThrowOnPublisherHandler(void* context, SilKit_Experimental_ServiceDiscoveryEvent_Type type,
                                        const SilKit_Experimental_ServiceDescriptor* serviceDescriptor)
{
    CapturingHandler(context, type, serviceDescriptor);
    if (serviceDescriptor->serviceKind == SilKit_Experimental_ServiceKind_DataPublisher)
    {
        throw std::runtime_error{"handler failure"};
    }
}

// A throwing handler must not cost the matches resolved by the same event.
TEST(Test_ServiceObserverExceptions, throwing_handler_keeps_remaining_matches)
{
    CallbackData data;
    VSilKit::ServiceObserver observer{&ThrowOnPublisherHandler, &data};

    const std::string uuid = "pub-uuid-throw";
    for (SilKit::Core::EndpointId i = 0; i < 3; ++i)
    {
        observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated, MakeUserSubscriber(100 + i, "T"));
        observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated,
                             MakeInternalConnection(200 + i, 100 + i, uuid));
    }

    EXPECT_NO_THROW(observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated,
                                         MakeDataPublisher(uuid, "PubParticipant", "MyPub")));

    const auto matches = std::count_if(data.events.begin(), data.events.end(), [](const CapturedEvent& e) {
        return e.serviceKind == SilKit_Experimental_ServiceKind_PubSubMatch;
    });
    EXPECT_EQ(matches, 3);
}

// --------------------------------------------------------------------------------------------------
// Snapshot flag: carried by everything emitted for a snapshot event, including resolved matches.
// --------------------------------------------------------------------------------------------------

TEST_F(Test_ServiceObserver, snapshot_flag_is_carried_to_service_and_resolved_match)
{
    const std::string uuid = "pub-uuid-snapshot";
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated, MakeUserSubscriber(100, "T"), true);
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated, MakeInternalConnection(200, 100, uuid), true);
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceCreated,
                         MakeDataPublisher(uuid, "PubParticipant", "MyPub"), true);

    ASSERT_EQ(data.callCount, 3); // subscriber, publisher, match
    for (const auto& event : data.events)
    {
        EXPECT_TRUE(event.isSnapshot);
    }
    EXPECT_EQ(data.last().serviceKind, SilKit_Experimental_ServiceKind_PubSubMatch);

    // A live event after the snapshot is not flagged.
    observer.HandleEvent(ServiceDiscoveryEvent::Type::ServiceRemoved,
                         MakeDataPublisher(uuid, "PubParticipant", "MyPub"));
    ASSERT_EQ(data.callCount, 5); // match removed, publisher removed
    EXPECT_FALSE(data.events[3].isSnapshot);
    EXPECT_FALSE(data.events[4].isSnapshot);
}

} // namespace
