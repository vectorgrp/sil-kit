// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "silkit/capi/SilKit.h"

#include "core/mock/participant/MockParticipant.hpp"
#include "core/internal/ServiceDescriptor.hpp"

namespace {

using SilKit::Core::Tests::DummyParticipant;
using ServiceDiscoveryEvent = SilKit::Core::Discovery::ServiceDiscoveryEvent;

// Captures, inside the handler invocation, everything the C accessors return. Copying into owned strings here also
// verifies that the descriptor handle and the returned pointers are valid for the duration of the callback.
struct CallbackData
{
    int callCount{0};
    SilKit_Experimental_ServiceDiscoveryEvent_Type lastType{
        SilKit_Experimental_ServiceDiscoveryEvent_Type_Invalid};
    std::string participantName;
    std::string serviceName;
    std::string networkName;
    SilKit_Experimental_ServiceType serviceType{SilKit_Experimental_ServiceType_Undefined};
    bool hasTopic{false};
    std::string topic;
};

void SilKitCALL TestDiscoveryHandler(void* context, SilKit_Experimental_ServiceDiscoveryEvent_Type type,
                                     const SilKit_Experimental_ServiceDescriptor* serviceDescriptor)
{
    auto* data = static_cast<CallbackData*>(context);
    data->callCount += 1;
    data->lastType = type;

    const char* str = nullptr;
    EXPECT_EQ(SilKit_Experimental_ServiceDescriptor_GetParticipantName(serviceDescriptor, &str),
              SilKit_ReturnCode_SUCCESS);
    data->participantName = str;
    EXPECT_EQ(SilKit_Experimental_ServiceDescriptor_GetServiceName(serviceDescriptor, &str),
              SilKit_ReturnCode_SUCCESS);
    data->serviceName = str;
    EXPECT_EQ(SilKit_Experimental_ServiceDescriptor_GetNetworkName(serviceDescriptor, &str),
              SilKit_ReturnCode_SUCCESS);
    data->networkName = str;
    EXPECT_EQ(SilKit_Experimental_ServiceDescriptor_GetServiceType(serviceDescriptor, &data->serviceType),
              SilKit_ReturnCode_SUCCESS);

    SilKit_Bool hasValue = SilKit_False;
    const char* value = nullptr;
    EXPECT_EQ(
        SilKit_Experimental_ServiceDescriptor_GetSupplementalDataItem(serviceDescriptor, "topic", &value, &hasValue),
        SilKit_ReturnCode_SUCCESS);
    data->hasTopic = (hasValue == SilKit_True);
    if (data->hasTopic)
    {
        data->topic = value;
    }
}

void SilKitCALL NoopDiscoveryHandler(void* /*context*/, SilKit_Experimental_ServiceDiscoveryEvent_Type /*type*/,
                                     const SilKit_Experimental_ServiceDescriptor* /*serviceDescriptor*/)
{
}

class Test_CapiServiceDiscovery : public testing::Test
{
public:
    DummyParticipant mockParticipant;

    // Build a populated descriptor for the accessor tests.
    static SilKit::Core::ServiceDescriptor MakeDescriptor()
    {
        SilKit::Core::ServiceDescriptor descriptor;
        descriptor.SetParticipantNameAndComputeId("ParticipantA");
        descriptor.SetServiceName("MyPublisher");
        descriptor.SetNetworkName("NetworkX");
        descriptor.SetServiceType(SilKit::Core::ServiceType::Controller);
        descriptor.SetSupplementalDataItem("topic", "TopicA");
        return descriptor;
    }

