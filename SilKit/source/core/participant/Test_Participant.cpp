// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <chrono>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "NullConnectionParticipant.hpp"
#include "CanController.hpp"
#include "ConfigurationTestUtils.hpp"
#include "ParticipantConfiguration.hpp"

namespace {

using namespace SilKit::Core;
using namespace SilKit::Config;

class Test_Participant : public testing::Test
{
protected:
    Test_Participant() {}
};

TEST_F(Test_Participant, throw_on_empty_participant_name)
{
    EXPECT_THROW(CreateNullConnectionParticipantImpl(SilKit::Config::MakeEmptyParticipantConfigurationImpl(), ""),
                 SilKit::ConfigurationError);
}

TEST_F(Test_Participant, support_nullptr_in_IParticipantConfiguration)
{
    EXPECT_NO_THROW(CreateNullConnectionParticipantImpl(nullptr, "TestParticipant"));
    EXPECT_NO_THROW(CreateNullConnectionParticipantImpl(SilKit::Config::MakeEmptyParticipantConfigurationImpl(),
                                                        "TestParticipant"));
}

TEST_F(Test_Participant, use_configured_name_on_participant_name_mismatch)
{
    const auto configuredParticipantName = "ConfiguredParticipantName";
    auto mockConfig = std::make_shared<SilKit::Config::ParticipantConfiguration>();
    mockConfig->participantName = configuredParticipantName;


    auto participant = CreateNullConnectionParticipantImpl(mockConfig, "TestParticipant");
    auto comParticipantName = participant->GetParticipantName();
    EXPECT_EQ(participant->GetParticipantName(), configuredParticipantName);
}

TEST_F(Test_Participant, make_basic_controller)
{
    auto participant =
        CreateNullConnectionParticipantImpl(SilKit::Config::MakeEmptyParticipantConfigurationImpl(), "TestParticipant");

    auto* canController = participant->CreateCanController("CAN1", "CAN1");
    auto basicCanController = dynamic_cast<SilKit::Services::Can::CanController*>(canController);

    EXPECT_NE(basicCanController, nullptr);
}

TEST_F(Test_Participant, error_on_create_system_monitor_twice)
{
    auto participant =
        CreateNullConnectionParticipantImpl(SilKit::Config::MakeEmptyParticipantConfigurationImpl(), "TestParticipant");

    // ignore returned controller
    EXPECT_NO_THROW(participant->CreateSystemMonitor());

    // ignore returned controller
    EXPECT_THROW(participant->CreateSystemMonitor(), SilKit::SilKitError);
}

TEST_F(Test_Participant, error_on_create_lifecycle_service_twice)
{
    auto participant =
        CreateNullConnectionParticipantImpl(SilKit::Config::MakeEmptyParticipantConfigurationImpl(), "TestParticipant");

    // ignore returned controller
    EXPECT_NO_THROW(participant->CreateLifecycleService({SilKit::Services::Orchestration::OperationMode::Coordinated}));

    // ignore returned controller
    EXPECT_THROW(participant->CreateLifecycleService({SilKit::Services::Orchestration::OperationMode::Coordinated}),
                 SilKit::SilKitError);
}

TEST_F(Test_Participant, no_error_on_get_logger_twice)
{
    auto participant =
        CreateNullConnectionParticipantImpl(SilKit::Config::MakeEmptyParticipantConfigurationImpl(), "TestParticipant");

    EXPECT_NO_THROW({
        participant->GetLogger();
        participant->GetLogger();
    });
}

TEST_F(Test_Participant, error_on_create_basic_controller_twice)
{
    auto participant =
        CreateNullConnectionParticipantImpl(SilKit::Config::MakeEmptyParticipantConfigurationImpl(), "TestParticipant");

    participant->CreateCanController("CAN1", "CAN1");
    EXPECT_THROW(participant->CreateCanController("CAN1", "CAN1"), SilKit::ConfigurationError);
}


TEST_F(Test_Participant, error_on_create_basic_controller_twice_different_network)
{
    auto participant =
        CreateNullConnectionParticipantImpl(SilKit::Config::MakeEmptyParticipantConfigurationImpl(), "TestParticipant");

    participant->CreateCanController("CAN1", "CAN1");
    EXPECT_THROW(participant->CreateCanController("CAN1", "CAN2"), SilKit::ConfigurationError);
}

} // anonymous namespace
