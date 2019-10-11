// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "Validation.hpp"

#include "ib/cfg/Config.hpp"

namespace ib {
namespace sim {
namespace fr {

void Validate(const ClusterParameters& clusterParameters)
{
    if (clusterParameters.gColdstartAttempts < 2 || clusterParameters.gColdstartAttempts > 31)
        throw ib::cfg::Misconfiguration{"gColdstartAttempts must be in range 2 - 31"};

    if (clusterParameters.gCycleCountMax % 2 != 1)
        throw ib::cfg::Misconfiguration{"gCycleCountMax must be an odd value"};
        
    if (clusterParameters.gCycleCountMax < 7 || clusterParameters.gCycleCountMax > 63)
        throw ib::cfg::Misconfiguration{"gCycleCountMax must be in range 7, 9, ..., 63"};

    if (clusterParameters.gdActionPointOffset < 1 || clusterParameters.gdActionPointOffset > 63)
        throw ib::cfg::Misconfiguration{"gdActionPointOffset must be in range 1 - 63"};

    if (clusterParameters.gdDynamicSlotIdlePhase < 0 || clusterParameters.gdDynamicSlotIdlePhase > 2)
        throw ib::cfg::Misconfiguration{"gdDynamicSlotIdlePhase must be in range 0 - 2"};

    if (clusterParameters.gdMiniSlot < 2 || clusterParameters.gdMiniSlot > 63)
        throw ib::cfg::Misconfiguration{"gdMiniSlot must be in range 2 - 63"};

    if (clusterParameters.gdMiniSlotActionPointOffset < 1 || clusterParameters.gdMiniSlotActionPointOffset > 31)
        throw ib::cfg::Misconfiguration{"gdMiniSlotActionPointOffset must be in range 1 - 31"};

    if (clusterParameters.gdStaticSlot < 3 || clusterParameters.gdStaticSlot > 664)
        throw ib::cfg::Misconfiguration{"gdStaticSlot must be in range 3 - 664"};

    if (clusterParameters.gdSymbolWindow < 0 || clusterParameters.gdSymbolWindow > 162)
        throw ib::cfg::Misconfiguration{"gdSymbolWindow must be in range 0 - 162"};

    if (clusterParameters.gdSymbolWindowActionPointOffset < 1 || clusterParameters.gdSymbolWindowActionPointOffset > 63)
        throw ib::cfg::Misconfiguration{"gdSymbolWindowActionPointOffset must be in range 1 - 63"};

    if (clusterParameters.gdTSSTransmitter < 1 || clusterParameters.gdTSSTransmitter > 15)
        throw ib::cfg::Misconfiguration{"gdTSSTransmitter must be in range 1 - 15"};

    if (clusterParameters.gdWakeupTxActive < 15 || clusterParameters.gdWakeupTxActive > 60)
        throw ib::cfg::Misconfiguration{"gdWakeupTxActive must be in range 15 - 60"};

    if (clusterParameters.gdWakeupTxIdle < 45 || clusterParameters.gdWakeupTxIdle > 180)
        throw ib::cfg::Misconfiguration{"gdWakeupTxIdle must be in range 45 - 180"};

    if (clusterParameters.gListenNoise < 2 || clusterParameters.gListenNoise > 16)
        throw ib::cfg::Misconfiguration{"gListenNoise must be in range 2 - 16"};

    if (clusterParameters.gMacroPerCycle < 8 || clusterParameters.gMacroPerCycle > 16000)
        throw ib::cfg::Misconfiguration{"gMacroPerCycle must be in range 8 - 16000"};

    if (clusterParameters.gMaxWithoutClockCorrectionFatal < 1 || clusterParameters.gMaxWithoutClockCorrectionFatal > 15)
        throw ib::cfg::Misconfiguration{"gMaxWithoutClockCorrectionFatal must be in range 1 - 15"};

    if (clusterParameters.gMaxWithoutClockCorrectionPassive < 1 || clusterParameters.gMaxWithoutClockCorrectionPassive > 15)
        throw ib::cfg::Misconfiguration{"gMaxWithoutClockCorrectionPassive must be in range 1 - 15"};

    if (clusterParameters.gNumberOfMiniSlots < 0 || clusterParameters.gNumberOfMiniSlots > 7988)
        throw ib::cfg::Misconfiguration{"gNumberOfMiniSlots must be in range 0 - 7988"};

    if (clusterParameters.gNumberOfStaticSlots < 2 || clusterParameters.gNumberOfStaticSlots > 1023)
        throw ib::cfg::Misconfiguration{"gNumberOfStaticSlots must be in range 2 - 1023"};

    if (clusterParameters.gPayloadLengthStatic < 0 || clusterParameters.gPayloadLengthStatic > 127)
        throw ib::cfg::Misconfiguration{"gPayloadLengthStatic must be in range 0 - 127"};

    if (clusterParameters.gSyncFrameIDCountMax < 2 || clusterParameters.gSyncFrameIDCountMax > 15)
        throw ib::cfg::Misconfiguration{"gSyncFrameIDCountMax must be in range 2 - 15"};
}

void Validate(const NodeParameters& nodeParameters)
{
    if (nodeParameters.pAllowHaltDueToClock < 0 || nodeParameters.pAllowHaltDueToClock > 1)
        throw ib::cfg::Misconfiguration{"pAllowHaltDueToClock must be 0 or 1"};

    if (nodeParameters.pAllowPassiveToActive < 0 || nodeParameters.pAllowPassiveToActive > 31)
        throw ib::cfg::Misconfiguration{"pAllowPassiveToActive must be in range 0 - 31"};

    if (nodeParameters.pClusterDriftDamping < 0 || nodeParameters.pClusterDriftDamping > 10)
        throw ib::cfg::Misconfiguration{"pClusterDriftDamping must be in range 0 - 10"};

    if (nodeParameters.pdAcceptedStartupRange < 29 || nodeParameters.pdAcceptedStartupRange > 2743)
        throw ib::cfg::Misconfiguration{"pdAcceptedStartupRange must be in range 29 - 2743"};

    if (nodeParameters.pdListenTimeout < 1926 || nodeParameters.pdListenTimeout > 2567692)
        throw ib::cfg::Misconfiguration{"pdListenTimeout must be in range 1926 - 2567692"};

    if (nodeParameters.pKeySlotId < 0 || nodeParameters.pKeySlotId > 1023)
        throw ib::cfg::Misconfiguration{"pKeySlotId must be in range 0 - 1023"};

    if (nodeParameters.pKeySlotOnlyEnabled < 0 || nodeParameters.pKeySlotOnlyEnabled > 1)
        throw ib::cfg::Misconfiguration{"pKeySlotOnlyEnabled must be 0 or 1"};

    if (nodeParameters.pKeySlotUsedForStartup < 0 || nodeParameters.pKeySlotUsedForStartup > 1)
        throw ib::cfg::Misconfiguration{"pKeySlotUsedForStartup must be 0 or 1"};

    if (nodeParameters.pKeySlotUsedForSync < 0 || nodeParameters.pKeySlotUsedForSync > 1)
        throw ib::cfg::Misconfiguration{"pKeySlotUsedForSync must be 0 or 1"};

    if (nodeParameters.pLatestTx < 0 || nodeParameters.pLatestTx > 7988)
        throw ib::cfg::Misconfiguration{"pLatestTx must be in range 0 - 7988"};

    if (nodeParameters.pMacroInitialOffsetA < 2 || nodeParameters.pMacroInitialOffsetA > 68)
        throw ib::cfg::Misconfiguration{"pMacroInitialOffsetA must be in range 2 - 68"};

    if (nodeParameters.pMacroInitialOffsetB < 2 || nodeParameters.pMacroInitialOffsetB > 68)
        throw ib::cfg::Misconfiguration{"pMacroInitialOffsetB must be in range 2 - 68"};

    if (nodeParameters.pMicroInitialOffsetA < 0 || nodeParameters.pMicroInitialOffsetA > 239)
        throw ib::cfg::Misconfiguration{"pMicroInitialOffsetA must be in range 0 - 239"};

    if (nodeParameters.pMicroInitialOffsetB < 0 || nodeParameters.pMicroInitialOffsetB > 239)
        throw ib::cfg::Misconfiguration{"pMicroInitialOffsetB must be in range 0 - 239"};

    if (nodeParameters.pMicroPerCycle < 960 || nodeParameters.pMicroPerCycle > 1280000)
        throw ib::cfg::Misconfiguration{"pMicroPerCycle must be in range 960 - 1280000"};

    if (nodeParameters.pOffsetCorrectionOut < 15 || nodeParameters.pOffsetCorrectionOut > 16082)
        throw ib::cfg::Misconfiguration{"pOffsetCorrectionOut must be in range 15 - 16082"};

    if (nodeParameters.pOffsetCorrectionStart < 7 || nodeParameters.pOffsetCorrectionStart > 15999)
        throw ib::cfg::Misconfiguration{"pOffsetCorrectionStart must be in range 7 - 15999"};

    if (nodeParameters.pRateCorrectionOut < 3 || nodeParameters.pRateCorrectionOut > 3846)
        throw ib::cfg::Misconfiguration{"pRateCorrectionOut must be in range 3 - 3846"};

    if (nodeParameters.pWakeupPattern < 0 || nodeParameters.pWakeupPattern > 63)
        throw ib::cfg::Misconfiguration{"pWakeupPattern must be in range 0 - 63"};

    if (nodeParameters.pSamplesPerMicrotick < 1 || nodeParameters.pSamplesPerMicrotick > 2)
        throw ib::cfg::Misconfiguration{"pSamplesPerMicrotick must be 1 or 2"};
}

} // namespace fr
} // namespace sim
} // namespace ib
