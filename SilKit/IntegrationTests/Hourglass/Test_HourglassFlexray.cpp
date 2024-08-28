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

#include "silkit/SilKit.hpp"
#include "silkit/detail/impl/ThrowOnError.hpp"

#include "MockCapiTest.hpp"

namespace {

using testing::DoAll;
using testing::SetArgPointee;
using testing::Return;

using namespace SilKit::Services::Flexray;

MATCHER_P(FlexrayControllerConfigMatcher, controlFrame, "")
{
    *result_listener << "matches Flexray frames of the c-api to the cpp api";

    FlexrayControllerConfig config1 = controlFrame;
    const SilKit_FlexrayControllerConfig* config2 = arg;

    if (static_cast<uint16_t>(config1.nodeParams.pAllowHaltDueToClock)
            != static_cast<uint16_t>(config2->nodeParams->pAllowHaltDueToClock)
        || static_cast<uint16_t>(config1.nodeParams.pAllowPassiveToActive)
               != static_cast<uint16_t>(config2->nodeParams->pAllowPassiveToActive)
        || static_cast<uint16_t>(config1.nodeParams.pChannels) != static_cast<uint16_t>(config2->nodeParams->pChannels)
        || static_cast<uint16_t>(config1.nodeParams.pClusterDriftDamping)
               != static_cast<uint16_t>(config2->nodeParams->pClusterDriftDamping)
        || static_cast<uint16_t>(config1.nodeParams.pdAcceptedStartupRange)
               != static_cast<uint16_t>(config2->nodeParams->pdAcceptedStartupRange)
        || static_cast<uint16_t>(config1.nodeParams.pdListenTimeout)
               != static_cast<uint16_t>(config2->nodeParams->pdListenTimeout)
        || static_cast<uint16_t>(config1.nodeParams.pdMicrotick)
               != static_cast<uint16_t>(config2->nodeParams->pdMicrotick)
        || static_cast<uint16_t>(config1.nodeParams.pKeySlotId)
               != static_cast<uint16_t>(config2->nodeParams->pKeySlotId)
        || static_cast<uint16_t>(config1.nodeParams.pKeySlotOnlyEnabled)
               != static_cast<uint16_t>(config2->nodeParams->pKeySlotOnlyEnabled)
        || static_cast<uint16_t>(config1.nodeParams.pKeySlotUsedForStartup)
               != static_cast<uint16_t>(config2->nodeParams->pKeySlotUsedForStartup)
        || static_cast<uint16_t>(config1.nodeParams.pKeySlotUsedForSync)
               != static_cast<uint16_t>(config2->nodeParams->pKeySlotUsedForSync)
        || static_cast<uint16_t>(config1.nodeParams.pLatestTx) != static_cast<uint16_t>(config2->nodeParams->pLatestTx)
        || static_cast<uint16_t>(config1.nodeParams.pMacroInitialOffsetA)
               != static_cast<uint16_t>(config2->nodeParams->pMacroInitialOffsetA)
        || static_cast<uint16_t>(config1.nodeParams.pMacroInitialOffsetB)
               != static_cast<uint16_t>(config2->nodeParams->pMacroInitialOffsetB)
        || static_cast<uint16_t>(config1.nodeParams.pMicroInitialOffsetA)
               != static_cast<uint16_t>(config2->nodeParams->pMicroInitialOffsetA)
        || static_cast<uint16_t>(config1.nodeParams.pMicroInitialOffsetB)
               != static_cast<uint16_t>(config2->nodeParams->pMicroInitialOffsetB)
        || static_cast<uint16_t>(config1.nodeParams.pMicroPerCycle)
               != static_cast<uint16_t>(config2->nodeParams->pMicroPerCycle)
        || static_cast<uint16_t>(config1.nodeParams.pOffsetCorrectionOut)
               != static_cast<uint16_t>(config2->nodeParams->pOffsetCorrectionOut)
        || static_cast<uint16_t>(config1.nodeParams.pOffsetCorrectionStart)
               != static_cast<uint16_t>(config2->nodeParams->pOffsetCorrectionStart)
        || static_cast<uint16_t>(config1.nodeParams.pRateCorrectionOut)
               != static_cast<uint16_t>(config2->nodeParams->pRateCorrectionOut)
        || static_cast<uint16_t>(config1.nodeParams.pSamplesPerMicrotick)
               != static_cast<uint16_t>(config2->nodeParams->pSamplesPerMicrotick)
        || static_cast<uint16_t>(config1.nodeParams.pWakeupChannel)
               != static_cast<uint16_t>(config2->nodeParams->pWakeupChannel)
        || static_cast<uint16_t>(config1.nodeParams.pChannels) != static_cast<uint16_t>(config2->nodeParams->pChannels)
        || static_cast<uint16_t>(config1.nodeParams.pWakeupPattern)
               != static_cast<uint16_t>(config2->nodeParams->pWakeupPattern)
        || static_cast<uint16_t>(config1.clusterParams.gColdstartAttempts)
               != static_cast<uint16_t>(config2->clusterParams->gColdstartAttempts)
        || static_cast<uint16_t>(config1.clusterParams.gCycleCountMax)
               != static_cast<uint16_t>(config2->clusterParams->gCycleCountMax)
        || static_cast<uint16_t>(config1.clusterParams.gdActionPointOffset)
               != static_cast<uint16_t>(config2->clusterParams->gdActionPointOffset)
        || static_cast<uint16_t>(config1.clusterParams.gdDynamicSlotIdlePhase)
               != static_cast<uint16_t>(config2->clusterParams->gdDynamicSlotIdlePhase)
        || static_cast<uint16_t>(config1.clusterParams.gdMiniSlot)
               != static_cast<uint16_t>(config2->clusterParams->gdMiniSlot)
        || static_cast<uint16_t>(config1.clusterParams.gdMiniSlotActionPointOffset)
               != static_cast<uint16_t>(config2->clusterParams->gdMiniSlotActionPointOffset)
        || static_cast<uint16_t>(config1.clusterParams.gdStaticSlot)
               != static_cast<uint16_t>(config2->clusterParams->gdStaticSlot)
        || static_cast<uint16_t>(config1.clusterParams.gdSymbolWindow)
               != static_cast<uint16_t>(config2->clusterParams->gdSymbolWindow)
        || static_cast<uint16_t>(config1.clusterParams.gdSymbolWindowActionPointOffset)
               != static_cast<uint16_t>(config2->clusterParams->gdSymbolWindowActionPointOffset)
        || static_cast<uint16_t>(config1.clusterParams.gdTSSTransmitter)
               != static_cast<uint16_t>(config2->clusterParams->gdTSSTransmitter)
        || static_cast<uint16_t>(config1.clusterParams.gdWakeupTxActive)
               != static_cast<uint16_t>(config2->clusterParams->gdWakeupTxActive)
        || static_cast<uint16_t>(config1.clusterParams.gdWakeupTxIdle)
               != static_cast<uint16_t>(config2->clusterParams->gdWakeupTxIdle)
        || static_cast<uint16_t>(config1.clusterParams.gListenNoise)
               != static_cast<uint16_t>(config2->clusterParams->gListenNoise)
        || static_cast<uint16_t>(config1.clusterParams.gMacroPerCycle)
               != static_cast<uint16_t>(config2->clusterParams->gMacroPerCycle)
        || static_cast<uint16_t>(config1.clusterParams.gMaxWithoutClockCorrectionFatal)
               != static_cast<uint16_t>(config2->clusterParams->gMaxWithoutClockCorrectionFatal)
        || static_cast<uint16_t>(config1.clusterParams.gMaxWithoutClockCorrectionPassive)
               != static_cast<uint16_t>(config2->clusterParams->gMaxWithoutClockCorrectionPassive)
        || static_cast<uint16_t>(config1.clusterParams.gNumberOfMiniSlots)
               != static_cast<uint16_t>(config2->clusterParams->gNumberOfMiniSlots)
        || static_cast<uint16_t>(config1.clusterParams.gNumberOfStaticSlots)
               != static_cast<uint16_t>(config2->clusterParams->gNumberOfStaticSlots)
        || static_cast<uint16_t>(config1.clusterParams.gPayloadLengthStatic)
               != static_cast<uint16_t>(config2->clusterParams->gPayloadLengthStatic)
        || static_cast<uint16_t>(config1.clusterParams.gSyncFrameIDCountMax)
               != static_cast<uint16_t>(config2->clusterParams->gSyncFrameIDCountMax))
    {
        return false;
    }

    if (config1.bufferConfigs.size() != config2->numBufferConfigs)
    {
        return false;
    }

    for (size_t i = 0; i < config1.bufferConfigs.size(); i++)
    {
        if (static_cast<uint16_t>(config1.bufferConfigs[i].channels)
            != static_cast<uint16_t>(config2->bufferConfigs[i].channels))
        {
            return false;
        }
        if (static_cast<uint16_t>(config1.bufferConfigs[i].hasPayloadPreambleIndicator)
            != static_cast<uint16_t>(config2->bufferConfigs[i].hasPayloadPreambleIndicator))
        {
            return false;
        }
        if (static_cast<uint16_t>(config1.bufferConfigs[i].headerCrc)
            != static_cast<uint16_t>(config2->bufferConfigs[i].headerCrc))
        {
            return false;
        }
        if (static_cast<uint16_t>(config1.bufferConfigs[i].offset)
            != static_cast<uint16_t>(config2->bufferConfigs[i].offset))
        {
            return false;
        }
        if (static_cast<uint16_t>(config1.bufferConfigs[i].repetition)
            != static_cast<uint16_t>(config2->bufferConfigs[i].repetition))
        {
            return false;
        }
        if (static_cast<uint16_t>(config1.bufferConfigs[i].slotId)
            != static_cast<uint16_t>(config2->bufferConfigs[i].slotId))
        {
            return false;
        }
        if (static_cast<uint16_t>(config1.bufferConfigs[i].transmissionMode)
            != static_cast<uint16_t>(config2->bufferConfigs[i].transmissionMode))
        {
            return false;
        }
    }
    return true;
}

MATCHER_P(FlexRayTxBufferConfigMatcher, controlFrame, "")
{
    *result_listener << "matches Flexray frames of the c-api to the cpp api";

    FlexrayTxBufferConfig bufferConfig1 = controlFrame;
    const SilKit_FlexrayTxBufferConfig* bufferConfig2 = arg;

    if (static_cast<uint16_t>(bufferConfig1.channels) != static_cast<uint16_t>(bufferConfig2->channels))
    {
        return false;
    }
    if (static_cast<uint16_t>(bufferConfig1.hasPayloadPreambleIndicator)
        != static_cast<uint16_t>(bufferConfig2->hasPayloadPreambleIndicator))
    {
        return false;
    }
    if (static_cast<uint16_t>(bufferConfig1.headerCrc) != static_cast<uint16_t>(bufferConfig2->headerCrc))
    {
        return false;
    }
    if (static_cast<uint16_t>(bufferConfig1.offset) != static_cast<uint16_t>(bufferConfig2->offset))
    {
        return false;
    }
    if (static_cast<uint16_t>(bufferConfig1.repetition) != static_cast<uint16_t>(bufferConfig2->repetition))
    {
        return false;
    }
    if (static_cast<uint16_t>(bufferConfig1.slotId) != static_cast<uint16_t>(bufferConfig2->slotId))
    {
        return false;
    }
    if (static_cast<uint16_t>(bufferConfig1.transmissionMode) != static_cast<uint16_t>(bufferConfig2->transmissionMode))
    {
        return false;
    }
    return true;
}

MATCHER_P(FlexrayTxBufferUpdateMatcher, controlFrame, "")
{
    *result_listener << "matches Flexray frames of the c-api to the cpp api";

    FlexrayTxBufferUpdate update1 = controlFrame;
    const SilKit_FlexrayTxBufferUpdate* update2 = arg;

    if (static_cast<uint16_t>(update1.payloadDataValid) != static_cast<uint16_t>(update2->payloadDataValid)
        || static_cast<uint16_t>(update1.txBufferIndex) != static_cast<uint16_t>(update2->txBufferIndex))
    {
        return false;
    }
    if (update1.payload.size() != update2->payload.size)
    {
        return false;
    }
    for (size_t i = 0; i < update1.payload.size(); i++)
    {
        if (static_cast<uint16_t>(update1.payload.at(i)) != static_cast<uint16_t>(update2->payload.data[i]))
        {
            return false;
        }
    }
    return true;
}

class Test_HourglassFlexray : public SilKitHourglassTests::MockCapiTest
{
public:
    SilKit_FlexrayController* mockFlexrayController{reinterpret_cast<SilKit_FlexrayController*>(uintptr_t(0x12345678))};

