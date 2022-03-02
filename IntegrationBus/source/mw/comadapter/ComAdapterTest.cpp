// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "NullConnectionComAdapter.hpp"
#include "CanControllerFacade.hpp"
#include "MockParticipantConfiguration.hpp"
#include "ParticipantConfiguration.hpp"

namespace {

using namespace ib::mw;
using namespace ib::cfg;

class ComAdapterTest : public testing::Test
{
protected:
    ComAdapterTest()
    {
    }
};

// TODO: Fix
TEST_F(ComAdapterTest, throw_on_empty_participant_name)
{
    EXPECT_THROW(CreateNullConnectionComAdapterImpl(ib::cfg::MockParticipantConfiguration(), "", false),
                 ib::ConfigurationError);
}

TEST_F(ComAdapterTest, support_nullptr_in_IParticipantConfiguration)
{
    EXPECT_NO_THROW(CreateNullConnectionComAdapterImpl(nullptr, "TestParticipant", false));
    EXPECT_NO_THROW(CreateNullConnectionComAdapterImpl(std::make_shared<ib::cfg::IParticipantConfiguration>(),
                                                       "TestParticipant", false));
}

TEST_F(ComAdapterTest, use_configured_name_on_participant_name_mismatch)
{
    const auto configuredParticipantName = "ConfiguredParticipantName";
    auto mockConfig =
        std::make_shared<ib::cfg::datatypes::ParticipantConfiguration>(ib::cfg::datatypes::ParticipantConfiguration());
    mockConfig->participantName = configuredParticipantName;


    auto comAdapter =
        CreateNullConnectionComAdapterImpl(mockConfig, "TestParticipant", false);
    auto comParticipantName = comAdapter->GetParticipantName();
    EXPECT_EQ(comAdapter->GetParticipantName(), configuredParticipantName);
}

TEST_F(ComAdapterTest, make_basic_controller)
{
    auto comAdapter =
        CreateNullConnectionComAdapterImpl(ib::cfg::MockParticipantConfiguration(), "TestParticipant", false);

    auto* canController = comAdapter->CreateCanController("CAN1");
    auto basicCanController = dynamic_cast<ib::sim::can::CanControllerFacade*>(canController);

    EXPECT_NE(basicCanController, nullptr);
}

} // anonymous namespace
