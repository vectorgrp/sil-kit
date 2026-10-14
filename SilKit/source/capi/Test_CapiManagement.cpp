// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

// Parameter validation of the management client CAPI. The conversion of the events is covered end-to-end by
// ITest_Management.cpp.

#include "gtest/gtest.h"

#include "silkit/capi/SilKit.h"

namespace {

void SilKitCALL RegistryInfoHandler(void*, SilKit_Experimental_ManagementClient*,
                                    const SilKit_Experimental_ManagementRegistryInfo*)
{
}

void SilKitCALL SimulationsHandler(void*, SilKit_Experimental_ManagementClient*,
                                   const SilKit_Experimental_ManagementSimulationsEvent*)
{
}

void SilKitCALL ParticipantConfigurationHandler(void*, SilKit_Experimental_ManagementClient*, SilKit_RpcCallStatus,
                                                const SilKit_Experimental_ManagementParticipantConfigurationReply*)
{
}

TEST(Test_CapiManagement, bad_parameters)
{
    auto* client = reinterpret_cast<SilKit_Experimental_ManagementClient*>(uintptr_t{0x1234});
    auto* configuration = reinterpret_cast<SilKit_ParticipantConfiguration*>(uintptr_t{0x1234});
    SilKit_Experimental_ManagementClient* outClient{nullptr};

    EXPECT_EQ(SilKit_Experimental_ManagementClient_Create(nullptr, configuration, "silkit://localhost:8500"),
              SilKit_ReturnCode_BADPARAMETER);
    EXPECT_EQ(SilKit_Experimental_ManagementClient_Create(&outClient, nullptr, "silkit://localhost:8500"),
              SilKit_ReturnCode_BADPARAMETER);
    EXPECT_EQ(SilKit_Experimental_ManagementClient_Create(&outClient, configuration, nullptr),
              SilKit_ReturnCode_BADPARAMETER);

    EXPECT_EQ(SilKit_Experimental_ManagementClient_Destroy(nullptr), SilKit_ReturnCode_BADPARAMETER);

    EXPECT_EQ(SilKit_Experimental_ManagementClient_SetRegistryInfoHandler(nullptr, nullptr, &RegistryInfoHandler),
              SilKit_ReturnCode_BADPARAMETER);
    EXPECT_EQ(SilKit_Experimental_ManagementClient_SetRegistryInfoHandler(client, nullptr, nullptr),
              SilKit_ReturnCode_BADPARAMETER);

    EXPECT_EQ(SilKit_Experimental_ManagementClient_SetSimulationsHandler(nullptr, nullptr, &SimulationsHandler),
              SilKit_ReturnCode_BADPARAMETER);
    EXPECT_EQ(SilKit_Experimental_ManagementClient_SetSimulationsHandler(client, nullptr, nullptr),
              SilKit_ReturnCode_BADPARAMETER);

    EXPECT_EQ(SilKit_Experimental_ManagementClient_GetParticipantConfiguration(nullptr, "a", "A1", 0, nullptr,
                                                                               &ParticipantConfigurationHandler),
              SilKit_ReturnCode_BADPARAMETER);
    EXPECT_EQ(SilKit_Experimental_ManagementClient_GetParticipantConfiguration(client, nullptr, "A1", 0, nullptr,
                                                                               &ParticipantConfigurationHandler),
              SilKit_ReturnCode_BADPARAMETER);
    EXPECT_EQ(SilKit_Experimental_ManagementClient_GetParticipantConfiguration(client, "a", nullptr, 0, nullptr,
                                                                               &ParticipantConfigurationHandler),
              SilKit_ReturnCode_BADPARAMETER);
    EXPECT_EQ(SilKit_Experimental_ManagementClient_GetParticipantConfiguration(client, "a", "A1", 0, nullptr, nullptr),
              SilKit_ReturnCode_BADPARAMETER);
}

} // namespace
