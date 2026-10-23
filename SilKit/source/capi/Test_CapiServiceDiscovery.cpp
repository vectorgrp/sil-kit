// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

// Tests for the thin CAPI wrapper around the service-discovery observer. The event-translation logic
// itself is covered by Test_ServiceObserver.cpp, which drives the VSilKit::ServiceObserver directly.
// Here we verify parameter validation, that the CAPI wires the participant's internal service discovery
// through to the user handler, and the single-handler semantics of SetServiceDiscoveryHandler.

#include <string>
#include <thread>
#include <vector>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "silkit/capi/SilKit.h"

#include "core/mock/participant/MockParticipant.hpp"
#include "core/internal/ServiceDescriptor.hpp"
#include "core/internal/ServiceConfigKeys.hpp"

namespace {

using SilKit::Core::Tests::DummyParticipant;
using SilKit::Core::ServiceDescriptor;
using SilKit::Core::ServiceType;
using ServiceDiscoveryEvent = SilKit::Core::Discovery::ServiceDiscoveryEvent;
namespace Discovery = SilKit::Core::Discovery;

struct CallbackData
{
    int callCount{0};
    SilKit_Experimental_ServiceDiscoveryEvent_Type lastType{
        SilKit_Experimental_ServiceDiscoveryEvent_Type_Invalid};
    SilKit_Experimental_ServiceKind serviceKind{SilKit_Experimental_ServiceKind_Undefined};
    std::string participantName;
    std::vector<std::string> participantNames; // per call
};

void SilKitCALL CapturingHandler(void* context, SilKit_Experimental_ServiceDiscoveryEvent_Type type,
                                 const SilKit_Experimental_ServiceDescriptor* serviceDescriptor)
{
    auto* data = static_cast<CallbackData*>(context);
    data->callCount += 1;
    data->lastType = type;
    data->serviceKind = serviceDescriptor->serviceKind;
    data->participantName = serviceDescriptor->participantName;
    data->participantNames.push_back(serviceDescriptor->participantName);
}

auto MakeCanController(const std::string& participantName) -> ServiceDescriptor
{
    ServiceDescriptor descriptor;
    descriptor.SetParticipantNameAndComputeId(participantName);
    descriptor.SetServiceName("Can1");
    descriptor.SetNetworkName("CAN1");
    descriptor.SetServiceType(ServiceType::Controller);
    descriptor.SetSupplementalDataItem(Discovery::controllerType, Discovery::controllerTypeCan);
    return descriptor;
}

void SilKitCALL NoopHandler(void* /*context*/, SilKit_Experimental_ServiceDiscoveryEvent_Type /*type*/,
                            const SilKit_Experimental_ServiceDescriptor* /*serviceDescriptor*/)
{
}

struct NestedSetData
{
    SilKit_Experimental_ServiceDiscovery* serviceDiscovery{nullptr};
    std::vector<std::string> participantNames;
    std::vector<SilKit_ReturnCode> nestedResults;
};

// Records the event, then tries to set another handler from within the handler.
void SilKitCALL NestedSettingHandler(void* context, SilKit_Experimental_ServiceDiscoveryEvent_Type /*type*/,
                                     const SilKit_Experimental_ServiceDescriptor* serviceDescriptor)
{
    auto* data = static_cast<NestedSetData*>(context);
    data->participantNames.push_back(serviceDescriptor->participantName);
    data->nestedResults.push_back(
        SilKit_Experimental_ServiceDiscovery_SetServiceDiscoveryHandler(data->serviceDiscovery, data, &NoopHandler));
}

class Test_CapiServiceDiscovery : public testing::Test
{
public:
    DummyParticipant mockParticipant;
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

// The CAPI must register a handler on the participant's internal service discovery and route the
// events it delivers through to the user's C handler (via a ServiceObserver).
TEST_F(Test_CapiServiceDiscovery, wires_internal_events_to_user_handler)
{
    SilKit::Core::Discovery::ServiceDiscoveryHandler internalHandler;
    EXPECT_CALL(mockParticipant.mockServiceDiscovery, RegisterServiceDiscoveryHandler(testing::_))
        .WillOnce(testing::SaveArg<0>(&internalHandler));

    SilKit_Experimental_ServiceDiscovery* serviceDiscovery = nullptr;
    ASSERT_EQ(SilKit_Experimental_ServiceDiscovery_Create(&serviceDiscovery, (SilKit_Participant*)&mockParticipant),
              SilKit_ReturnCode_SUCCESS);

    CallbackData data;
    ASSERT_EQ(SilKit_Experimental_ServiceDiscovery_SetServiceDiscoveryHandler(serviceDiscovery, &data,
                                                                              &CapturingHandler),
              SilKit_ReturnCode_SUCCESS);
    ASSERT_TRUE(static_cast<bool>(internalHandler));

    internalHandler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeCanController("ParticipantA"));