    Test_HourglassFlexray()
    {
        using testing::_;
        ON_CALL(capi, SilKit_FlexrayController_Create(_, _, _, _))
            .WillByDefault(DoAll(SetArgPointee<0>(mockFlexrayController), Return(SilKit_ReturnCode_SUCCESS)));
    }
};

TEST_F(Test_HourglassFlexray, SilKit_FlexrayController_Create)
{
    SilKit_Participant* participant{(SilKit_Participant*)123456};
    std::string name = "FlexrayController1";
    std::string network = "FlexrayNetwork1";

    EXPECT_CALL(capi, SilKit_FlexrayController_Create(testing::_, participant, testing::StrEq(name.c_str()),
                                                      testing::StrEq(network.c_str())))
        .Times(1);
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Flexray::FlexrayController FlexrayController(
        participant, name, network);
}

TEST_F(Test_HourglassFlexray, SilKit_FlexrayController_Configure)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Flexray::FlexrayController FlexrayController(
        nullptr, "FlexrayController1", "FlexrayNetwork1");

    FlexrayControllerConfig controllerConfig{};
    controllerConfig.nodeParams.pAllowHaltDueToClock = 1;
    controllerConfig.nodeParams.pAllowPassiveToActive = 2;
    controllerConfig.nodeParams.pChannels = FlexrayChannel::AB;
    controllerConfig.nodeParams.pClusterDriftDamping = 10;
    controllerConfig.nodeParams.pdAcceptedStartupRange = 2743;
    controllerConfig.nodeParams.pdListenTimeout = 2567692;
    controllerConfig.nodeParams.pdMicrotick = FlexrayClockPeriod::T50NS;
    controllerConfig.nodeParams.pKeySlotId = 1023;
    controllerConfig.nodeParams.pKeySlotOnlyEnabled = 3;
    controllerConfig.nodeParams.pKeySlotUsedForStartup = 4;
    controllerConfig.nodeParams.pKeySlotUsedForSync = 5;
    controllerConfig.nodeParams.pLatestTx = 7988;
    controllerConfig.nodeParams.pMacroInitialOffsetA = 68;
    controllerConfig.nodeParams.pMacroInitialOffsetB = 67;
    controllerConfig.nodeParams.pMicroInitialOffsetA = 239;
    controllerConfig.nodeParams.pMicroInitialOffsetB = 238;
    controllerConfig.nodeParams.pMicroPerCycle = 1280000;
    controllerConfig.nodeParams.pOffsetCorrectionOut = 16082;
    controllerConfig.nodeParams.pOffsetCorrectionStart = 15999;
    controllerConfig.nodeParams.pRateCorrectionOut = 3846;
    controllerConfig.nodeParams.pSamplesPerMicrotick = 6;
    controllerConfig.nodeParams.pWakeupChannel = FlexrayChannel::A;
    controllerConfig.nodeParams.pWakeupPattern = 7;
    controllerConfig.clusterParams.gColdstartAttempts = 31;
    controllerConfig.clusterParams.gCycleCountMax = 63;
    controllerConfig.clusterParams.gdActionPointOffset = 62;
    controllerConfig.clusterParams.gdDynamicSlotIdlePhase = 8;
    controllerConfig.clusterParams.gdMiniSlot = 60;
    controllerConfig.clusterParams.gdMiniSlotActionPointOffset = 29;
    controllerConfig.clusterParams.gdStaticSlot = 664;
    controllerConfig.clusterParams.gdSymbolWindow = 162;
    controllerConfig.clusterParams.gdSymbolWindowActionPointOffset = 59;
    controllerConfig.clusterParams.gdTSSTransmitter = 15;
    controllerConfig.clusterParams.gdWakeupTxActive = 16;
    controllerConfig.clusterParams.gdWakeupTxIdle = 180;
    controllerConfig.clusterParams.gListenNoise = 14;
    controllerConfig.clusterParams.gMacroPerCycle = 16000;
    controllerConfig.clusterParams.gMaxWithoutClockCorrectionFatal = 13;
    controllerConfig.clusterParams.gMaxWithoutClockCorrectionPassive = 12;
    controllerConfig.clusterParams.gNumberOfMiniSlots = 7988;
    controllerConfig.clusterParams.gNumberOfStaticSlots = 1022;
    controllerConfig.clusterParams.gPayloadLengthStatic = 127;
    controllerConfig.clusterParams.gSyncFrameIDCountMax = 10;
    FlexrayTxBufferConfig bufferConfig{FlexrayChannel::A, 1, 2, 3, true, 5, FlexrayTransmissionMode::Continuous};
    controllerConfig.bufferConfigs.push_back(bufferConfig);

