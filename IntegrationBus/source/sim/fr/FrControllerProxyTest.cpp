// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "FrControllerProxy.hpp"

#include <chrono>
#include <functional>
#include <numeric>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "ib/util/functional.hpp"

#include "FrDatatypeUtils.hpp"

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
    MOCK_METHOD2(SendIbMessage, void(const IIbServiceEndpoint*, const HostCommand&));
    MOCK_METHOD2(SendIbMessage, void(const IIbServiceEndpoint*, const ControllerConfig&));
    MOCK_METHOD2(SendIbMessage, void(const IIbServiceEndpoint*, const TxBufferConfigUpdate&));
    MOCK_METHOD2(SendIbMessage, void(const IIbServiceEndpoint*, const TxBufferUpdate&));
};

class FrControllerProxyTest : public testing::Test
{
protected:
    struct Callbacks
    {
        MOCK_METHOD2(MessageHandler, void(IFrController*, const FrMessage&));
        MOCK_METHOD2(MessageAckHandler, void(IFrController*, const FrMessageAck&));
        MOCK_METHOD2(WakeupHandler, void(IFrController*, const FrSymbol&));
        MOCK_METHOD2(PocStatusHandler, void(IFrController*, const PocStatus&));
        MOCK_METHOD2(SymbolHandler, void(IFrController*, const FrSymbol&));
        MOCK_METHOD2(SymbolAckHandler, void(IFrController*, const FrSymbolAck&));
        MOCK_METHOD2(CycleStartHandler, void(IFrController*, const CycleStart&));
    };

protected:
    FrControllerProxyTest()
        : proxy(&participant, FrControllerProxyTest::GetDummyConfig())
        , proxyFrom(&participant, FrControllerProxyTest::GetDummyConfig())
        , proxyConfigured(&participant, FrControllerProxyTest::GetDummyConfigWithValues())
    {
        proxy.SetServiceDescriptor(from_endpointAddress(proxyAddress));

        referencePayload.resize(20);
        std::iota(referencePayload.begin(), referencePayload.end(), '\000');

        proxyFrom.SetServiceDescriptor(from_endpointAddress(controllerAddress));
    }

protected:
    const EndpointAddress proxyAddress{4, 5};
    const EndpointAddress controllerAddress{9, 5};

    decltype(TxBufferUpdate::payload) referencePayload;

    MockParticipant participant;
    FrControllerProxy proxy;
    FrControllerProxy proxyFrom;
    FrControllerProxy proxyConfigured;
    Callbacks callbacks;

    ib::cfg::FlexRayController dummyConfig;

    auto static GetDummyConfig() -> ib::cfg::FlexRayController 
    {
        ib::cfg::FlexRayController dummyConfig;
        dummyConfig.network = "testNetwork";
        dummyConfig.name = "testController";
        return dummyConfig;
    }
    auto static GetDummyConfigWithValues() -> ib::cfg::FlexRayController
    {
        ib::cfg::FlexRayController dummyConfig;
        dummyConfig.network = "testNetwork";
        dummyConfig.name = "testController";
        dummyConfig.clusterParameters = MakeValidClusterParams();
        dummyConfig.nodeParameters = MakeValidNodeParams();
        dummyConfig.txBufferConfigurations.push_back(MakeValidTxBufferConfig());
        return dummyConfig;
    }

