// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <memory>
#include <string>

#include "gtest/gtest.h"
#include "SimTestHarness.hpp"

namespace {
using namespace SilKit::Tests;

struct ITest_GetParameter : public testing::Test
{
};


TEST_F(ITest_GetParameter, get_parameter_set_by_api)
{
    const std::string participantName = "P1"; 

    SimTestHarnessArgs args;
    args.deferParticipantCreation = true;
    args.asyncParticipantNames = {participantName};
    //args.registry.listenUri = "silkit://127.0.0.1:0"
    auto simTestHarness = std::make_unique<SimTestHarness>(args);

    auto participant = simTestHarness->GetParticipant(participantName)->Participant();
    auto participantNameByGetParameter = participant->GetParameter(SilKit::Parameter::ParticipantName);
    EXPECT_EQ(participantName, participantNameByGetParameter);
}

} //end namespace
