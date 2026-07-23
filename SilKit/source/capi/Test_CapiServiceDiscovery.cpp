// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

// Tests for the thin CAPI wrapper around the service-discovery observer. The event-translation logic
// itself is covered by Test_ServiceObserver.cpp, which drives the VSilKit::ServiceObserver directly.
// Here we only verify parameter validation and that the CAPI wires the participant's internal service
// discovery through to the user handler.

#include <string>

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
};

void SilKitCALL CapturingHandler(void* context, SilKit_Experimental_ServiceDiscoveryEvent_Type type,
                                 const SilKit_Experimental_ServiceDescriptor* serviceDescriptor)
{
    auto* data = static_cast<CallbackData*>(context);
    data->callCount += 1;
    data->lastType = type;
    data->serviceKind = serviceDescriptor->serviceKind;
    data->participantName = serviceDescriptor->participantName;
}

void SilKitCALL NoopHandler(void* /*context*/, SilKit_Experimental_ServiceDiscoveryEvent_Type /*type*/,
                            const SilKit_Experimental_ServiceDescriptor* /*serviceDescriptor*/)
{
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

    ServiceDescriptor descriptor;
    descriptor.SetParticipantNameAndComputeId("ParticipantA");
    descriptor.SetServiceName("Can1");
    descriptor.SetNetworkName("CAN1");
    descriptor.SetServiceType(ServiceType::Controller);
    descriptor.SetSupplementalDataItem(Discovery::controllerType, Discovery::controllerTypeCan);

    internalHandler(ServiceDiscoveryEvent::Type::ServiceCreated, descriptor);

    EXPECT_EQ(data.callCount, 1);
    EXPECT_EQ(data.lastType, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated);
    EXPECT_EQ(data.serviceKind, SilKit_Experimental_ServiceKind_CanController);
    EXPECT_EQ(data.participantName, "ParticipantA");
}

} // namespace
