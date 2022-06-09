// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "FlexrayController.hpp"

#include <chrono>
#include <functional>
#include <numeric>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "ib/util/functional.hpp"

#include "FlexrayDatatypeUtils.hpp"

#include "MockParticipant.hpp"

#include "ParticipantConfiguration.hpp"

namespace {

using namespace std::chrono_literals;
using namespace std::placeholders;

using testing::_;
using testing::A;
using testing::An;
using testing::InSequence;
using testing::NiceMock;
using testing::Return;

using namespace ib::mw;
using namespace ib::util;
using namespace ib::sim::fr;

using ::ib::mw::test::DummyParticipant;

class MockParticipant : public DummyParticipant
{
public:
    MOCK_METHOD2(SendIbMessage, void(const IIbServiceEndpoint*, const FlexrayHostCommand&));
    MOCK_METHOD2(SendIbMessage, void(const IIbServiceEndpoint*, const FlexrayControllerConfig&));
    MOCK_METHOD2(SendIbMessage, void(const IIbServiceEndpoint*, const FlexrayTxBufferConfigUpdate&));
    MOCK_METHOD2(SendIbMessage, void(const IIbServiceEndpoint*, const FlexrayTxBufferUpdate&));
};

class FlexrayControllerTest : public testing::Test
{
protected:
    struct Callbacks
    {
        MOCK_METHOD2(MessageHandler, void(IFlexrayController*, const FlexrayFrameEvent &));
        MOCK_METHOD2(MessageAckHandler, void(IFlexrayController*, const FlexrayFrameTransmitEvent &));
        MOCK_METHOD2(WakeupHandler, void(IFlexrayController*, const FlexrayWakeupEvent &));
        MOCK_METHOD2(PocStatusHandler, void(IFlexrayController*, const FlexrayPocStatusEvent &));
        MOCK_METHOD2(SymbolHandler, void(IFlexrayController*, const FlexraySymbolEvent &));
        MOCK_METHOD2(SymbolAckHandler, void(IFlexrayController*, const FlexraySymbolTransmitEvent &));
        MOCK_METHOD2(CycleStartHandler, void(IFlexrayController*, const FlexrayCycleStartEvent &));
    };

protected:
    FlexrayControllerTest()
        : controller(&participant, FlexrayControllerTest::GetDummyConfig(), participant.GetTimeProvider())
        , controllerBusSim(&participant, FlexrayControllerTest::GetDummyConfig(), participant.GetTimeProvider())
        , controllerConfigured(&participant, FlexrayControllerTest::GetDummyConfigWithValues(),
                          participant.GetTimeProvider())
    {
        
        controller.SetServiceDescriptor(from_endpointAddress(controllerAddress));
        controller.SetDetailedBehavior(from_endpointAddress(busSimAddress));
        referencePayload.resize(20);
        std::iota(referencePayload.begin(), referencePayload.end(), '\000');

        controllerBusSim.SetServiceDescriptor(from_endpointAddress(busSimAddress));
    }

protected:
    const EndpointAddress controllerAddress{4, 5};
    const EndpointAddress busSimAddress{9, 5};

    decltype(FlexrayTxBufferUpdate::payload) referencePayload;

    MockParticipant participant;
    FlexrayController controller;
    FlexrayController controllerBusSim;
    FlexrayController controllerConfigured;
    Callbacks callbacks;

    ib::cfg::FlexrayController dummyConfig;

    auto static GetDummyConfig() -> ib::cfg::FlexrayController
    {
        ib::cfg::FlexrayController dummyConfig;
        dummyConfig.network = "testNetwork";
        dummyConfig.name = "testController";
        return dummyConfig;
    }
    auto static GetDummyConfigWithValues() -> ib::cfg::FlexrayController
    {
        ib::cfg::FlexrayController dummyConfig;
        dummyConfig.network = "testNetwork";
        dummyConfig.name = "testController";
        dummyConfig.clusterParameters = MakeValidClusterParams();
        dummyConfig.nodeParameters = MakeValidNodeParams();
        dummyConfig.txBufferConfigurations.push_back(MakeValidTxBufferConfig());
        return dummyConfig;
    }

