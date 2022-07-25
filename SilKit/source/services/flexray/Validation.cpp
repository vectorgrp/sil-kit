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

#include "ParticipantConfiguration.hpp"

namespace SilKit {
namespace Services {
namespace Flexray {

void Validate(const FlexrayClusterParameters& clusterParameters)
{
    if (clusterParameters.gColdstartAttempts < 2 || clusterParameters.gColdstartAttempts > 31)
        throw SilKit::ConfigurationError{"gColdstartAttempts must be in range 2 - 31"};

    if (clusterParameters.gCycleCountMax % 2 != 1)
        throw SilKit::ConfigurationError{"gCycleCountMax must be an odd value"};
        
    if (clusterParameters.gCycleCountMax < 7 || clusterParameters.gCycleCountMax > 63)
        throw SilKit::ConfigurationError{"gCycleCountMax must be in range 7, 9, ..., 63"};

    if (clusterParameters.gdActionPointOffset < 1 || clusterParameters.gdActionPointOffset > 63)
        throw SilKit::ConfigurationError{"gdActionPointOffset must be in range 1 - 63"};

    if (clusterParameters.gdDynamicSlotIdlePhase > 2)
        throw SilKit::ConfigurationError{"gdDynamicSlotIdlePhase must be in range 0 - 2"};

    if (clusterParameters.gdMiniSlot < 2 || clusterParameters.gdMiniSlot > 63)
        throw SilKit::ConfigurationError{"gdMiniSlot must be in range 2 - 63"};

    if (clusterParameters.gdMiniSlotActionPointOffset < 1 || clusterParameters.gdMiniSlotActionPointOffset > 31)
        throw SilKit::ConfigurationError{"gdMiniSlotActionPointOffset must be in range 1 - 31"};

    if (clusterParameters.gdStaticSlot < 3 || clusterParameters.gdStaticSlot > 664)
        throw SilKit::ConfigurationError{"gdStaticSlot must be in range 3 - 664"};

    if (clusterParameters.gdSymbolWindow > 162)
        throw SilKit::ConfigurationError{"gdSymbolWindow must be in range 0 - 162"};

    if (clusterParameters.gdSymbolWindowActionPointOffset < 1 || clusterParameters.gdSymbolWindowActionPointOffset > 63)
        throw SilKit::ConfigurationError{"gdSymbolWindowActionPointOffset must be in range 1 - 63"};

    if (clusterParameters.gdTSSTransmitter < 1 || clusterParameters.gdTSSTransmitter > 15)
        throw SilKit::ConfigurationError{"gdTSSTransmitter must be in range 1 - 15"};

    if (clusterParameters.gdWakeupTxActive < 15 || clusterParameters.gdWakeupTxActive > 60)
        throw SilKit::ConfigurationError{"gdWakeupTxActive must be in range 15 - 60"};

    if (clusterParameters.gdWakeupTxIdle < 45 || clusterParameters.gdWakeupTxIdle > 180)
        throw SilKit::ConfigurationError{"gdWakeupTxIdle must be in range 45 - 180"};

    if (clusterParameters.gListenNoise < 2 || clusterParameters.gListenNoise > 16)
        throw SilKit::ConfigurationError{"gListenNoise must be in range 2 - 16"};

    if (clusterParameters.gMacroPerCycle < 8 || clusterParameters.gMacroPerCycle > 16000)
        throw SilKit::ConfigurationError{"gMacroPerCycle must be in range 8 - 16000"};

    if (clusterParameters.gMaxWithoutClockCorrectionFatal < 1 || clusterParameters.gMaxWithoutClockCorrectionFatal > 15)
        throw SilKit::ConfigurationError{"gMaxWithoutClockCorrectionFatal must be in range 1 - 15"};

    if (clusterParameters.gMaxWithoutClockCorrectionPassive < 1 || clusterParameters.gMaxWithoutClockCorrectionPassive > 15)
        throw SilKit::ConfigurationError{"gMaxWithoutClockCorrectionPassive must be in range 1 - 15"};

    if (clusterParameters.gNumberOfMiniSlots > 7988)
        throw SilKit::ConfigurationError{"gNumberOfMiniSlots must be in range 0 - 7988"};

    if (clusterParameters.gNumberOfStaticSlots < 2 || clusterParameters.gNumberOfStaticSlots > 1023)
        throw SilKit::ConfigurationError{"gNumberOfStaticSlots must be in range 2 - 1023"};

    if (clusterParameters.gPayloadLengthStatic > 127)
        throw SilKit::ConfigurationError{"gPayloadLengthStatic must be in range 0 - 127"};

    if (clusterParameters.gSyncFrameIDCountMax < 2 || clusterParameters.gSyncFrameIDCountMax > 15)
        throw SilKit::ConfigurationError{"gSyncFrameIDCountMax must be in range 2 - 15"};
}

void Validate(const FlexrayNodeParameters& nodeParameters)
{
    if (nodeParameters.pAllowHaltDueToClock > 1)
        throw SilKit::ConfigurationError{"pAllowHaltDueToClock must be 0 or 1"};

    if (nodeParameters.pAllowPassiveToActive > 31)
        throw SilKit::ConfigurationError{"pAllowPassiveToActive must be in range 0 - 31"};

    if (nodeParameters.pClusterDriftDamping > 10)
        throw SilKit::ConfigurationError{"pClusterDriftDamping must be in range 0 - 10"};

    if (nodeParameters.pdAcceptedStartupRange < 29 || nodeParameters.pdAcceptedStartupRange > 2743)
        throw SilKit::ConfigurationError{"pdAcceptedStartupRange must be in range 29 - 2743"};

    if (nodeParameters.pdListenTimeout < 1926 || nodeParameters.pdListenTimeout > 2567692)
        throw SilKit::ConfigurationError{"pdListenTimeout must be in range 1926 - 2567692"};

    if (nodeParameters.pKeySlotId > 1023)
        throw SilKit::ConfigurationError{"pKeySlotId must be in range 0 - 1023"};

    if (nodeParameters.pKeySlotOnlyEnabled > 1)
        throw SilKit::ConfigurationError{"pKeySlotOnlyEnabled must be 0 or 1"};

    if (nodeParameters.pKeySlotUsedForStartup > 1)
        throw SilKit::ConfigurationError{"pKeySlotUsedForStartup must be 0 or 1"};

    if (nodeParameters.pKeySlotUsedForSync > 1)
        throw SilKit::ConfigurationError{"pKeySlotUsedForSync must be 0 or 1"};

    if (nodeParameters.pLatestTx > 7988)
        throw SilKit::ConfigurationError{"pLatestTx must be in range 0 - 7988"};

    if (nodeParameters.pMacroInitialOffsetA < 2 || nodeParameters.pMacroInitialOffsetA > 68)
        throw SilKit::ConfigurationError{"pMacroInitialOffsetA must be in range 2 - 68"};

    if (nodeParameters.pMacroInitialOffsetB < 2 || nodeParameters.pMacroInitialOffsetB > 68)
        throw SilKit::ConfigurationError{"pMacroInitialOffsetB must be in range 2 - 68"};

    if (nodeParameters.pMicroInitialOffsetA < 0 || nodeParameters.pMicroInitialOffsetA > 239)
        throw SilKit::ConfigurationError{"pMicroInitialOffsetA must be in range 0 - 239"};

    if (nodeParameters.pMicroInitialOffsetB < 0 || nodeParameters.pMicroInitialOffsetB > 239)
        throw SilKit::ConfigurationError{"pMicroInitialOffsetB must be in range 0 - 239"};

    if (nodeParameters.pMicroPerCycle < 960 || nodeParameters.pMicroPerCycle > 1280000)
        throw SilKit::ConfigurationError{"pMicroPerCycle must be in range 960 - 1280000"};

    if (nodeParameters.pOffsetCorrectionOut < 15 || nodeParameters.pOffsetCorrectionOut > 16082)
        throw SilKit::ConfigurationError{"pOffsetCorrectionOut must be in range 15 - 16082"};

    if (nodeParameters.pOffsetCorrectionStart < 7 || nodeParameters.pOffsetCorrectionStart > 15999)
        throw SilKit::ConfigurationError{"pOffsetCorrectionStart must be in range 7 - 15999"};

    if (nodeParameters.pRateCorrectionOut < 3 || nodeParameters.pRateCorrectionOut > 3846)
        throw SilKit::ConfigurationError{"pRateCorrectionOut must be in range 3 - 3846"};

    if (nodeParameters.pWakeupChannel != FlexrayChannel::A && nodeParameters.pWakeupChannel != FlexrayChannel::B)
        throw SilKit::ConfigurationError{"pWakeupChannel must be either FlexrayChannel A or FlexrayChannel B"};

    if (nodeParameters.pWakeupPattern > 63)
        throw SilKit::ConfigurationError{"pWakeupPattern must be in range 0 - 63"};

    if (nodeParameters.pSamplesPerMicrotick < 1 || nodeParameters.pSamplesPerMicrotick > 2)
        throw SilKit::ConfigurationError{"pSamplesPerMicrotick must be 1 or 2"};
}

} // namespace Flexray
} // namespace Services
} // namespace SilKit