    auto static MakeValidClusterParams() -> ClusterParameters
    {
        ClusterParameters clusterParams;
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

    auto static MakeValidNodeParams() -> NodeParameters
    {
        NodeParameters nodeParams;
        nodeParams.pAllowHaltDueToClock = 0;
        nodeParams.pAllowPassiveToActive = 0;
        nodeParams.pChannels = Channel::A;
        nodeParams.pClusterDriftDamping = 0;
        nodeParams.pdAcceptedStartupRange = 29;
        nodeParams.pdListenTimeout = 1926;
        nodeParams.pdMicrotick = ClockPeriod::T12_5NS;
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
        nodeParams.pWakeupChannel = Channel::A;
        nodeParams.pWakeupPattern = 0;

        return nodeParams;
    }

    auto static MakeValidTxBufferConfig() -> TxBufferConfig 
    {
        TxBufferConfig bufferCfg{};
        bufferCfg.channels = Channel::AB;
        bufferCfg.hasPayloadPreambleIndicator = false;
        bufferCfg.offset = 0;
        bufferCfg.repetition = 0;
        bufferCfg.slotId = 17;
        bufferCfg.transmissionMode = TransmissionMode::SingleShot;
        
        return bufferCfg;
    }
};

TEST_F(FrControllerProxyTest, send_controller_config)
{
    // Configure Controller
    ControllerConfig controllerCfg{};
    controllerCfg.clusterParams = MakeValidClusterParams();
    controllerCfg.nodeParams = MakeValidNodeParams();

    controllerCfg.bufferConfigs.push_back(MakeValidTxBufferConfig());

    EXPECT_CALL(participant, SendIbMessage(&proxy, controllerCfg)).Times(1);

    proxy.Configure(controllerCfg);
}

TEST_F(FrControllerProxyTest, send_controller_config_override_identical)
{
    // Configure Controller
    ControllerConfig configuredControllerCfg{};
    ControllerConfig testControllerCfg{};

    configuredControllerCfg.clusterParams = MakeValidClusterParams();
    configuredControllerCfg.nodeParams = MakeValidNodeParams();
    configuredControllerCfg.bufferConfigs.push_back(MakeValidTxBufferConfig());

    testControllerCfg.clusterParams = MakeValidClusterParams();
    testControllerCfg.nodeParams = MakeValidNodeParams();
    testControllerCfg.bufferConfigs.push_back(MakeValidTxBufferConfig());

    EXPECT_CALL(participant, SendIbMessage(&proxyConfigured, configuredControllerCfg)).Times(1);
    proxyConfigured.Configure(testControllerCfg);
}

TEST_F(FrControllerProxyTest, send_controller_config_override_cluster_params)
{
    // Configure Controller
    ControllerConfig configuredControllerCfg{};
    ControllerConfig testControllerCfg{};

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

    EXPECT_CALL(participant, SendIbMessage(&proxyConfigured, configuredControllerCfg)).Times(1);
    proxyConfigured.Configure(testControllerCfg);
}

TEST_F(FrControllerProxyTest, send_controller_config_override_node_params)
{
    // Configure Controller
    ControllerConfig configuredControllerCfg{};
    ControllerConfig testControllerCfg{};

    configuredControllerCfg.clusterParams = MakeValidClusterParams();
    configuredControllerCfg.nodeParams = MakeValidNodeParams();
    configuredControllerCfg.bufferConfigs.push_back(MakeValidTxBufferConfig());

    // Change clusterParams in tester and make sure they are ignored
    testControllerCfg.nodeParams = MakeValidNodeParams();
    testControllerCfg.nodeParams.pKeySlotId = 42; // was 0
    testControllerCfg.nodeParams.pChannels = Channel::B; // was A

    // default values (not configured)
    testControllerCfg.clusterParams = MakeValidClusterParams();
    testControllerCfg.bufferConfigs.push_back(MakeValidTxBufferConfig());

    EXPECT_CALL(participant, SendIbMessage(&proxyConfigured, configuredControllerCfg)).Times(1);
    proxyConfigured.Configure(testControllerCfg);
}

TEST_F(FrControllerProxyTest, send_controller_config_override_tx_buffer)
{
    // Configure Controller
    ControllerConfig configuredControllerCfg{};
    ControllerConfig testControllerCfg{};

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

    EXPECT_CALL(participant, SendIbMessage(&proxyConfigured, configuredControllerCfg)).Times(1);
    proxyConfigured.Configure(testControllerCfg);
}

TEST_F(FrControllerProxyTest, send_txbuffer_configupdate)
{
    TxBufferConfig bufferCfg{};
    bufferCfg.channels = Channel::AB;
    bufferCfg.hasPayloadPreambleIndicator = false;
    bufferCfg.offset = 0;
    bufferCfg.repetition = 0;
    bufferCfg.slotId = 17;
    bufferCfg.transmissionMode = TransmissionMode::SingleShot;

    ControllerConfig controllerCfg{};
    controllerCfg.clusterParams = MakeValidClusterParams();
    controllerCfg.nodeParams = MakeValidNodeParams();
    controllerCfg.bufferConfigs.push_back(bufferCfg);

    EXPECT_CALL(participant, SendIbMessage(&proxy, controllerCfg)).Times(1);
    proxy.Configure(controllerCfg);

    // Reconfigure TxBuffer 0
    bufferCfg.channels = Channel::A;
    bufferCfg.hasPayloadPreambleIndicator = true;
    bufferCfg.offset = 1;
    bufferCfg.repetition = 3;
    bufferCfg.slotId = 15;
    bufferCfg.transmissionMode = TransmissionMode::Continuous;

    TxBufferConfigUpdate expectedUpdate{};
    expectedUpdate.txBufferIndex = 0;
    expectedUpdate.txBufferConfig = bufferCfg;

    EXPECT_CALL(participant, SendIbMessage(&proxy, expectedUpdate)).Times(1);
    proxy.ReconfigureTxBuffer(0, bufferCfg);
}

TEST_F(FrControllerProxyTest, throw_on_unconfigured_tx_buffer_configupdate)
{
    ControllerConfig controllerCfg;
    controllerCfg.clusterParams = MakeValidClusterParams();
    controllerCfg.nodeParams = MakeValidNodeParams();
    controllerCfg.bufferConfigs.resize(5);

    EXPECT_CALL(participant, SendIbMessage(&proxy, controllerCfg)).Times(1);
    proxy.Configure(controllerCfg);

    // Attempt to reconfigure TxBuffer 6, which should be out of range
    TxBufferConfig bufferCfg{};
    EXPECT_CALL(participant, SendIbMessage(An<const IIbServiceEndpoint*>(), A<const TxBufferConfigUpdate &>())).Times(0);
    EXPECT_THROW(proxy.ReconfigureTxBuffer(6, bufferCfg), std::out_of_range);
}


TEST_F(FrControllerProxyTest, send_txbuffer_update)
{
    TxBufferConfig bufferCfg{};

    ControllerConfig controllerCfg;
    controllerCfg.clusterParams = MakeValidClusterParams();
    controllerCfg.nodeParams = MakeValidNodeParams();
    controllerCfg.bufferConfigs.push_back(bufferCfg);

    EXPECT_CALL(participant, SendIbMessage(&proxy, controllerCfg)).Times(1);
    proxy.Configure(controllerCfg);

    TxBufferUpdate update{};
    update.txBufferIndex = 0;
    update.payload = referencePayload;
    update.payloadDataValid = true;

    EXPECT_CALL(participant, SendIbMessage(&proxy, update))
        .Times(1);

    proxy.UpdateTxBuffer(update);
}


TEST_F(FrControllerProxyTest, throw_on_unconfigured_tx_buffer_update)
{
    ControllerConfig controllerCfg;
    controllerCfg.clusterParams = MakeValidClusterParams();
    controllerCfg.nodeParams = MakeValidNodeParams();
    controllerCfg.bufferConfigs.resize(1);

    EXPECT_CALL(participant, SendIbMessage(&proxy, controllerCfg)).Times(1);
    proxy.Configure(controllerCfg);

    TxBufferUpdate update;
    update.txBufferIndex = 7; // only txBufferIdx = 0 is configured
    EXPECT_CALL(participant, SendIbMessage(An<const IIbServiceEndpoint*>(), A<const TxBufferConfigUpdate &>())).Times(0);
    EXPECT_THROW(proxy.UpdateTxBuffer(update), std::out_of_range);
}

TEST_F(FrControllerProxyTest, send_run_command)
{
    EXPECT_CALL(participant, SendIbMessage(&proxy, HostCommand{ChiCommand::RUN}))
        .Times(1);

    proxy.Run();
}

TEST_F(FrControllerProxyTest, send_deferred_halt_command)
{
    EXPECT_CALL(participant, SendIbMessage(&proxy, HostCommand{ChiCommand::DEFERRED_HALT}))
        .Times(1);

    proxy.DeferredHalt();
}

TEST_F(FrControllerProxyTest, send_freeze_command)
{
    EXPECT_CALL(participant, SendIbMessage(&proxy, HostCommand{ChiCommand::FREEZE}))
        .Times(1);

    proxy.Freeze();
}

TEST_F(FrControllerProxyTest, send_allow_coldstart_command)
{
    EXPECT_CALL(participant, SendIbMessage(&proxy, HostCommand{ChiCommand::ALLOW_COLDSTART}))
        .Times(1);

    proxy.AllowColdstart();
}

TEST_F(FrControllerProxyTest, send_all_slots_command)
{
    HostCommand cmd;
    cmd.command = ChiCommand::ALL_SLOTS;

    EXPECT_CALL(participant, SendIbMessage(&proxy, cmd))
        .Times(1);

    proxy.AllSlots();
}

TEST_F(FrControllerProxyTest, send_wakeup_command)
{
    EXPECT_CALL(participant, SendIbMessage(&proxy, HostCommand{ChiCommand::WAKEUP}))
        .Times(1);

    proxy.Wakeup();
}


TEST_F(FrControllerProxyTest, call_message_handler)
{
    proxy.RegisterMessageHandler(bind_method(&callbacks, &Callbacks::MessageHandler));

    FrMessage message;
    message.timestamp = 17ns;
    message.channel = Channel::A;
    message.frame.header.frameId = 13;
    message.frame.header.payloadLength = static_cast<uint8_t>(referencePayload.size() / 2);
    message.frame.payload = referencePayload;

    EXPECT_CALL(callbacks, MessageHandler(&proxy, message))
        .Times(1);

    proxy.ReceiveIbMessage(&proxyFrom, message);
}

TEST_F(FrControllerProxyTest, call_message_ack_handler)
{
    proxy.RegisterMessageAckHandler(bind_method(&callbacks, &Callbacks::MessageAckHandler));

    FrMessageAck ack;
    ack.timestamp = 17ns;
    ack.channel = Channel::A;
    ack.frame.header.frameId = 13;
    ack.frame.header.payloadLength = static_cast<uint8_t>(referencePayload.size() / 2);
    ack.frame.payload = referencePayload;

    EXPECT_CALL(callbacks, MessageAckHandler(&proxy, ack))
        .Times(1);

    proxy.ReceiveIbMessage(&proxyFrom, ack);
}

TEST_F(FrControllerProxyTest, call_wakeup_handler)
{
    proxy.RegisterWakeupHandler(bind_method(&callbacks, &Callbacks::WakeupHandler));

    FrSymbol wus;
    wus.pattern = SymbolPattern::Wus;
    EXPECT_CALL(callbacks, WakeupHandler(&proxy, wus))
        .Times(1);

    FrSymbol wudop;
    wudop.pattern = SymbolPattern::Wudop;
    EXPECT_CALL(callbacks, WakeupHandler(&proxy, wudop))
        .Times(1);

    FrSymbol casMts;
    casMts.pattern = SymbolPattern::CasMts;
    EXPECT_CALL(callbacks, WakeupHandler(&proxy, casMts))
        .Times(0);

    proxy.ReceiveIbMessage(&proxyFrom, wus);
    proxy.ReceiveIbMessage(&proxyFrom, wudop);
    proxy.ReceiveIbMessage(&proxyFrom, casMts);
}

TEST_F(FrControllerProxyTest, call_pocstatus_handler)
{
    // new POC API
    proxy.RegisterPocStatusHandler(bind_method(&callbacks, &Callbacks::PocStatusHandler));
    PocStatus poc{};
    poc.timestamp = 14ms;
    poc.state = PocState::Ready;

    EXPECT_CALL(callbacks, PocStatusHandler(&proxy, poc))
        .Times(1);

    proxy.ReceiveIbMessage(&proxyFrom, poc);
}

TEST_F(FrControllerProxyTest, call_symbol_handler)
{
    proxy.RegisterSymbolHandler(bind_method(&callbacks, &Callbacks::SymbolHandler));

    FrSymbol wus;
    wus.pattern = SymbolPattern::Wus;
    EXPECT_CALL(callbacks, SymbolHandler(&proxy, wus))
        .Times(1);

    FrSymbol wudop;
    wudop.pattern = SymbolPattern::Wudop;
    EXPECT_CALL(callbacks, SymbolHandler(&proxy, wudop))
        .Times(1);

    FrSymbol casMts;
    casMts.pattern = SymbolPattern::CasMts;
    EXPECT_CALL(callbacks, SymbolHandler(&proxy, casMts))
        .Times(1);

    proxy.ReceiveIbMessage(&proxyFrom, wus);
    proxy.ReceiveIbMessage(&proxyFrom, wudop);
    proxy.ReceiveIbMessage(&proxyFrom, casMts);
}

TEST_F(FrControllerProxyTest, call_symbol_ack_handler)
{
    proxy.RegisterSymbolAckHandler(bind_method(&callbacks, &Callbacks::SymbolAckHandler));

    FrSymbolAck ack;
    ack.channel = Channel::B;
    ack.pattern = SymbolPattern::CasMts;

    EXPECT_CALL(callbacks, SymbolAckHandler(&proxy, ack))
        .Times(1);

    proxy.ReceiveIbMessage(&proxyFrom, ack);
}

TEST_F(FrControllerProxyTest, call_cyclestart_handler)
{
    proxy.RegisterCycleStartHandler(bind_method(&callbacks, &Callbacks::CycleStartHandler));

    CycleStart cycleStart;
    cycleStart.timestamp = 15ns;
    cycleStart.cycleCounter = 5u;

    EXPECT_CALL(callbacks, CycleStartHandler(&proxy, cycleStart))
        .Times(1);

    proxy.ReceiveIbMessage(&proxyFrom, cycleStart);
}

} // namespace