    static const SilKit_Experimental_ServiceDescriptor* AsC(const SilKit::Core::ServiceDescriptor& descriptor)
    {
        return reinterpret_cast<const SilKit_Experimental_ServiceDescriptor*>(&descriptor);
    }
};

TEST_F(Test_CapiServiceDiscovery, create_returns_service_discovery)
{
    SilKit_Experimental_ServiceDiscovery* serviceDiscovery = nullptr;
    const auto returnCode =
        SilKit_Experimental_ServiceDiscovery_Create(&serviceDiscovery, (SilKit_Participant*)&mockParticipant);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
    EXPECT_NE(serviceDiscovery, nullptr);
}

TEST_F(Test_CapiServiceDiscovery, create_bad_parameters)
{
    SilKit_Experimental_ServiceDiscovery* serviceDiscovery = nullptr;

    EXPECT_EQ(SilKit_Experimental_ServiceDiscovery_Create(nullptr, (SilKit_Participant*)&mockParticipant),
              SilKit_ReturnCode_BADPARAMETER);
    EXPECT_EQ(SilKit_Experimental_ServiceDiscovery_Create(&serviceDiscovery, nullptr),
              SilKit_ReturnCode_BADPARAMETER);
}

TEST_F(Test_CapiServiceDiscovery, set_handler_bad_parameters)
{
    SilKit_Experimental_ServiceDiscovery* serviceDiscovery = nullptr;
    ASSERT_EQ(SilKit_Experimental_ServiceDiscovery_Create(&serviceDiscovery, (SilKit_Participant*)&mockParticipant),
              SilKit_ReturnCode_SUCCESS);

    EXPECT_EQ(SilKit_Experimental_ServiceDiscovery_SetServiceDiscoveryHandler(nullptr, nullptr, &NoopDiscoveryHandler),
              SilKit_ReturnCode_BADPARAMETER);
    EXPECT_EQ(SilKit_Experimental_ServiceDiscovery_SetServiceDiscoveryHandler(serviceDiscovery, nullptr, nullptr),
              SilKit_ReturnCode_BADPARAMETER);
}

TEST_F(Test_CapiServiceDiscovery, set_handler_registers_and_forwards_events)
{
    SilKit::Core::Discovery::ServiceDiscoveryHandler capturedHandler;
    EXPECT_CALL(mockParticipant.mockServiceDiscovery, RegisterServiceDiscoveryHandler(testing::_))
        .WillOnce(testing::SaveArg<0>(&capturedHandler));

    SilKit_Experimental_ServiceDiscovery* serviceDiscovery = nullptr;
    ASSERT_EQ(SilKit_Experimental_ServiceDiscovery_Create(&serviceDiscovery, (SilKit_Participant*)&mockParticipant),
              SilKit_ReturnCode_SUCCESS);

    CallbackData data;
    ASSERT_EQ(SilKit_Experimental_ServiceDiscovery_SetServiceDiscoveryHandler(serviceDiscovery, &data,
                                                                              &TestDiscoveryHandler),
              SilKit_ReturnCode_SUCCESS);
    ASSERT_TRUE(static_cast<bool>(capturedHandler));

    const auto descriptor = MakeDescriptor();

    capturedHandler(ServiceDiscoveryEvent::Type::ServiceCreated, descriptor);
    EXPECT_EQ(data.callCount, 1);
    EXPECT_EQ(data.lastType, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated);
    EXPECT_EQ(data.participantName, "ParticipantA");
    EXPECT_EQ(data.serviceName, "MyPublisher");
    EXPECT_EQ(data.networkName, "NetworkX");
    EXPECT_EQ(data.serviceType, SilKit_Experimental_ServiceType_Controller);
    EXPECT_TRUE(data.hasTopic);
    EXPECT_EQ(data.topic, "TopicA");

    capturedHandler(ServiceDiscoveryEvent::Type::ServiceRemoved, descriptor);
    EXPECT_EQ(data.callCount, 2);
    EXPECT_EQ(data.lastType, SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceRemoved);
}

TEST_F(Test_CapiServiceDiscovery, descriptor_accessors)
{
    const auto descriptor = MakeDescriptor();
    const auto* cDescriptor = AsC(descriptor);

    const char* str = nullptr;
    EXPECT_EQ(SilKit_Experimental_ServiceDescriptor_GetParticipantName(cDescriptor, &str), SilKit_ReturnCode_SUCCESS);
    EXPECT_STREQ(str, "ParticipantA");
    EXPECT_EQ(SilKit_Experimental_ServiceDescriptor_GetServiceName(cDescriptor, &str), SilKit_ReturnCode_SUCCESS);
    EXPECT_STREQ(str, "MyPublisher");
    EXPECT_EQ(SilKit_Experimental_ServiceDescriptor_GetNetworkName(cDescriptor, &str), SilKit_ReturnCode_SUCCESS);
    EXPECT_STREQ(str, "NetworkX");

    SilKit_Experimental_ServiceType serviceType = SilKit_Experimental_ServiceType_Undefined;
    EXPECT_EQ(SilKit_Experimental_ServiceDescriptor_GetServiceType(cDescriptor, &serviceType),
              SilKit_ReturnCode_SUCCESS);
    EXPECT_EQ(serviceType, SilKit_Experimental_ServiceType_Controller);

    SilKit_Bool hasValue = SilKit_False;
    const char* value = nullptr;
    EXPECT_EQ(SilKit_Experimental_ServiceDescriptor_GetSupplementalDataItem(cDescriptor, "topic", &value, &hasValue),
              SilKit_ReturnCode_SUCCESS);
    EXPECT_EQ(hasValue, SilKit_True);
    EXPECT_STREQ(value, "TopicA");

    // Absent key is not an error: SUCCESS with hasValue == SilKit_False and value == nullptr.
    EXPECT_EQ(
        SilKit_Experimental_ServiceDescriptor_GetSupplementalDataItem(cDescriptor, "missing", &value, &hasValue),
        SilKit_ReturnCode_SUCCESS);
    EXPECT_EQ(hasValue, SilKit_False);
    EXPECT_EQ(value, nullptr);
}

TEST_F(Test_CapiServiceDiscovery, descriptor_accessors_bad_parameters)
{
    const auto descriptor = MakeDescriptor();
    const auto* cDescriptor = AsC(descriptor);

    const char* str = nullptr;
    SilKit_Experimental_ServiceType serviceType = SilKit_Experimental_ServiceType_Undefined;
    SilKit_Bool hasValue = SilKit_False;
    const char* value = nullptr;

    EXPECT_EQ(SilKit_Experimental_ServiceDescriptor_GetParticipantName(nullptr, &str), SilKit_ReturnCode_BADPARAMETER);
    EXPECT_EQ(SilKit_Experimental_ServiceDescriptor_GetParticipantName(cDescriptor, nullptr),
              SilKit_ReturnCode_BADPARAMETER);
    EXPECT_EQ(SilKit_Experimental_ServiceDescriptor_GetServiceName(nullptr, &str), SilKit_ReturnCode_BADPARAMETER);
    EXPECT_EQ(SilKit_Experimental_ServiceDescriptor_GetNetworkName(nullptr, &str), SilKit_ReturnCode_BADPARAMETER);
    EXPECT_EQ(SilKit_Experimental_ServiceDescriptor_GetServiceType(nullptr, &serviceType),
              SilKit_ReturnCode_BADPARAMETER);
    EXPECT_EQ(SilKit_Experimental_ServiceDescriptor_GetServiceType(cDescriptor, nullptr),
              SilKit_ReturnCode_BADPARAMETER);

    EXPECT_EQ(SilKit_Experimental_ServiceDescriptor_GetSupplementalDataItem(nullptr, "topic", &value, &hasValue),
              SilKit_ReturnCode_BADPARAMETER);
    EXPECT_EQ(SilKit_Experimental_ServiceDescriptor_GetSupplementalDataItem(cDescriptor, nullptr, &value, &hasValue),
              SilKit_ReturnCode_BADPARAMETER);
    EXPECT_EQ(SilKit_Experimental_ServiceDescriptor_GetSupplementalDataItem(cDescriptor, "topic", nullptr, &hasValue),
              SilKit_ReturnCode_BADPARAMETER);
    EXPECT_EQ(SilKit_Experimental_ServiceDescriptor_GetSupplementalDataItem(cDescriptor, "topic", &value, nullptr),
              SilKit_ReturnCode_BADPARAMETER);
}

} // namespace