    auto static MakeValidClusterParams() -> FlexrayClusterParameters
    {
        FlexrayClusterParameters clusterParams;
        clusterParams.gColdstartAttempts = 2;
        clusterParams.gCycleCountMax = 7;
        clusterParams.gdActionPointOffset = 1;
        clusterParams.gdDynamicSlotIdlePhase = 0;
        clusterParams.gdMiniSlot = 2;
        clusterParams.gdMiniSlotActionPointOffset = 1;
        clusterParams.gdStaticSlot = 3;
        clusterParams.gdSymbolWindow = 1;
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

        return clusterParams;
    }

    auto static MakeValidNodeParams() -> FlexrayNodeParameters
    {
        FlexrayNodeParameters nodeParams;
        nodeParams.pAllowHaltDueToClock = 0;
        nodeParams.pAllowPassiveToActive = 0;
        nodeParams.pChannels = FlexrayChannel::A;
        nodeParams.pClusterDriftDamping = 0;
        nodeParams.pdAcceptedStartupRange = 29;
        nodeParams.pdListenTimeout = 1926;
        nodeParams.pdMicrotick = FlexrayClockPeriod::T12_5NS;
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
        nodeParams.pSamplesPerMicrotick = 1;
        nodeParams.pWakeupChannel = FlexrayChannel::A;
        nodeParams.pWakeupPattern = 0;

        return nodeParams;
    }

