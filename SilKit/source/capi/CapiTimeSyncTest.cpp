// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "silkit/capi/SilKit.h"
#include "silkit/services/orchestration/all.hpp"

#include "MockParticipant.hpp"

namespace {
using namespace SilKit::Services::Orchestration;
using SilKit::Core::Tests::DummyParticipant;

class CapiTimeSyncTest : public testing::Test
{
protected: 
    SilKit::Core::Tests::DummyParticipant mockParticipant;

    CapiTimeSyncTest() {}

};

void SimTask(void* /*context*/, SilKit_TimeSyncService* /*timeSyncService*/, SilKit_NanosecondsTime /*now*/,
             SilKit_NanosecondsTime /*duration*/)
{
}

TEST_F(CapiTimeSyncTest, participant_state_handling_nullpointer_params)
{
    SilKit_ReturnCode returnCode;

    returnCode = SilKit_TimeSyncService_SetSimulationStepHandler(nullptr, nullptr, &SimTask, 1000000);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_TimeSyncService_SetSimulationStepHandler(
        (SilKit_TimeSyncService*)(mockParticipant.CreateLifecycleServiceWithTimeSync()->GetTimeSyncService()), nullptr,
        nullptr, 0);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
}

TEST_F(CapiTimeSyncTest, participant_state_handling_function_mapping)
{
    SilKit_ReturnCode returnCode;

    EXPECT_CALL(mockParticipant.mockTimeSyncService,
                SetSimulationStepHandler(
            testing::Matcher<SilKit::Services::Orchestration::ITimeSyncService::SimulationStepT>(testing::_), testing ::_)
    ).Times(testing::Exactly(1));
    returnCode = SilKit_TimeSyncService_SetSimulationStepHandler(
        (SilKit_TimeSyncService*)(mockParticipant.CreateLifecycleServiceWithTimeSync()->GetTimeSyncService()), nullptr,
        &SimTask, 1000000);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
}

} //namespace