// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "silkit/capi/SilKit.h"

#include "silkit/SilKit.hpp"
#include "silkit/detail/impl/ThrowOnError.hpp"
#include "silkit/experimental/participant/ParticipantExtensions.hpp"
#include "silkit/experimental/serviceDiscovery/IServiceDiscovery.hpp"
#include "silkit/experimental/serviceDiscovery/string_utils.hpp"

#include "MockCapiTest.hpp"

#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace {

using testing::_;
using testing::DoAll;
using testing::Return;
using testing::SaveArg;
using testing::SetArgPointee;

namespace SD = SilKit::Experimental::ServiceDiscovery;
namespace Detail = SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME;

using ServiceDiscoveryWrapper = Detail::Impl::Experimental::ServiceDiscovery::ServiceDiscovery;

class Test_HourglassServiceDiscovery : public SilKitHourglassTests::MockCapiTest
{
public:
    SilKit_Participant* mockParticipant{reinterpret_cast<SilKit_Participant*>(uintptr_t(0x12345678))};
    SilKit_Experimental_ServiceDiscovery* mockServiceDiscovery{
        reinterpret_cast<SilKit_Experimental_ServiceDiscovery*>(uintptr_t(0x45362718))};

    Test_HourglassServiceDiscovery()
    {
        ON_CALL(capi, SilKit_Experimental_ServiceDiscovery_Create(_, _))
            .WillByDefault(DoAll(SetArgPointee<0>(mockServiceDiscovery), Return(SilKit_ReturnCode_SUCCESS)));
    }
};

TEST_F(Test_HourglassServiceDiscovery, SilKit_Experimental_ServiceDiscovery_Create)
{
    Detail::Impl::Participant participant{mockParticipant};

    EXPECT_CALL(capi, SilKit_Experimental_ServiceDiscovery_Create(_, mockParticipant));

    auto* serviceDiscovery = Detail::Experimental::Participant::CreateServiceDiscovery(&participant);
    EXPECT_NE(serviceDiscovery, nullptr);
}

TEST_F(Test_HourglassServiceDiscovery, SilKit_Experimental_ServiceDiscovery_SetServiceDiscoveryHandler)
{
    ServiceDiscoveryWrapper serviceDiscovery{mockParticipant};

    void* capturedContext{nullptr};
    SilKit_Experimental_ServiceDiscoveryHandler_t capturedHandler{nullptr};
    EXPECT_CALL(capi, SilKit_Experimental_ServiceDiscovery_SetServiceDiscoveryHandler(mockServiceDiscovery, _, _))
        .WillOnce(DoAll(SaveArg<1>(&capturedContext), SaveArg<2>(&capturedHandler),
                        Return(SilKit_ReturnCode_SUCCESS)));

    serviceDiscovery.SetServiceDiscoveryHandler([](SD::ServiceDiscoveryEventType, const SD::ServiceDescriptor&) {});

    EXPECT_NE(capturedContext, nullptr);
    EXPECT_NE(capturedHandler, nullptr);
}