    auto static MakeValidTxBufferConfig() -> FlexrayTxBufferConfig
    {
        FlexrayTxBufferConfig bufferCfg{};
        bufferCfg.channels = FlexrayChannel::AB;
        bufferCfg.hasPayloadPreambleIndicator = false;
        bufferCfg.offset = 0;
        bufferCfg.repetition = 0;
        bufferCfg.slotId = 17;
        bufferCfg.transmissionMode = FlexrayTransmissionMode::SingleShot;
        
        return bufferCfg;
    }
};

TEST_F(FlexrayControllerTest, send_controller_config)
{
    // Configure Controller
    FlexrayControllerConfig controllerCfg{};
    controllerCfg.clusterParams = MakeValidClusterParams();
    controllerCfg.nodeParams = MakeValidNodeParams();

    controllerCfg.bufferConfigs.push_back(MakeValidTxBufferConfig());

    EXPECT_CALL(participant, SendIbMessage(&controller, controllerCfg)).Times(1);

    controller.Configure(controllerCfg);
}

TEST_F(FlexrayControllerTest, send_controller_config_override_identical)
{
    // Configure Controller
    FlexrayControllerConfig configuredControllerCfg{};
    FlexrayControllerConfig testControllerCfg{};

    configuredControllerCfg.clusterParams = MakeValidClusterParams();
    configuredControllerCfg.nodeParams = MakeValidNodeParams();
    configuredControllerCfg.bufferConfigs.push_back(MakeValidTxBufferConfig());

    testControllerCfg.clusterParams = MakeValidClusterParams();
    testControllerCfg.nodeParams = MakeValidNodeParams();
    testControllerCfg.bufferConfigs.push_back(MakeValidTxBufferConfig());

    EXPECT_CALL(participant, SendIbMessage(&controllerConfigured, configuredControllerCfg)).Times(1);
    controllerConfigured.Configure(testControllerCfg);
}

TEST_F(FlexrayControllerTest, send_controller_config_override_cluster_params)
{
    // Configure Controller
    FlexrayControllerConfig configuredControllerCfg{};
    FlexrayControllerConfig testControllerCfg{};

    configuredControllerCfg.clusterParams = MakeValidClusterParams();
    configuredControllerCfg.nodeParams = MakeValidNodeParams();
    configuredControllerCfg.bufferConfigs.push_back(MakeValidTxBufferConfig());

    // Change clusterParams in tester and make sure they are ignored
    testControllerCfg.clusterParams = MakeValidClusterParams();
    testControllerCfg.clusterParams.gNumberOfMiniSlots = 999; // was 0
    testControllerCfg.clusterParams.gMacroPerCycle = 1337; // was 0

    // default values (not configured)
    testControllerCfg.nodeParams = MakeValidNodeParams();
    testControllerCfg.bufferConfigs.push_back(MakeValidTxBufferConfig());

    EXPECT_CALL(participant, SendIbMessage(&controllerConfigured, configuredControllerCfg)).Times(1);
    controllerConfigured.Configure(testControllerCfg);
}

TEST_F(FlexrayControllerTest, send_controller_config_override_node_params)
{
    // Configure Controller
    FlexrayControllerConfig configuredControllerCfg{};
    FlexrayControllerConfig testControllerCfg{};

    configuredControllerCfg.clusterParams = MakeValidClusterParams();
    configuredControllerCfg.nodeParams = MakeValidNodeParams();
    configuredControllerCfg.bufferConfigs.push_back(MakeValidTxBufferConfig());

    // Change clusterParams in tester and make sure they are ignored
    testControllerCfg.nodeParams = MakeValidNodeParams();
    testControllerCfg.nodeParams.pKeySlotId = 42; // was 0
    testControllerCfg.nodeParams.pChannels = FlexrayChannel::B; // was A

    // default values (not configured)
    testControllerCfg.clusterParams = MakeValidClusterParams();
    testControllerCfg.bufferConfigs.push_back(MakeValidTxBufferConfig());

    EXPECT_CALL(participant, SendIbMessage(&controllerConfigured, configuredControllerCfg)).Times(1);
    controllerConfigured.Configure(testControllerCfg);
}

TEST_F(FlexrayControllerTest, send_controller_config_override_tx_buffer)
{
    // Configure Controller
    FlexrayControllerConfig configuredControllerCfg{};
    FlexrayControllerConfig testControllerCfg{};

    configuredControllerCfg.clusterParams = MakeValidClusterParams();
    configuredControllerCfg.nodeParams = MakeValidNodeParams();
    configuredControllerCfg.bufferConfigs.push_back(MakeValidTxBufferConfig());

    // Change clusterParams in tester and make sure they are ignored
    auto txBufferConfig = MakeValidTxBufferConfig();
    txBufferConfig.slotId = 42; // was 17
    testControllerCfg.bufferConfigs.push_back(txBufferConfig);

    // default values (not configured)
    testControllerCfg.clusterParams = MakeValidClusterParams();
    testControllerCfg.nodeParams = MakeValidNodeParams();

    EXPECT_CALL(participant, SendIbMessage(&controllerConfigured, configuredControllerCfg)).Times(1);
    controllerConfigured.Configure(testControllerCfg);
}

TEST_F(FlexrayControllerTest, send_txbuffer_configupdate)
{
    FlexrayTxBufferConfig bufferCfg{};
    bufferCfg.channels = FlexrayChannel::AB;
    bufferCfg.hasPayloadPreambleIndicator = false;
    bufferCfg.offset = 0;
    bufferCfg.repetition = 0;
    bufferCfg.slotId = 17;
    bufferCfg.transmissionMode = FlexrayTransmissionMode::SingleShot;

    FlexrayControllerConfig controllerCfg{};
    controllerCfg.clusterParams = MakeValidClusterParams();
    controllerCfg.nodeParams = MakeValidNodeParams();
    controllerCfg.bufferConfigs.push_back(bufferCfg);

    EXPECT_CALL(participant, SendIbMessage(&controller, controllerCfg)).Times(1);
    controller.Configure(controllerCfg);

    // Reconfigure TxBuffer 0
    bufferCfg.channels = FlexrayChannel::A;
    bufferCfg.hasPayloadPreambleIndicator = true;
    bufferCfg.offset = 1;
    bufferCfg.repetition = 3;
    bufferCfg.slotId = 15;
    bufferCfg.transmissionMode = FlexrayTransmissionMode::Continuous;

    FlexrayTxBufferConfigUpdate expectedUpdate{};
    expectedUpdate.txBufferIndex = 0;
    expectedUpdate.txBufferConfig = bufferCfg;

    EXPECT_CALL(participant, SendIbMessage(&controller, expectedUpdate)).Times(1);
    controller.ReconfigureTxBuffer(0, bufferCfg);
}

TEST_F(FlexrayControllerTest, throw_on_unconfigured_tx_buffer_configupdate)
{
    FlexrayControllerConfig controllerCfg;
    controllerCfg.clusterParams = MakeValidClusterParams();
    controllerCfg.nodeParams = MakeValidNodeParams();
    controllerCfg.bufferConfigs.resize(5);

    EXPECT_CALL(participant, SendIbMessage(&controller, controllerCfg)).Times(1);
    controller.Configure(controllerCfg);

    // Attempt to reconfigure TxBuffer 6, which should be out of range
    FlexrayTxBufferConfig bufferCfg{};
    EXPECT_CALL(participant, SendIbMessage(An<const IIbServiceEndpoint*>(), A<const FlexrayTxBufferConfigUpdate &>())).Times(0);
    EXPECT_THROW(controller.ReconfigureTxBuffer(6, bufferCfg), std::out_of_range);
}


TEST_F(FlexrayControllerTest, send_txbuffer_update)
{
    FlexrayTxBufferConfig bufferCfg{};

    FlexrayControllerConfig controllerCfg;
    controllerCfg.clusterParams = MakeValidClusterParams();
    controllerCfg.nodeParams = MakeValidNodeParams();
    controllerCfg.bufferConfigs.push_back(bufferCfg);

    EXPECT_CALL(participant, SendIbMessage(&controller, controllerCfg)).Times(1);
    controller.Configure(controllerCfg);

    FlexrayTxBufferUpdate update{};
    update.txBufferIndex = 0;
    update.payload = referencePayload;
    update.payloadDataValid = true;

    EXPECT_CALL(participant, SendIbMessage(&controller, update))
        .Times(1);

    controller.UpdateTxBuffer(update);
}


TEST_F(FlexrayControllerTest, throw_on_unconfigured_tx_buffer_update)
{
    FlexrayControllerConfig controllerCfg;
    controllerCfg.clusterParams = MakeValidClusterParams();
    controllerCfg.nodeParams = MakeValidNodeParams();
    controllerCfg.bufferConfigs.resize(1);

    EXPECT_CALL(participant, SendIbMessage(&controller, controllerCfg)).Times(1);
    controller.Configure(controllerCfg);

    FlexrayTxBufferUpdate update;
    update.txBufferIndex = 7; // only txBufferIdx = 0 is configured
    EXPECT_CALL(participant, SendIbMessage(An<const IIbServiceEndpoint*>(), A<const FlexrayTxBufferConfigUpdate &>())).Times(0);
    EXPECT_THROW(controller.UpdateTxBuffer(update), std::out_of_range);
}

TEST_F(FlexrayControllerTest, send_run_command)
{
    EXPECT_CALL(participant, SendIbMessage(&controller, FlexrayHostCommand{FlexrayChiCommand::RUN}))
        .Times(1);

    controller.Run();
}

TEST_F(FlexrayControllerTest, send_deferred_halt_command)
{
    EXPECT_CALL(participant, SendIbMessage(&controller, FlexrayHostCommand{FlexrayChiCommand::DEFERRED_HALT}))
        .Times(1);

    controller.DeferredHalt();
}

TEST_F(FlexrayControllerTest, send_freeze_command)
{
    EXPECT_CALL(participant, SendIbMessage(&controller, FlexrayHostCommand{FlexrayChiCommand::FREEZE}))
        .Times(1);

    controller.Freeze();
}

TEST_F(FlexrayControllerTest, send_allow_coldstart_command)
{
    EXPECT_CALL(participant, SendIbMessage(&controller, FlexrayHostCommand{FlexrayChiCommand::ALLOW_COLDSTART}))
        .Times(1);

    controller.AllowColdstart();
}

TEST_F(FlexrayControllerTest, send_all_slots_command)
{
    FlexrayHostCommand cmd;
    cmd.command = FlexrayChiCommand::ALL_SLOTS;

    EXPECT_CALL(participant, SendIbMessage(&controller, cmd))
        .Times(1);

    controller.AllSlots();
}

TEST_F(FlexrayControllerTest, send_wakeup_command)
{
    EXPECT_CALL(participant, SendIbMessage(&controller, FlexrayHostCommand{FlexrayChiCommand::WAKEUP}))
        .Times(1);

    controller.Wakeup();
}


TEST_F(FlexrayControllerTest, call_message_handler)
{
    controller.AddFrameHandler(bind_method(&callbacks, &Callbacks::MessageHandler));

    FlexrayFrameEvent message;
    message.timestamp = 17ns;
    message.channel = FlexrayChannel::A;
    message.frame.header.frameId = 13;
    message.frame.header.payloadLength = static_cast<uint8_t>(referencePayload.size() / 2);
    message.frame.payload = referencePayload;

    EXPECT_CALL(callbacks, MessageHandler(&controller, message))
        .Times(1);

    controller.ReceiveIbMessage(&controllerBusSim, message);
}

TEST_F(FlexrayControllerTest, call_message_ack_handler)
{
    controller.AddFrameTransmitHandler(bind_method(&callbacks, &Callbacks::MessageAckHandler));

    FlexrayFrameTransmitEvent ack;
    ack.timestamp = 17ns;
    ack.channel = FlexrayChannel::A;
    ack.frame.header.frameId = 13;
    ack.frame.header.payloadLength = static_cast<uint8_t>(referencePayload.size() / 2);
    ack.frame.payload = referencePayload;

    EXPECT_CALL(callbacks, MessageAckHandler(&controller, ack))
        .Times(1);

    controller.ReceiveIbMessage(&controllerBusSim, ack);
}

TEST_F(FlexrayControllerTest, call_wakeup_handler)
{
    controller.AddWakeupHandler(bind_method(&callbacks, &Callbacks::WakeupHandler));

    FlexraySymbolEvent wusSymbolEvent = {};
    wusSymbolEvent.pattern = FlexraySymbolPattern::Wus;
    auto wus = FlexrayWakeupEvent{wusSymbolEvent};
    EXPECT_CALL(callbacks, WakeupHandler(&controller, wus))
        .Times(1);

    FlexraySymbolEvent wudopSymbolEvent = {};
    wudopSymbolEvent.pattern = FlexraySymbolPattern::Wudop;
    auto wudop = FlexrayWakeupEvent{wudopSymbolEvent};
    EXPECT_CALL(callbacks, WakeupHandler(&controller, wudop))
        .Times(1);

    FlexraySymbolEvent casMtsSymbolEvent = {};
    casMtsSymbolEvent.pattern = FlexraySymbolPattern::CasMts;
    auto casMts = FlexrayWakeupEvent{casMtsSymbolEvent};
    EXPECT_CALL(callbacks, WakeupHandler(&controller, casMts))
        .Times(0);

    controller.ReceiveIbMessage(&controllerBusSim, wusSymbolEvent);
    controller.ReceiveIbMessage(&controllerBusSim, wudopSymbolEvent);
    controller.ReceiveIbMessage(&controllerBusSim, casMtsSymbolEvent);
}

TEST_F(FlexrayControllerTest, call_pocstatus_handler)
{
    // new POC API
    controller.AddPocStatusHandler(bind_method(&callbacks, &Callbacks::PocStatusHandler));
    FlexrayPocStatusEvent poc{};
    poc.timestamp = 14ms;
    poc.state = FlexrayPocState::Ready;

    EXPECT_CALL(callbacks, PocStatusHandler(&controller, poc))
        .Times(1);

    controller.ReceiveIbMessage(&controllerBusSim, poc);
}

TEST_F(FlexrayControllerTest, call_symbol_handler)
{
    controller.AddSymbolHandler(bind_method(&callbacks, &Callbacks::SymbolHandler));

    FlexraySymbolEvent wus;
    wus.pattern = FlexraySymbolPattern::Wus;
    EXPECT_CALL(callbacks, SymbolHandler(&controller, wus))
        .Times(1);

    FlexraySymbolEvent wudop;
    wudop.pattern = FlexraySymbolPattern::Wudop;
    EXPECT_CALL(callbacks, SymbolHandler(&controller, wudop))
        .Times(1);

    FlexraySymbolEvent casMts;
    casMts.pattern = FlexraySymbolPattern::CasMts;
    EXPECT_CALL(callbacks, SymbolHandler(&controller, casMts))
        .Times(1);

    controller.ReceiveIbMessage(&controllerBusSim, wus);
    controller.ReceiveIbMessage(&controllerBusSim, wudop);
    controller.ReceiveIbMessage(&controllerBusSim, casMts);
}

TEST_F(FlexrayControllerTest, call_symbol_ack_handler)
{
    controller.AddSymbolTransmitHandler(bind_method(&callbacks, &Callbacks::SymbolAckHandler));

    FlexraySymbolTransmitEvent ack;
    ack.channel = FlexrayChannel::B;
    ack.pattern = FlexraySymbolPattern::CasMts;

    EXPECT_CALL(callbacks, SymbolAckHandler(&controller, ack))
        .Times(1);

    controller.ReceiveIbMessage(&controllerBusSim, ack);
}

TEST_F(FlexrayControllerTest, call_cyclestart_handler)
{
    controller.AddCycleStartHandler(bind_method(&callbacks, &Callbacks::CycleStartHandler));

    FlexrayCycleStartEvent cycleStart;
    cycleStart.timestamp = 15ns;
    cycleStart.cycleCounter = 5u;

    EXPECT_CALL(callbacks, CycleStartHandler(&controller, cycleStart))
        .Times(1);

    controller.ReceiveIbMessage(&controllerBusSim, cycleStart);
}

/*! \brief Multiple handlers added and removed
 */
TEST_F(FlexrayControllerTest, add_remove_handler)
{
    const int numHandlers = 10;
    std::vector<ib::sim::HandlerId> handlerIds;
    for (int i = 0; i < numHandlers; i++)
    {
        handlerIds.push_back(controller.AddFrameHandler(bind_method(&callbacks, &Callbacks::MessageHandler)));
    }

    FlexrayFrameEvent message;
    message.timestamp = 17ns;
    message.channel = FlexrayChannel::A;
    message.frame.header.frameId = 13;
    message.frame.header.payloadLength = static_cast<uint8_t>(referencePayload.size() / 2);
    message.frame.payload = referencePayload;

    EXPECT_CALL(callbacks, MessageHandler(&controller, message)).Times(numHandlers);
    controller.ReceiveIbMessage(&controllerBusSim, message);

    for (auto&& handlerId : handlerIds)
    {
        controller.RemoveFrameHandler(handlerId);
    }

    EXPECT_CALL(callbacks, MessageHandler(&controller, message)).Times(0);
    controller.ReceiveIbMessage(&controllerBusSim, message);
}

} // namespace