    EXPECT_CALL(capi, SilKit_FlexrayController_Configure(mockFlexrayController,
                                                         FlexrayControllerConfigMatcher(controllerConfig)))
        .Times(1);
    FlexrayController.Configure(controllerConfig);
}

TEST_F(Test_HourglassFlexray, SilKit_FlexrayController_ReconfigureTxBuffer)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Flexray::FlexrayController FlexrayController(
        nullptr, "FlexrayController1", "FlexrayNetwork1");

    FlexrayTxBufferConfig bufferConfig{FlexrayChannel::A, 1, 2, 3, true, 5, FlexrayTransmissionMode::Continuous};
    EXPECT_CALL(capi, SilKit_FlexrayController_ReconfigureTxBuffer(mockFlexrayController, 1234,
                                                                   FlexRayTxBufferConfigMatcher(bufferConfig)))
        .Times(1);
    FlexrayController.ReconfigureTxBuffer(1234, bufferConfig);
}

TEST_F(Test_HourglassFlexray, SilKit_FlexrayController_UpdateTxBuffer)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Flexray::FlexrayController FlexrayController(
        nullptr, "FlexrayController1", "FlexrayNetwork1");

    std::vector<uint8_t> payload{1, 2, 3};
    FlexrayTxBufferUpdate bufferUpdate{12345, true, payload};
    EXPECT_CALL(capi, SilKit_FlexrayController_UpdateTxBuffer(mockFlexrayController,
                                                              FlexrayTxBufferUpdateMatcher(bufferUpdate)))
        .Times(1);
    FlexrayController.UpdateTxBuffer(bufferUpdate);
}