// The C trampoline installed by the wrapper must translate the borrowed C descriptor into an owned
// C++ ServiceDescriptor and dispatch it to the user's std::function.
TEST_F(Test_HourglassServiceDiscovery, service_discovery_handler_round_trip)
{
    ServiceDiscoveryWrapper serviceDiscovery{mockParticipant};

    void* capturedContext{nullptr};
    SilKit_Experimental_ServiceDiscoveryHandler_t capturedHandler{nullptr};
    EXPECT_CALL(capi, SilKit_Experimental_ServiceDiscovery_SetServiceDiscoveryHandler(mockServiceDiscovery, _, _))
        .WillOnce(DoAll(SaveArg<1>(&capturedContext), SaveArg<2>(&capturedHandler),
                        Return(SilKit_ReturnCode_SUCCESS)));

    SD::ServiceDiscoveryEventType receivedEvent{SD::ServiceDiscoveryEventType::Invalid};
    SD::ServiceDescriptor received{};
    int callCount{0};
    serviceDiscovery.SetServiceDiscoveryHandler(
        [&](SD::ServiceDiscoveryEventType eventType, const SD::ServiceDescriptor& descriptor) {
        ++callCount;
        receivedEvent = eventType;
        received = descriptor;
    });
    ASSERT_NE(capturedHandler, nullptr);

    // Case 1: a data publisher with media type and two labels.
    SilKit_Label cLabels[2];
    cLabels[0].key = "kA";
    cLabels[0].value = "vA";
    cLabels[0].kind = SilKit_LabelKind_Mandatory;
    cLabels[1].key = "kB";
    cLabels[1].value = "vB";
    cLabels[1].kind = SilKit_LabelKind_Optional;

    SilKit_Experimental_ServiceDescriptor cDescriptor;
    SilKit_Struct_Init(SilKit_Experimental_ServiceDescriptor, cDescriptor);
    cDescriptor.participantName = "ParticipantA";
    cDescriptor.serviceName = "MyPublisher";
    cDescriptor.serviceId = 17;
    cDescriptor.serviceKind = SilKit_Experimental_ServiceKind_DataPublisher;
    cDescriptor.primaryIdentifier = "TopicA";
    cDescriptor.mediaType = "application/json";
    cDescriptor.labelList.numLabels = 2;
    cDescriptor.labelList.labels = cLabels;
    cDescriptor.isSnapshot = SilKit_True;

    capturedHandler(capturedContext, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated, &cDescriptor);

    EXPECT_EQ(callCount, 1);
    EXPECT_EQ(receivedEvent, SD::ServiceDiscoveryEventType::ServiceCreated);
    EXPECT_EQ(received.participantName, "ParticipantA");
    EXPECT_EQ(received.serviceName, "MyPublisher");
    EXPECT_EQ(received.serviceId, 17u);
    EXPECT_EQ(received.serviceKind, SD::ServiceKind::DataPublisher);
    EXPECT_EQ(received.primaryIdentifier, "TopicA");
    EXPECT_TRUE(received.isSnapshot);
    EXPECT_EQ(received.mediaType, "application/json");
    ASSERT_EQ(received.labels.size(), 2u);
    EXPECT_EQ(received.labels[0].key, "kA");
    EXPECT_EQ(received.labels[0].value, "vA");
    EXPECT_EQ(received.labels[0].kind, SilKit::Services::MatchingLabel::Kind::Mandatory);
    EXPECT_EQ(received.labels[1].key, "kB");
    EXPECT_EQ(received.labels[1].kind, SilKit::Services::MatchingLabel::Kind::Optional);

    // Case 2: a bus controller removal without labels/media type.
    SilKit_Experimental_ServiceDescriptor cBusDescriptor;
    SilKit_Struct_Init(SilKit_Experimental_ServiceDescriptor, cBusDescriptor);
    cBusDescriptor.participantName = "ParticipantB";
    cBusDescriptor.serviceName = "Can1";
    cBusDescriptor.serviceKind = SilKit_Experimental_ServiceKind_CanController;
    cBusDescriptor.primaryIdentifier = "CAN1";
    cBusDescriptor.networkType = SilKit_NetworkType_CAN;
    cBusDescriptor.mediaType = "";
    cBusDescriptor.labelList.numLabels = 0;
    cBusDescriptor.labelList.labels = nullptr;

    capturedHandler(capturedContext, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceRemoved, &cBusDescriptor);

    EXPECT_EQ(callCount, 2);
    EXPECT_EQ(receivedEvent, SD::ServiceDiscoveryEventType::ServiceRemoved);
    EXPECT_EQ(received.serviceKind, SD::ServiceKind::CanController);
    EXPECT_EQ(received.primaryIdentifier, "CAN1");
    EXPECT_EQ(received.networkType, SilKit::Experimental::NetworkSimulation::SimulatedNetworkType::CAN);
    EXPECT_TRUE(received.mediaType.empty());
    EXPECT_TRUE(received.labels.empty());
    EXPECT_FALSE(received.isSnapshot);

    // Case 3: a match and the orchestration fields.
    SilKit_Experimental_ServiceDescriptor cMatchDescriptor;
    SilKit_Struct_Init(SilKit_Experimental_ServiceDescriptor, cMatchDescriptor);
    cMatchDescriptor.participantName = "SubParticipant";
    cMatchDescriptor.serviceName = "Sub";
    cMatchDescriptor.serviceId = 3;
    cMatchDescriptor.serviceKind = SilKit_Experimental_ServiceKind_PubSubMatch;
    cMatchDescriptor.primaryIdentifier = "TopicA";
    cMatchDescriptor.connectedParticipantName = "ParticipantA";
    cMatchDescriptor.connectedServiceName = "MyPublisher";
    cMatchDescriptor.connectedServiceId = 17;
    cMatchDescriptor.operationMode = SilKit_OperationMode_Autonomous;
    cMatchDescriptor.timeSyncActive = SilKit_True;

    capturedHandler(capturedContext, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated, &cMatchDescriptor);

    EXPECT_EQ(callCount, 3);
    EXPECT_EQ(received.serviceKind, SD::ServiceKind::PubSubMatch);
    EXPECT_EQ(received.serviceId, 3u);
    EXPECT_EQ(received.connectedParticipantName, "ParticipantA");
    EXPECT_EQ(received.connectedServiceName, "MyPublisher");
    EXPECT_EQ(received.connectedServiceId, 17u);
    EXPECT_EQ(received.operationMode, SilKit::Services::Orchestration::OperationMode::Autonomous);
    EXPECT_TRUE(received.timeSyncActive);
}

