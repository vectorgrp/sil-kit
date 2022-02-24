// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "NullConnectionComAdapter.hpp"
#include "CanControllerFacade.hpp"
#include "MockParticipantConfiguration.hpp"

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
TEST_F(ComAdapterTest, DISABLED_throw_on_empty_participant_name)
{
    EXPECT_THROW(CreateNullConnectionComAdapterImpl(ib::cfg::MockParticipantConfiguration(),"", false), ib::configuration_error);
}

// TODO: Add test to validate participantName and configured participant name
TEST_F(ComAdapterTest, DISABLED_warn_on_participant_name_mismatch)
{
   
}

TEST_F(ComAdapterTest, make_basic_controller)
{
    auto comAdapter = CreateNullConnectionComAdapterImpl(ib::cfg::MockParticipantConfiguration(), "TestParticipant", false);

    auto* canController = comAdapter->CreateCanController("CAN1");
    auto basicCanController = dynamic_cast<ib::sim::can::CanControllerFacade*>(canController);

    EXPECT_NE(basicCanController, nullptr);
}

} // anonymous namespace