TEST_F(Test_HourglassFlexray, SilKit_FlexrayController_ExecuteCmd_Run)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Flexray::FlexrayController FlexrayController(
        nullptr, "FlexrayController1", "FlexrayNetwork1");

    EXPECT_CALL(capi, SilKit_FlexrayController_ExecuteCmd(mockFlexrayController, SilKit_FlexrayChiCommand_RUN))
        .Times(1);
    FlexrayController.Run();
}

TEST_F(Test_HourglassFlexray, SilKit_FlexrayController_ExecuteCmd_DeferredHalt)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Flexray::FlexrayController FlexrayController(
        nullptr, "FlexrayController1", "FlexrayNetwork1");

    EXPECT_CALL(capi,
                SilKit_FlexrayController_ExecuteCmd(mockFlexrayController, SilKit_FlexrayChiCommand_DEFERRED_HALT))
        .Times(1);
    FlexrayController.DeferredHalt();
}

TEST_F(Test_HourglassFlexray, SilKit_FlexrayController_ExecuteCmd_Freeze)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Flexray::FlexrayController FlexrayController(
        nullptr, "FlexrayController1", "FlexrayNetwork1");

    EXPECT_CALL(capi, SilKit_FlexrayController_ExecuteCmd(mockFlexrayController, SilKit_FlexrayChiCommand_FREEZE))
        .Times(1);
    FlexrayController.Freeze();
}