// Setting the handler twice must register the C trampoline exactly once (no duplicate delivery) and
// replace the user handler in place without dangling the previously stored std::function.
TEST_F(Test_HourglassServiceDiscovery, set_handler_twice_replaces_and_registers_once)
{
    ServiceDiscoveryWrapper serviceDiscovery{mockParticipant};

    void* capturedContext{nullptr};
    SilKit_Experimental_ServiceDiscoveryHandler_t capturedHandler{nullptr};
    EXPECT_CALL(capi, SilKit_Experimental_ServiceDiscovery_SetServiceDiscoveryHandler(mockServiceDiscovery, _, _))
        .Times(1)
        .WillOnce(DoAll(SaveArg<1>(&capturedContext), SaveArg<2>(&capturedHandler),
                        Return(SilKit_ReturnCode_SUCCESS)));

    int firstCalls{0};
    int secondCalls{0};
    serviceDiscovery.SetServiceDiscoveryHandler(
        [&](SD::ServiceDiscoveryEventType, const SD::ServiceDescriptor&) { ++firstCalls; });
    serviceDiscovery.SetServiceDiscoveryHandler(
        [&](SD::ServiceDiscoveryEventType, const SD::ServiceDescriptor&) { ++secondCalls; });

    ASSERT_NE(capturedHandler, nullptr);

    SilKit_Experimental_ServiceDescriptor cDescriptor;
    SilKit_Struct_Init(SilKit_Experimental_ServiceDescriptor, cDescriptor);
    cDescriptor.participantName = "P";
    cDescriptor.serviceName = "S";
    cDescriptor.serviceKind = SilKit_Experimental_ServiceKind_CanController;
    cDescriptor.primaryIdentifier = "CAN1";
    cDescriptor.mediaType = "";
    cDescriptor.labelList.numLabels = 0;
    cDescriptor.labelList.labels = nullptr;

    capturedHandler(capturedContext, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated, &cDescriptor);

    // Only the second (current) handler is invoked; the first was replaced, not left dangling.
    EXPECT_EQ(firstCalls, 0);
    EXPECT_EQ(secondCalls, 1);
}

auto MakeCanDescriptor() -> SilKit_Experimental_ServiceDescriptor
{
    SilKit_Experimental_ServiceDescriptor cDescriptor;
    SilKit_Struct_Init(SilKit_Experimental_ServiceDescriptor, cDescriptor);
    cDescriptor.participantName = "P";
    cDescriptor.serviceName = "S";
    cDescriptor.serviceKind = SilKit_Experimental_ServiceKind_CanController;
    cDescriptor.primaryIdentifier = "CAN1";
    cDescriptor.mediaType = "";
    cDescriptor.labelList.numLabels = 0;
    cDescriptor.labelList.labels = nullptr;
    return cDescriptor;
}

