// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <chrono>
#include <functional>
#include <set>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ServiceDiscovery.hpp"
#include "string_utils_internal.hpp"
#include "Uuid.hpp"
#include "LabelMatching.hpp"
#include "MockParticipant.hpp"
#include "MockServiceEndpoint.hpp"
#include "YamlParser.hpp"

namespace {

using namespace std::chrono_literals;

using namespace testing;

using namespace SilKit;
using namespace SilKit::Core;
using namespace SilKit::Core::Discovery;
using namespace SilKit::Util;

using ::SilKit::Core::Tests::DummyParticipant;

class TestWrapperSpecificDiscoveryStore : public SpecificDiscoveryStore
{
public:
    std::unordered_map<FilterType, DiscoveryKeyNode, FilterTypeHash>& GetLookup()
    {
        return _lookup;
    };
};

class Callbacks
{
public:
    MOCK_METHOD(void, ServiceDiscoveryHandler, (ServiceDiscoveryEvent::Type, const ServiceDescriptor&));
};
class Test_SpecificDiscoveryStore : public testing::Test
{
protected:
    Test_SpecificDiscoveryStore() {}


protected:
    // ----------------------------------------
    // Helper Methods

protected:
    // ----------------------------------------
    // Members

    Callbacks callbacks;
};

TEST_F(Test_SpecificDiscoveryStore, no_reaction_on_irrelevant_services)
{
    std::string controllerTypes[] = {controllerTypeServiceDiscovery,
                                     controllerTypeCan,
                                     controllerTypeDataSubscriberInternal,
                                     controllerTypeEthernet,
                                     controllerTypeFlexray,
                                     controllerTypeLifecycleService,
                                     controllerTypeLin,
                                     controllerTypeLoggerReceiver,
                                     controllerTypeLoggerSender,
                                     controllerTypeSystemController};

    ServiceDescriptor testDescriptor{};
    testDescriptor.SetParticipantNameAndComputeId("ParticipantA");
    testDescriptor.SetNetworkName("Link1");
    testDescriptor.SetServiceName("ServiceDiscovery");


    TestWrapperSpecificDiscoveryStore testStore;

    for (std::string& ctrlType : controllerTypes)
    {
        testDescriptor.SetSupplementalDataItem(Core::Discovery::controllerType, ctrlType);
        testStore.ServiceChange(ServiceDiscoveryEvent::Type::ServiceCreated, testDescriptor);
    }

    auto& lookup = testStore.GetLookup();
    ASSERT_EQ(lookup.size(), 0u);
}

TEST_F(Test_SpecificDiscoveryStore, lookup_entries_pubsub)
{
    ServiceDescriptor baseDescriptor{};
    baseDescriptor.SetParticipantNameAndComputeId("ParticipantA");
    baseDescriptor.SetNetworkName("Link1");
    baseDescriptor.SetServiceName("ServiceDiscovery");
    baseDescriptor.SetSupplementalDataItem(Core::Discovery::controllerType, controllerTypeDataPublisher);
    baseDescriptor.SetSupplementalDataItem(supplKeyDataPublisherTopic, "Topic1");
    baseDescriptor.SetSupplementalDataItem(supplKeyDataPublisherMediaType, "text/json");
    baseDescriptor.SetSupplementalDataItem(supplKeyDataPublisherPubLabels, "[]");

    ServiceDescriptor noLabelTestDescriptor{baseDescriptor};
    noLabelTestDescriptor.SetServiceId(1);

    ServiceDescriptor labelTestDescriptor{baseDescriptor};
    labelTestDescriptor.SetSupplementalDataItem(supplKeyDataPublisherPubLabels, "- key: kA\n  value: vA\n  kind: 2");
    labelTestDescriptor.SetServiceId(2);

    TestWrapperSpecificDiscoveryStore testStore;
    testStore.ServiceChange(ServiceDiscoveryEvent::Type::ServiceCreated, noLabelTestDescriptor);

    auto& lookup = testStore.GetLookup();
    ASSERT_EQ(lookup.size(), 1u);
    auto& entry = lookup[std::make_tuple(controllerTypeDataPublisher, "Topic1")];
    ASSERT_EQ(entry.noLabelCluster.nodes[0], noLabelTestDescriptor);
    ASSERT_EQ(entry.allCluster.nodes[0], noLabelTestDescriptor);

    testStore.ServiceChange(ServiceDiscoveryEvent::Type::ServiceCreated, labelTestDescriptor);

    //refresh lookup and entry
    entry = lookup[std::make_tuple(controllerTypeDataPublisher, "Topic1")];

    ASSERT_EQ(entry.noLabelCluster.nodes.size(), 1u);
    ASSERT_EQ(entry.noLabelCluster.nodes[0], noLabelTestDescriptor);
    auto& labelMapEntry = entry.labelMap[std::make_tuple("kA", "vA")];
    ASSERT_EQ(entry.labelMap[std::make_tuple("kA", "vA")].nodes[0], labelTestDescriptor);
    ASSERT_EQ(entry.allCluster.nodes[0], noLabelTestDescriptor);
    ASSERT_EQ(entry.allCluster.nodes[1], labelTestDescriptor);

    // check that old descriptor with no labels was added to notLabelMap
    ASSERT_EQ(entry.notLabelMap["kA"].nodes[0], noLabelTestDescriptor);

    testStore.ServiceChange(ServiceDiscoveryEvent::Type::ServiceRemoved, labelTestDescriptor);
    //refresh lookup and entry
    entry = lookup[std::make_tuple(controllerTypeDataPublisher, "Topic1")];

    ASSERT_EQ(entry.noLabelCluster.nodes.size(), 1u);
    ASSERT_EQ(entry.noLabelCluster.nodes[0], noLabelTestDescriptor);

    labelMapEntry = entry.labelMap[std::make_tuple("kA", "vA")];
    ASSERT_EQ(entry.labelMap[std::make_tuple("kA", "vA")].nodes.size(), 0u);
    ASSERT_EQ(entry.allCluster.nodes[0], noLabelTestDescriptor);
    ASSERT_EQ(entry.allCluster.nodes.size(), 1u);

    // check that old descriptor with no labels still exists notLabelMap
    ASSERT_EQ(entry.notLabelMap["kA"].nodes[0], noLabelTestDescriptor);
}

TEST_F(Test_SpecificDiscoveryStore, lookup_entries_rpc_client)
{
    ServiceDescriptor baseDescriptor{};
    baseDescriptor.SetParticipantNameAndComputeId("ParticipantA");
    baseDescriptor.SetNetworkName("Link1");
    baseDescriptor.SetServiceName("ServiceDiscovery");
    baseDescriptor.SetSupplementalDataItem(Core::Discovery::controllerType, controllerTypeRpcClient);
    baseDescriptor.SetSupplementalDataItem(supplKeyRpcClientFunctionName, "FunctionName1");
    baseDescriptor.SetSupplementalDataItem(supplKeyRpcClientMediaType, "text/json");
    baseDescriptor.SetSupplementalDataItem(supplKeyRpcClientLabels, "[]");

    ServiceDescriptor noLabelTestDescriptor{baseDescriptor};
    noLabelTestDescriptor.SetServiceId(1);

    ServiceDescriptor labelTestDescriptor{baseDescriptor};
    labelTestDescriptor.SetSupplementalDataItem(supplKeyRpcClientLabels, "- key: kA\n  value: vA\n  kind: 2");
    labelTestDescriptor.SetServiceId(2);

    TestWrapperSpecificDiscoveryStore testStore;
    testStore.ServiceChange(ServiceDiscoveryEvent::Type::ServiceCreated, noLabelTestDescriptor);

    auto& lookup = testStore.GetLookup();
    ASSERT_EQ(lookup.size(), 1u);
    auto& entry = lookup[std::make_tuple(controllerTypeRpcClient, "FunctionName1")];
    ASSERT_EQ(entry.noLabelCluster.nodes[0], noLabelTestDescriptor);
    ASSERT_EQ(entry.allCluster.nodes[0], noLabelTestDescriptor);

    testStore.ServiceChange(ServiceDiscoveryEvent::Type::ServiceCreated, labelTestDescriptor);

    //refresh lookup and entry
    entry = lookup[std::make_tuple(controllerTypeRpcClient, "FunctionName1")];

    ASSERT_EQ(entry.noLabelCluster.nodes.size(), 1u);
    ASSERT_EQ(entry.noLabelCluster.nodes[0], noLabelTestDescriptor);
    auto& labelMapEntry = entry.labelMap[std::make_tuple("kA", "vA")];
    ASSERT_EQ(entry.labelMap[std::make_tuple("kA", "vA")].nodes[0], labelTestDescriptor);
    ASSERT_EQ(entry.allCluster.nodes[0], noLabelTestDescriptor);
    ASSERT_EQ(entry.allCluster.nodes[1], labelTestDescriptor);

    // check that old descriptor with no labels was added to notLabelMap
    ASSERT_EQ(entry.notLabelMap["kA"].nodes[0], noLabelTestDescriptor);

    testStore.ServiceChange(ServiceDiscoveryEvent::Type::ServiceRemoved, labelTestDescriptor);
    //refresh lookup and entry
    entry = lookup[std::make_tuple(controllerTypeRpcClient, "FunctionName1")];

    ASSERT_EQ(entry.noLabelCluster.nodes.size(), 1u);
    ASSERT_EQ(entry.noLabelCluster.nodes[0], noLabelTestDescriptor);

    labelMapEntry = entry.labelMap[std::make_tuple("kA", "vA")];
    ASSERT_EQ(entry.labelMap[std::make_tuple("kA", "vA")].nodes.size(), 0u);
    ASSERT_EQ(entry.allCluster.nodes[0], noLabelTestDescriptor);
    ASSERT_EQ(entry.allCluster.nodes.size(), 1u);

    // check that old descriptor with no labels still exists notLabelMap
    ASSERT_EQ(entry.notLabelMap["kA"].nodes[0], noLabelTestDescriptor);
}

TEST_F(Test_SpecificDiscoveryStore, lookup_entries_rpc_server_internal)
{
    std::string uuid = "dda9a411-2bc8-4428-9e62-bd3000278b9e";
    ServiceDescriptor baseDescriptor{};
    baseDescriptor.SetParticipantNameAndComputeId("ParticipantA");
    baseDescriptor.SetNetworkName("Link1");
    baseDescriptor.SetServiceName("ServiceDiscovery");
    baseDescriptor.SetSupplementalDataItem(Core::Discovery::controllerType, controllerTypeRpcServerInternal);
    baseDescriptor.SetSupplementalDataItem(supplKeyRpcServerInternalClientUUID, uuid);
    baseDescriptor.SetSupplementalDataItem(supplKeyRpcServerMediaType, "text/json");

    ServiceDescriptor noLabelTestDescriptor{baseDescriptor};
    noLabelTestDescriptor.SetServiceId(1);

    TestWrapperSpecificDiscoveryStore testStore;
    testStore.ServiceChange(ServiceDiscoveryEvent::Type::ServiceCreated, noLabelTestDescriptor);

    auto& lookup = testStore.GetLookup();
    ASSERT_EQ(lookup.size(), 1u);
    auto& entry = lookup[std::make_tuple(controllerTypeRpcServerInternal, uuid)];
    ASSERT_EQ(entry.noLabelCluster.nodes[0], noLabelTestDescriptor);
    ASSERT_EQ(entry.allCluster.nodes[0], noLabelTestDescriptor);

    testStore.ServiceChange(ServiceDiscoveryEvent::Type::ServiceRemoved, noLabelTestDescriptor);
    //refresh lookup and entry
    entry = lookup[std::make_tuple(controllerTypeRpcServerInternal, uuid)];
    ASSERT_EQ(entry.allCluster.nodes.size(), 0u);
}

TEST_F(Test_SpecificDiscoveryStore, lookup_handler_then_service_discovery)
{
    TestWrapperSpecificDiscoveryStore testStore;

    ServiceDescriptor baseDescriptor{};
    baseDescriptor.SetParticipantNameAndComputeId("ParticipantA");
    baseDescriptor.SetNetworkName("Link1");
    baseDescriptor.SetServiceName("ServiceDiscovery");
    baseDescriptor.SetSupplementalDataItem(Core::Discovery::controllerType, controllerTypeDataPublisher);
    baseDescriptor.SetSupplementalDataItem(supplKeyDataPublisherTopic, "Topic1");
    baseDescriptor.SetSupplementalDataItem(supplKeyDataPublisherMediaType, "text/json");
    baseDescriptor.SetSupplementalDataItem(supplKeyDataPublisherPubLabels, "[]");

    ServiceDescriptor noLabelTestDescriptor{baseDescriptor};
    noLabelTestDescriptor.SetServiceId(1);

    testStore.RegisterSpecificServiceDiscoveryHandler(
        [this](ServiceDiscoveryEvent::Type discoveryType, const ServiceDescriptor& sd) {
        callbacks.ServiceDiscoveryHandler(discoveryType, sd);
    }, controllerTypeDataPublisher, "Topic1", {});
    EXPECT_CALL(callbacks, ServiceDiscoveryHandler(ServiceDiscoveryEvent::Type::ServiceCreated, noLabelTestDescriptor))
        .Times(1);

    testStore.ServiceChange(ServiceDiscoveryEvent::Type::ServiceCreated, noLabelTestDescriptor);
}

TEST_F(Test_SpecificDiscoveryStore, lookup_service_discovery_then_handler_no_labels)
{
    TestWrapperSpecificDiscoveryStore testStore;

    ServiceDescriptor baseDescriptor{};
    baseDescriptor.SetParticipantNameAndComputeId("ParticipantA");
    baseDescriptor.SetNetworkName("Link1");
    baseDescriptor.SetServiceName("ServiceDiscovery");
    baseDescriptor.SetSupplementalDataItem(Core::Discovery::controllerType, controllerTypeDataPublisher);
    baseDescriptor.SetSupplementalDataItem(supplKeyDataPublisherTopic, "Topic1");
    baseDescriptor.SetSupplementalDataItem(supplKeyDataPublisherMediaType, "text/json");
    baseDescriptor.SetSupplementalDataItem(supplKeyDataPublisherPubLabels, "[]");

    ServiceDescriptor noLabelTestDescriptor{baseDescriptor};
    noLabelTestDescriptor.SetServiceId(1);

    testStore.ServiceChange(ServiceDiscoveryEvent::Type::ServiceCreated, noLabelTestDescriptor);

    EXPECT_CALL(callbacks, ServiceDiscoveryHandler(ServiceDiscoveryEvent::Type::ServiceCreated, noLabelTestDescriptor))
        .Times(1);
    testStore.RegisterSpecificServiceDiscoveryHandler(
        [this](ServiceDiscoveryEvent::Type discoveryType, const ServiceDescriptor& sd) {
        callbacks.ServiceDiscoveryHandler(discoveryType, sd);
    }, controllerTypeDataPublisher, "Topic1", {});
}

TEST_F(Test_SpecificDiscoveryStore, lookup_service_discovery_then_handler_labels)
{
    TestWrapperSpecificDiscoveryStore testStore;
    SilKit::Services::MatchingLabel label = {"keyA", "valA", SilKit::Services::MatchingLabel::Kind::Mandatory};

    ServiceDescriptor baseDescriptor{};
    baseDescriptor.SetParticipantNameAndComputeId("ParticipantA");
    baseDescriptor.SetNetworkName("Link1");
    baseDescriptor.SetServiceName("ServiceDiscovery");
    baseDescriptor.SetSupplementalDataItem(Core::Discovery::controllerType, controllerTypeDataPublisher);
    baseDescriptor.SetSupplementalDataItem(supplKeyDataPublisherTopic, "Topic1");
    baseDescriptor.SetSupplementalDataItem(supplKeyDataPublisherMediaType, "text/json");
    baseDescriptor.SetSupplementalDataItem(supplKeyDataPublisherPubLabels, "- key: kA\n  value: vA\n  kind: 2");

    ServiceDescriptor noLabelTestDescriptor{baseDescriptor};
    noLabelTestDescriptor.SetServiceId(1);

    testStore.ServiceChange(ServiceDiscoveryEvent::Type::ServiceCreated, noLabelTestDescriptor);

    EXPECT_CALL(callbacks, ServiceDiscoveryHandler(ServiceDiscoveryEvent::Type::ServiceCreated, noLabelTestDescriptor))
        .Times(1);

    testStore.RegisterSpecificServiceDiscoveryHandler(
        [this](ServiceDiscoveryEvent::Type discoveryType, const ServiceDescriptor& sd) {
        callbacks.ServiceDiscoveryHandler(discoveryType, sd);
    }, controllerTypeDataPublisher, "Topic1", {label});
}

TEST_F(Test_SpecificDiscoveryStore, lookup_service_discovery_then_handler_issues)
{
    TestWrapperSpecificDiscoveryStore testStore;

    ServiceDescriptor baseDescriptor{};
    baseDescriptor.SetParticipantNameAndComputeId("ParticipantA");
    baseDescriptor.SetNetworkName("Link1");
    baseDescriptor.SetServiceName("ServiceDiscovery");
    baseDescriptor.SetSupplementalDataItem(Core::Discovery::controllerType, controllerTypeDataPublisher);
    baseDescriptor.SetSupplementalDataItem(supplKeyDataPublisherTopic, "Topic1");
    baseDescriptor.SetSupplementalDataItem(supplKeyDataPublisherMediaType, "text/json");
    baseDescriptor.SetSupplementalDataItem(supplKeyDataPublisherPubLabels, "[]");

    ServiceDescriptor noLabelTestDescriptor{baseDescriptor};
    noLabelTestDescriptor.SetServiceId(1);

    ServiceDescriptor labelTestDescriptor{baseDescriptor};
    labelTestDescriptor.SetSupplementalDataItem(supplKeyDataPublisherPubLabels,
                                                " - key: kA\n   value: vA\n   kind: 2\n - key: kB\n   value: vB\n   "
                                                "kind: 2\n - key: kC\n   value: vC\n   kind: 2 ");
    labelTestDescriptor.SetServiceId(2);

    testStore.ServiceChange(ServiceDiscoveryEvent::Type::ServiceCreated, labelTestDescriptor);
    testStore.ServiceChange(ServiceDiscoveryEvent::Type::ServiceCreated, noLabelTestDescriptor);

    EXPECT_CALL(callbacks, ServiceDiscoveryHandler(ServiceDiscoveryEvent::Type::ServiceCreated, noLabelTestDescriptor))
        .Times(2);

    EXPECT_CALL(callbacks, ServiceDiscoveryHandler(ServiceDiscoveryEvent::Type::ServiceCreated, labelTestDescriptor))
        .Times(1);

    std::vector<SilKit::Services::MatchingLabel> optionalSubscriberLabels{
        {"kA", "vA", SilKit::Services::MatchingLabel::Kind::Optional},
        {"kB", "vB2", SilKit::Services::MatchingLabel::Kind::Optional},
        {"kC", "vC", SilKit::Services::MatchingLabel::Kind::Optional}};

    testStore.RegisterSpecificServiceDiscoveryHandler(
        [this, optionalSubscriberLabels](ServiceDiscoveryEvent::Type discoveryType, const ServiceDescriptor& sd) {
        std::vector<SilKit::Services::MatchingLabel> labels;
        std::string labelsStr;
        if (sd.GetSupplementalDataItem(supplKeyDataPublisherPubLabels, labelsStr))
        {
            labels = SilKit::Config::Deserialize<decltype(labels)>(labelsStr);
        }
        if (MatchLabels(labels, optionalSubscriberLabels))
        {
            callbacks.ServiceDiscoveryHandler(discoveryType, sd);
        }
    }, controllerTypeDataPublisher, "Topic1", optionalSubscriberLabels);

    std::vector<SilKit::Services::MatchingLabel> optionalSubscriberLabels2{
        {"kA", "vA", SilKit::Services::MatchingLabel::Kind::Optional},
        {"kB", "vB", SilKit::Services::MatchingLabel::Kind::Optional},
        {"kC", "vC", SilKit::Services::MatchingLabel::Kind::Optional}};
    testStore.RegisterSpecificServiceDiscoveryHandler(
        [this, optionalSubscriberLabels2](ServiceDiscoveryEvent::Type discoveryType, const ServiceDescriptor& sd) {
        std::vector<SilKit::Services::MatchingLabel> labels;
        std::string labelsStr;
        if (sd.GetSupplementalDataItem(supplKeyDataPublisherPubLabels, labelsStr))
        {
            labels = SilKit::Config::Deserialize<decltype(labels)>(labelsStr);
        }
        if (MatchLabels(labels, optionalSubscriberLabels2))
        {
            callbacks.ServiceDiscoveryHandler(discoveryType, sd);
        }
    }, controllerTypeDataPublisher, "Topic1", optionalSubscriberLabels2);
}

} // namespace