TEST_F(Test_HourglassFlexray, SilKit_FlexrayController_ExecuteCmd_AllowColdstart)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Flexray::FlexrayController FlexrayController(
        nullptr, "FlexrayController1", "FlexrayNetwork1");

    EXPECT_CALL(capi,
                SilKit_FlexrayController_ExecuteCmd(mockFlexrayController, SilKit_FlexrayChiCommand_ALLOW_COLDSTART))
        .Times(1);
    FlexrayController.AllowColdstart();
}

TEST_F(Test_HourglassFlexray, SilKit_FlexrayController_ExecuteCmd_AllSlots)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Flexray::FlexrayController FlexrayController(
        nullptr, "FlexrayController1", "FlexrayNetwork1");

    EXPECT_CALL(capi, SilKit_FlexrayController_ExecuteCmd(mockFlexrayController, SilKit_FlexrayChiCommand_ALL_SLOTS))
        .Times(1);
    FlexrayController.AllSlots();
}

TEST_F(Test_HourglassFlexray, SilKit_FlexrayController_ExecuteCmd_Wakeup)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Flexray::FlexrayController FlexrayController(
        nullptr, "FlexrayController1", "FlexrayNetwork1");

    EXPECT_CALL(capi, SilKit_FlexrayController_ExecuteCmd(mockFlexrayController, SilKit_FlexrayChiCommand_WAKEUP))
        .Times(1);
    FlexrayController.Wakeup();
}

