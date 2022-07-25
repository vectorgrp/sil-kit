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

#include "Validation.hpp"

#include <chrono>
#include <functional>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "silkit/services/flexray/FlexrayDatatypes.hpp"
#include "silkit/participant/exception.hpp"

namespace {

using namespace SilKit::Services::Flexray;

TEST(TestSimFlexrayValidation, throw_if_gColdstartAttempts_is_out_of_range)
{
    FlexrayClusterParameters clusterParams;
    clusterParams.gColdstartAttempts = 0;
    EXPECT_THROW(Validate(clusterParams), SilKit::ConfigurationError);
}

TEST(TestSimFlexrayValidation, throw_if_pAllowHaltDueToClock_is_not_0_or_1)
{
    FlexrayNodeParameters nodeParams;
    nodeParams.pAllowHaltDueToClock = 2;
    EXPECT_THROW(Validate(nodeParams), SilKit::ConfigurationError);
}

TEST(TestSimFlexrayValidation, valid_cluster_params_lower_boundary_must_not_throw)
{
    FlexrayClusterParameters clusterParams;
    clusterParams.gColdstartAttempts = 2;
    clusterParams.gCycleCountMax = 7;
    clusterParams.gdActionPointOffset = 1;
    clusterParams.gdDynamicSlotIdlePhase = 0;
    clusterParams.gdMiniSlot = 2;
    clusterParams.gdMiniSlotActionPointOffset = 1;
    clusterParams.gdStaticSlot = 3;
    clusterParams.gdSymbolWindow = 0;
    clusterParams.gdSymbolWindowActionPointOffset = 1;
    clusterParams.gdTSSTransmitter = 1;
    clusterParams.gdWakeupTxActive = 15;
    clusterParams.gdWakeupTxIdle = 45;
    clusterParams.gListenNoise = 2;
    clusterParams.gMacroPerCycle = 8;
    clusterParams.gMaxWithoutClockCorrectionFatal = 1;
    clusterParams.gMaxWithoutClockCorrectionPassive = 1;
    clusterParams.gNumberOfMiniSlots = 0;
    clusterParams.gNumberOfStaticSlots = 2;
    clusterParams.gPayloadLengthStatic = 0;
    clusterParams.gSyncFrameIDCountMax = 2;
    
    EXPECT_NO_THROW(Validate(clusterParams));
}

TEST(TestSimFlexrayValidation, valid_cluster_params_upper_boundary_must_not_throw)
{
    FlexrayClusterParameters clusterParams;
    clusterParams.gColdstartAttempts = 31;
    clusterParams.gCycleCountMax = 63;
    clusterParams.gdActionPointOffset = 63;
    clusterParams.gdDynamicSlotIdlePhase = 2;
    clusterParams.gdMiniSlot = 63;
    clusterParams.gdMiniSlotActionPointOffset = 31;
    clusterParams.gdStaticSlot = 664;
    clusterParams.gdSymbolWindow = 162;
    clusterParams.gdSymbolWindowActionPointOffset = 63;
    clusterParams.gdTSSTransmitter = 15;
    clusterParams.gdWakeupTxActive = 60;
    clusterParams.gdWakeupTxIdle = 180;
    clusterParams.gListenNoise = 16;
    clusterParams.gMacroPerCycle = 16000;
    clusterParams.gMaxWithoutClockCorrectionFatal = 15;
    clusterParams.gMaxWithoutClockCorrectionPassive = 15;
    clusterParams.gNumberOfMiniSlots = 7988;
    clusterParams.gNumberOfStaticSlots = 1023;
    clusterParams.gPayloadLengthStatic = 127;
    clusterParams.gSyncFrameIDCountMax = 15;
    
    EXPECT_NO_THROW(Validate(clusterParams));
}

TEST(TestSimFlexrayValidation, valid_node_parameters_lower_boundary_must_not_throw)
{
    FlexrayNodeParameters nodeParams;
    nodeParams.pAllowHaltDueToClock = 0;
    nodeParams.pAllowPassiveToActive = 0;
    nodeParams.pChannels = FlexrayChannel::A;
    nodeParams.pClusterDriftDamping = 0;
    nodeParams.pdAcceptedStartupRange = 29;
    nodeParams.pdListenTimeout = 1926;
    nodeParams.pKeySlotId = 0;
    nodeParams.pKeySlotOnlyEnabled = 0;
    nodeParams.pKeySlotUsedForStartup = 0;
    nodeParams.pKeySlotUsedForSync = 0;
    nodeParams.pLatestTx = 0;
    nodeParams.pMacroInitialOffsetA = 2;
    nodeParams.pMacroInitialOffsetB = 2;
    nodeParams.pMicroInitialOffsetA = 0;
    nodeParams.pMicroInitialOffsetB = 0;
    nodeParams.pMicroPerCycle = 960;
    nodeParams.pOffsetCorrectionOut = 15;
    nodeParams.pOffsetCorrectionStart = 7;
    nodeParams.pRateCorrectionOut = 3;
    nodeParams.pWakeupChannel = FlexrayChannel::A;
    nodeParams.pWakeupPattern = 0;
    nodeParams.pdMicrotick = FlexrayClockPeriod::T12_5NS;
    nodeParams.pSamplesPerMicrotick = 1;

    EXPECT_NO_THROW(Validate(nodeParams));
}


TEST(TestSimFlexrayValidation, valid_node_parameters_upper_boundary_must_not_throw)
{
    FlexrayNodeParameters nodeParams;
    nodeParams.pAllowHaltDueToClock = 1;
    nodeParams.pAllowPassiveToActive = 31;
    nodeParams.pChannels = FlexrayChannel::AB;
    nodeParams.pClusterDriftDamping = 10;
    nodeParams.pdAcceptedStartupRange = 2743;
    nodeParams.pdListenTimeout = 2567692;
    nodeParams.pKeySlotId = 1023;
    nodeParams.pKeySlotOnlyEnabled = 1;
    nodeParams.pKeySlotUsedForStartup = 1;
    nodeParams.pKeySlotUsedForSync = 1;
    nodeParams.pLatestTx = 7988;
    nodeParams.pMacroInitialOffsetA = 68;
    nodeParams.pMacroInitialOffsetB = 68;
    nodeParams.pMicroInitialOffsetA = 239;
    nodeParams.pMicroInitialOffsetB = 239;
    nodeParams.pMicroPerCycle = 1280000;
    nodeParams.pOffsetCorrectionOut = 16082;
    nodeParams.pOffsetCorrectionStart = 15999;
    nodeParams.pRateCorrectionOut = 3846;
    nodeParams.pWakeupChannel = FlexrayChannel::B;
    nodeParams.pWakeupPattern = 63;
    nodeParams.pdMicrotick = FlexrayClockPeriod::T50NS;
    nodeParams.pSamplesPerMicrotick = 2;

    EXPECT_NO_THROW(Validate(nodeParams));
}

} // anonymous namespace