    EXPECT_EQ(data.callCount, 1);
    EXPECT_EQ(data.lastType, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated);
    EXPECT_EQ(data.serviceKind, SilKit_Experimental_ServiceKind_CanController);
    EXPECT_EQ(data.participantName, "ParticipantA");
}

// Services replayed by the internal service discovery during the registration reach the user handler before
// SetServiceDiscoveryHandler returns; events delivered later, from another thread or from the registering thread
// after the call returned, are forwarded as well.
TEST_F(Test_CapiServiceDiscovery, replays_known_services_before_set_returns)
{
    CallbackData data;
    size_t callsDuringRegistration{0};

    SilKit::Core::Discovery::ServiceDiscoveryHandler internalHandler;
    EXPECT_CALL(mockParticipant.mockServiceDiscovery, RegisterServiceDiscoveryHandler(testing::_))
        .WillOnce([&](SilKit::Core::Discovery::ServiceDiscoveryHandler handler) {
        // Replay of the already known services, as the internal service discovery does.
        handler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeCanController("Known1"));
        handler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeCanController("Known2"));
        callsDuringRegistration = data.participantNames.size();
        internalHandler = std::move(handler);
    });

    SilKit_Experimental_ServiceDiscovery* serviceDiscovery = nullptr;
    ASSERT_EQ(SilKit_Experimental_ServiceDiscovery_Create(&serviceDiscovery, (SilKit_Participant*)&mockParticipant),
              SilKit_ReturnCode_SUCCESS);

    ASSERT_EQ(SilKit_Experimental_ServiceDiscovery_SetServiceDiscoveryHandler(serviceDiscovery, &data,
                                                                              &CapturingHandler),
              SilKit_ReturnCode_SUCCESS);
    EXPECT_EQ(callsDuringRegistration, 2u);

    std::thread ioThread{[&] {
        internalHandler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeCanController("Live1"));
    }};
    ioThread.join();
    internalHandler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeCanController("Live2"));

    EXPECT_THAT(data.participantNames, testing::ElementsAre("Known1", "Known2", "Live1", "Live2"));
}

// A participant has a single handler: a second call replaces handler and context without registering again and
// without replaying the known services.
TEST_F(Test_CapiServiceDiscovery, set_twice_replaces_handler_and_registers_once)
{
    SilKit::Core::Discovery::ServiceDiscoveryHandler internalHandler;
    EXPECT_CALL(mockParticipant.mockServiceDiscovery, RegisterServiceDiscoveryHandler(testing::_))
        .WillOnce([&internalHandler](SilKit::Core::Discovery::ServiceDiscoveryHandler handler) {
        handler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeCanController("Known1"));
        internalHandler = std::move(handler);
    });

    SilKit_Experimental_ServiceDiscovery* serviceDiscovery = nullptr;
    ASSERT_EQ(SilKit_Experimental_ServiceDiscovery_Create(&serviceDiscovery, (SilKit_Participant*)&mockParticipant),
              SilKit_ReturnCode_SUCCESS);

    CallbackData first;
    CallbackData second;
    ASSERT_EQ(SilKit_Experimental_ServiceDiscovery_SetServiceDiscoveryHandler(serviceDiscovery, &first,
                                                                              &CapturingHandler),
              SilKit_ReturnCode_SUCCESS);
    ASSERT_EQ(SilKit_Experimental_ServiceDiscovery_SetServiceDiscoveryHandler(serviceDiscovery, &second,
                                                                              &CapturingHandler),
              SilKit_ReturnCode_SUCCESS);
    EXPECT_TRUE(second.participantNames.empty()) << "replacing the handler must not replay the known services";

    internalHandler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeCanController("Live1"));

    EXPECT_THAT(first.participantNames, testing::ElementsAre("Known1"));
    EXPECT_THAT(second.participantNames, testing::ElementsAre("Live1"));
}

// Setting the handler from within the handler is not supported, also not while the known services are replayed.
TEST_F(Test_CapiServiceDiscovery, set_within_replay_is_rejected)
{
    SilKit::Core::Discovery::ServiceDiscoveryHandler internalHandler;
    EXPECT_CALL(mockParticipant.mockServiceDiscovery, RegisterServiceDiscoveryHandler(testing::_))
        .WillOnce([&internalHandler](SilKit::Core::Discovery::ServiceDiscoveryHandler handler) {
        handler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeCanController("Known1"));
        handler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeCanController("Known2"));
        internalHandler = std::move(handler);
    });

    NestedSetData data;
    ASSERT_EQ(
        SilKit_Experimental_ServiceDiscovery_Create(&data.serviceDiscovery, (SilKit_Participant*)&mockParticipant),
        SilKit_ReturnCode_SUCCESS);
    ASSERT_EQ(SilKit_Experimental_ServiceDiscovery_SetServiceDiscoveryHandler(data.serviceDiscovery, &data,
                                                                              &NestedSettingHandler),
              SilKit_ReturnCode_SUCCESS);

    EXPECT_THAT(data.participantNames, testing::ElementsAre("Known1", "Known2"));
    EXPECT_THAT(data.nestedResults,
                testing::ElementsAre(SilKit_ReturnCode_WRONGSTATE, SilKit_ReturnCode_WRONGSTATE));
}

// Setting the handler from within the handler is not supported; the handler stays in place.
TEST_F(Test_CapiServiceDiscovery, set_within_live_event_is_rejected)
{
    SilKit::Core::Discovery::ServiceDiscoveryHandler internalHandler;
    EXPECT_CALL(mockParticipant.mockServiceDiscovery, RegisterServiceDiscoveryHandler(testing::_))
        .WillOnce(testing::SaveArg<0>(&internalHandler));

    NestedSetData data;
    ASSERT_EQ(
        SilKit_Experimental_ServiceDiscovery_Create(&data.serviceDiscovery, (SilKit_Participant*)&mockParticipant),
        SilKit_ReturnCode_SUCCESS);
    ASSERT_EQ(SilKit_Experimental_ServiceDiscovery_SetServiceDiscoveryHandler(data.serviceDiscovery, &data,
                                                                              &NestedSettingHandler),
              SilKit_ReturnCode_SUCCESS);

    internalHandler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeCanController("Live1"));
    internalHandler(ServiceDiscoveryEvent::Type::ServiceCreated, MakeCanController("Live2"));

    EXPECT_THAT(data.participantNames, testing::ElementsAre("Live1", "Live2"));
    EXPECT_THAT(data.nestedResults,
                testing::ElementsAre(SilKit_ReturnCode_WRONGSTATE, SilKit_ReturnCode_WRONGSTATE));
}

} // namespace