TEST_F(Test_HourglassFlexray, SilKit_FlexrayController_AddFrameHandler)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Flexray::FlexrayController FlexrayController(
        nullptr, "FlexrayController1", "FlexrayNetwork1");

    EXPECT_CALL(capi,
                SilKit_FlexrayController_AddFrameHandler(mockFlexrayController, testing::_, testing::_, testing::_))
        .Times(1);
    FlexrayController.AddFrameHandler({});
}

TEST_F(Test_HourglassFlexray, SilKit_FlexrayController_RemoveFrameHandler)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Flexray::FlexrayController FlexrayController(
        nullptr, "FlexrayController1", "FlexrayNetwork1");

    EXPECT_CALL(capi, SilKit_FlexrayController_RemoveFrameHandler(mockFlexrayController, testing::_)).Times(1);
    FlexrayController.RemoveFrameHandler({});
}

TEST_F(Test_HourglassFlexray, SilKit_FlexrayController_AddFrameTransmitHandler)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Flexray::FlexrayController FlexrayController(
        nullptr, "FlexrayController1", "FlexrayNetwork1");

    EXPECT_CALL(capi, SilKit_FlexrayController_AddFrameTransmitHandler(mockFlexrayController, testing::_, testing::_,
                                                                       testing::_))
        .Times(1);
    FlexrayController.AddFrameTransmitHandler({});
}

TEST_F(Test_HourglassFlexray, SilKit_FlexrayController_RemoveFrameTransmitHandler)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Flexray::FlexrayController FlexrayController(
        nullptr, "FlexrayController1", "FlexrayNetwork1");

    EXPECT_CALL(capi, SilKit_FlexrayController_RemoveFrameTransmitHandler(mockFlexrayController, testing::_)).Times(1);
    FlexrayController.RemoveFrameTransmitHandler({});
}

TEST_F(Test_HourglassFlexray, SilKit_FlexrayController_AddWakeupHandler)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Flexray::FlexrayController FlexrayController(
        nullptr, "FlexrayController1", "FlexrayNetwork1");

    EXPECT_CALL(capi,
                SilKit_FlexrayController_AddWakeupHandler(mockFlexrayController, testing::_, testing::_, testing::_))
        .Times(1);
    FlexrayController.AddWakeupHandler({});
}

