/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "silkit/capi/SilKit.h"
#include "silkit/services/orchestration/all.hpp"

#include "MockParticipant.hpp"

namespace {
using namespace SilKit::Services::Orchestration;
using SilKit::Core::Tests::DummyParticipant;

class Test_CapiTimeSync : public testing::Test
{
protected: 
    SilKit::Core::Tests::DummyParticipant mockParticipant;

    Test_CapiTimeSync() {}

};

void SilKitCALL SimTask(void* /*context*/, SilKit_TimeSyncService* /*timeSyncService*/, SilKit_NanosecondsTime /*now*/,
             SilKit_NanosecondsTime /*duration*/)
{
}

TEST_F(Test_CapiTimeSync, participant_state_handling_nullpointer_params)
{
    SilKit_ReturnCode returnCode;

    returnCode = SilKit_TimeSyncService_SetSimulationStepHandler(nullptr, nullptr, &SimTask, 1000000);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_TimeSyncService_SetSimulationStepHandler(
        (SilKit_TimeSyncService*)(mockParticipant
                                      .CreateLifecycleService(LifecycleConfiguration{OperationMode::Coordinated})
                                      ->CreateTimeSyncService()),
        nullptr,
        nullptr, 0);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
}

TEST_F(Test_CapiTimeSync, participant_state_handling_function_mapping)
{
    SilKit_ReturnCode returnCode;

    EXPECT_CALL(mockParticipant.mockTimeSyncService,
                SetSimulationStepHandler(
            testing::Matcher<SilKit::Services::Orchestration::ITimeSyncService::SimulationStepHandler>(testing::_), testing ::_)
    ).Times(testing::Exactly(1));
    returnCode = SilKit_TimeSyncService_SetSimulationStepHandler(
        (SilKit_TimeSyncService*)(mockParticipant
                                      .CreateLifecycleService(LifecycleConfiguration{OperationMode::Coordinated})
                                      ->CreateTimeSyncService()),
        nullptr,
        &SimTask, 1000000);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
}

} //namespace
