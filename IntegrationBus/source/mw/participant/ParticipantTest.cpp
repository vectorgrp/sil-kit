// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "NullConnectionParticipant.hpp"
#include "CanController.hpp"
#include "MockParticipantConfiguration.hpp"
#include "ParticipantConfiguration.hpp"

namespace {

using namespace ib::mw;
using namespace ib::cfg;

class ParticipantTest : public testing::Test
{
protected:
    ParticipantTest()
    {
    }
};

// TODO: Fix
TEST_F(ParticipantTest, throw_on_empty_participant_name)
{
    EXPECT_THROW(CreateNullConnectionParticipantImpl(ib::cfg::MockParticipantConfiguration(), ""),
                 ib::ConfigurationError);
}

TEST_F(ParticipantTest, support_nullptr_in_IParticipantConfiguration)
{
    EXPECT_NO_THROW(CreateNullConnectionParticipantImpl(nullptr, "TestParticipant"));
    EXPECT_NO_THROW(CreateNullConnectionParticipantImpl(std::make_shared<ib::cfg::IParticipantConfiguration>(),
                                                       "TestParticipant"));
}

TEST_F(ParticipantTest, use_configured_name_on_participant_name_mismatch)
{
    const auto configuredParticipantName = "ConfiguredParticipantName";
    auto mockConfig =
        std::make_shared<ib::cfg::ParticipantConfiguration>(ib::cfg::ParticipantConfiguration());
    mockConfig->participantName = configuredParticipantName;


    auto participant =
        CreateNullConnectionParticipantImpl(mockConfig, "TestParticipant");
    auto comParticipantName = participant->GetParticipantName();
    EXPECT_EQ(participant->GetParticipantName(), configuredParticipantName);
}

TEST_F(ParticipantTest, make_basic_controller)
{
    auto participant =
        CreateNullConnectionParticipantImpl(ib::cfg::MockParticipantConfiguration(), "TestParticipant");

    auto* canController = participant->CreateCanController("CAN1");
    auto basicCanController = dynamic_cast<ib::sim::can::CanController*>(canController);

    EXPECT_NE(basicCanController, nullptr);
}

} // anonymous namespace