TEST_F(Test_HourglassFlexray, SilKit_FlexrayController_RemoveWakeupHandler)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Flexray::FlexrayController FlexrayController(
        nullptr, "FlexrayController1", "FlexrayNetwork1");

    EXPECT_CALL(capi, SilKit_FlexrayController_RemoveWakeupHandler(mockFlexrayController, testing::_)).Times(1);
    FlexrayController.RemoveWakeupHandler({});
}

TEST_F(Test_HourglassFlexray, SilKit_FlexrayController_AddPocStatusHandler)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Flexray::FlexrayController FlexrayController(
        nullptr, "FlexrayController1", "FlexrayNetwork1");

    EXPECT_CALL(capi,
                SilKit_FlexrayController_AddPocStatusHandler(mockFlexrayController, testing::_, testing::_, testing::_))
        .Times(1);
    FlexrayController.AddPocStatusHandler({});
}

TEST_F(Test_HourglassFlexray, SilKit_FlexrayController_RemovePocStatusHandler)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Flexray::FlexrayController FlexrayController(
        nullptr, "FlexrayController1", "FlexrayNetwork1");

    EXPECT_CALL(capi, SilKit_FlexrayController_RemovePocStatusHandler(mockFlexrayController, testing::_)).Times(1);
    FlexrayController.RemovePocStatusHandler({});
}

TEST_F(Test_HourglassFlexray, SilKit_FlexrayController_AddSymbolHandler)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Flexray::FlexrayController FlexrayController(
        nullptr, "FlexrayController1", "FlexrayNetwork1");

    EXPECT_CALL(capi,
                SilKit_FlexrayController_AddSymbolHandler(mockFlexrayController, testing::_, testing::_, testing::_))
        .Times(1);
    FlexrayController.AddSymbolHandler({});
}

TEST_F(Test_HourglassFlexray, SilKit_FlexrayController_RemoveSymbolHandler)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Flexray::FlexrayController FlexrayController(
        nullptr, "FlexrayController1", "FlexrayNetwork1");

    EXPECT_CALL(capi, SilKit_FlexrayController_RemoveSymbolHandler(mockFlexrayController, testing::_)).Times(1);
    FlexrayController.RemoveSymbolHandler({});
}

TEST_F(Test_HourglassFlexray, SilKit_FlexrayController_AddSymbolTransmitHandler)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Flexray::FlexrayController FlexrayController(
        nullptr, "FlexrayController1", "FlexrayNetwork1");

    EXPECT_CALL(capi, SilKit_FlexrayController_AddSymbolTransmitHandler(mockFlexrayController, testing::_, testing::_,
                                                                        testing::_))
        .Times(1);
    FlexrayController.AddSymbolTransmitHandler({});
}

TEST_F(Test_HourglassFlexray, SilKit_FlexrayController_RemoveSymbolTransmitHandler)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Flexray::FlexrayController FlexrayController(
        nullptr, "FlexrayController1", "FlexrayNetwork1");

    EXPECT_CALL(capi, SilKit_FlexrayController_RemoveSymbolTransmitHandler(mockFlexrayController, testing::_)).Times(1);
    FlexrayController.RemoveSymbolTransmitHandler({});
}

TEST_F(Test_HourglassFlexray, SilKit_FlexrayController_AddCycleStartHandler)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Flexray::FlexrayController FlexrayController(
        nullptr, "FlexrayController1", "FlexrayNetwork1");

    EXPECT_CALL(
        capi, SilKit_FlexrayController_AddCycleStartHandler(mockFlexrayController, testing::_, testing::_, testing::_))
        .Times(1);
    FlexrayController.AddCycleStartHandler({});
}

TEST_F(Test_HourglassFlexray, SilKit_FlexrayController_RemoveCycleStartHandler)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Flexray::FlexrayController FlexrayController(
        nullptr, "FlexrayController1", "FlexrayNetwork1");

    EXPECT_CALL(capi, SilKit_FlexrayController_RemoveCycleStartHandler(mockFlexrayController, testing::_)).Times(1);
    FlexrayController.RemoveCycleStartHandler({});
}

} //namespace