// A handler replacing itself must not destroy its own running closure.
TEST_F(Test_HourglassServiceDiscovery, handler_replacing_itself_is_safe)
{
    ServiceDiscoveryWrapper serviceDiscovery{mockParticipant};

    void* capturedContext{nullptr};
    SilKit_Experimental_ServiceDiscoveryHandler_t capturedHandler{nullptr};
    EXPECT_CALL(capi, SilKit_Experimental_ServiceDiscovery_SetServiceDiscoveryHandler(mockServiceDiscovery, _, _))
        .WillOnce(DoAll(SaveArg<1>(&capturedContext), SaveArg<2>(&capturedHandler), Return(SilKit_ReturnCode_SUCCESS)));

    bool firstDestroyed{false};
    bool firstAliveAfterReplace{false};
    int secondCalls{0};
    {
        // Flags the destruction of the first handler's closure
        std::shared_ptr<int> guard{new int{0}, [&firstDestroyed](int* p) {
            firstDestroyed = true;
            delete p;
        }};
        serviceDiscovery.SetServiceDiscoveryHandler(
            [&serviceDiscovery, &secondCalls, destroyed = &firstDestroyed, aliveAfterReplace = &firstAliveAfterReplace,
             guard](SD::ServiceDiscoveryEventType, const SD::ServiceDescriptor&) {
            // Copied out of the closure under test
            auto& sd = serviceDiscovery;
            auto* const destroyedFlag = destroyed;
            auto* const aliveFlag = aliveAfterReplace;
            sd.SetServiceDiscoveryHandler(
                [&secondCalls](SD::ServiceDiscoveryEventType, const SD::ServiceDescriptor&) { ++secondCalls; });
            *aliveFlag = !*destroyedFlag;
        });
    }
    ASSERT_NE(capturedHandler, nullptr);
    ASSERT_FALSE(firstDestroyed);

    const auto cDescriptor = MakeCanDescriptor();
    capturedHandler(capturedContext, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated, &cDescriptor);

    EXPECT_TRUE(firstAliveAfterReplace);
    EXPECT_TRUE(firstDestroyed);
    EXPECT_EQ(secondCalls, 0);

    capturedHandler(capturedContext, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated, &cDescriptor);

    EXPECT_EQ(secondCalls, 1);
}

// Smoke test (meaningful under a thread sanitizer): replace the handler while another thread invokes it.
TEST_F(Test_HourglassServiceDiscovery, concurrent_set_and_invoke)
{
    ServiceDiscoveryWrapper serviceDiscovery{mockParticipant};

    void* capturedContext{nullptr};
    SilKit_Experimental_ServiceDiscoveryHandler_t capturedHandler{nullptr};
    EXPECT_CALL(capi, SilKit_Experimental_ServiceDiscovery_SetServiceDiscoveryHandler(mockServiceDiscovery, _, _))
        .WillOnce(DoAll(SaveArg<1>(&capturedContext), SaveArg<2>(&capturedHandler), Return(SilKit_ReturnCode_SUCCESS)));

    std::atomic<size_t> payloadBytes{0};
    const auto makeHandler = [&payloadBytes] {
        auto payload = std::make_shared<std::string>("payload");
        return [&payloadBytes, payload](SD::ServiceDiscoveryEventType, const SD::ServiceDescriptor&) {
            payloadBytes += payload->size();
        };
    };

    serviceDiscovery.SetServiceDiscoveryHandler(makeHandler());
    ASSERT_NE(capturedHandler, nullptr);

    const auto cDescriptor = MakeCanDescriptor();
    std::atomic<bool> stop{false};
    std::thread invoker{[&] {
        while (!stop)
        {
            capturedHandler(capturedContext, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated,
                            &cDescriptor);
        }
    }};
    while (payloadBytes == 0)
    {
        std::this_thread::yield();
    }

    for (int i = 0; i < 1000; ++i)
    {
        serviceDiscovery.SetServiceDiscoveryHandler(makeHandler());
    }

    stop = true;
    invoker.join();
}

TEST_F(Test_HourglassServiceDiscovery, to_string_maps_enums)
{
    EXPECT_EQ(SD::to_string(SD::ServiceKind::DataSubscriber), "DataSubscriber");
    EXPECT_EQ(SD::to_string(SD::ServiceKind::RpcServer), "RpcServer");
    EXPECT_EQ(SD::to_string(SD::ServiceKind::NetworkSimulatorLink), "NetworkSimulatorLink");
    EXPECT_EQ(SD::to_string(SD::ServiceKind::PubSubMatch), "PubSubMatch");
    EXPECT_EQ(SD::to_string(SD::ServiceKind::RpcMatch), "RpcMatch");
    EXPECT_EQ(SD::to_string(SD::ServiceKind::LifecycleService), "LifecycleService");
    EXPECT_EQ(SD::to_string(SD::ServiceKind::TimeSyncService), "TimeSyncService");
    EXPECT_EQ(SD::to_string(SD::ServiceDiscoveryEventType::ServiceCreated), "ServiceCreated");
    EXPECT_EQ(SD::to_string(SD::ServiceDiscoveryEventType::ServiceRemoved), "ServiceRemoved");
}

} // namespace
