// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#include "FrControllerProxy.hpp"

#include <chrono>
#include <functional>
#include <numeric>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "ib/mw/string_utils.hpp"
#include "ib/util/functional.hpp"

#include "FrDatatypeUtils.hpp"

#include "MockComAdapter.hpp"

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

class FrControllerProxyTest : public testing::Test
{
protected:
    struct Callbacks
    {
        MOCK_METHOD2(MessageHandler, void(IFrController*, const FrMessage&));
        MOCK_METHOD2(MessageAckHandler, void(IFrController*, const FrMessageAck&));
        MOCK_METHOD2(WakeupHandler, void(IFrController*, const FrSymbol&));
        MOCK_METHOD2(ControllerStatusHandler, void(IFrController*, const ControllerStatus&));
        MOCK_METHOD2(SymbolHandler, void(IFrController*, const FrSymbol&));
        MOCK_METHOD2(SymbolAckHandler, void(IFrController*, const FrSymbolAck&));
    };

protected:
    FrControllerProxyTest()
    : proxy(&comAdapter)
    {
        proxy.SetEndpointAddress(proxyAddress);

        referencePayload.resize(20);
        std::iota(referencePayload.begin(), referencePayload.end(), '\000');
    }

protected:
    const EndpointAddress proxyAddress {4, 5};
    const EndpointAddress controllerAddress {9, 5};

    decltype(TxBufferUpdate::payload) referencePayload;

    ib::mw::test::MockComAdapter comAdapter;
    FrControllerProxy proxy;
    Callbacks callbacks;
};

TEST_F(FrControllerProxyTest, send_controller_config)
{
    TxBufferConfig bufferCfg;
    bufferCfg.channels = Channel::AB;
    bufferCfg.hasPayloadPreambleIndicator = false;
    bufferCfg.offset = 0;
    bufferCfg.repetition = 0;
    bufferCfg.slotId = 17;
    bufferCfg.transmissionMode = TransmissionMode::SingleShot;

    // Configure Controller
    ControllerConfig controllerCfg;
    controllerCfg.clusterParams.gCycleCountMax = 5;
    controllerCfg.clusterParams.gdSymbolWindowActionPointOffset = 100;
    controllerCfg.clusterParams.gdMiniSlotActionPointOffset= 9;
    controllerCfg.clusterParams.gMacroPerCycle = 42;
    controllerCfg.clusterParams.gdSymbolWindow = 11;

    controllerCfg.nodeParams.pAllowHaltDueToClock = 13;
    controllerCfg.nodeParams.pdMicrotick = ClockPeriod::T25NS;
    controllerCfg.nodeParams.pKeySlotId = 99;
    controllerCfg.nodeParams.pLatestTx = 50;

    controllerCfg.bufferConfigs.push_back(bufferCfg);

    EXPECT_CALL(comAdapter, SendIbMessage(proxyAddress, controllerCfg))
        .Times(1);

    proxy.Configure(controllerCfg);
}

TEST_F(FrControllerProxyTest, send_txbuffer_update)
{
    TxBufferUpdate update;
    update.payload = referencePayload;
    update.payloadDataValid = true;
    update.txBufferIndex = 7;


    EXPECT_CALL(comAdapter, SendIbMessage(proxyAddress, update))
        .Times(1);

    proxy.UpdateTxBuffer(update);
}

TEST_F(FrControllerProxyTest, send_run_command)
{
    HostCommand cmd;
    cmd.command = ChiCommand::RUN;

    EXPECT_CALL(comAdapter, SendIbMessage(proxyAddress, cmd))
        .Times(1);

    proxy.Run();
}

TEST_F(FrControllerProxyTest, send_deferred_halt_command)
{
    HostCommand cmd;
    cmd.command = ChiCommand::DEFERRED_HALT;

    EXPECT_CALL(comAdapter, SendIbMessage(proxyAddress, cmd))
        .Times(1);

    proxy.DeferredHalt();
}

TEST_F(FrControllerProxyTest, send_freeze_command)
{
    HostCommand cmd;
    cmd.command = ChiCommand::FREEZE;

    EXPECT_CALL(comAdapter, SendIbMessage(proxyAddress, cmd))
        .Times(1);

    proxy.Freeze();
}

TEST_F(FrControllerProxyTest, send_allow_coldstart_command)
{
    HostCommand cmd;
    cmd.command = ChiCommand::ALLOW_COLDSTART;

    EXPECT_CALL(comAdapter, SendIbMessage(proxyAddress, cmd))
        .Times(1);

    proxy.AllowColdstart();
}

TEST_F(FrControllerProxyTest, send_all_slots_command)
{
    HostCommand cmd;
    cmd.command = ChiCommand::ALL_SLOTS;

    EXPECT_CALL(comAdapter, SendIbMessage(proxyAddress, cmd))
        .Times(1);

    proxy.AllSlots();
}

TEST_F(FrControllerProxyTest, send_wakeup_command)
{
    HostCommand cmd;
    cmd.command = ChiCommand::WAKEUP;

    EXPECT_CALL(comAdapter, SendIbMessage(proxyAddress, cmd))
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

    proxy.ReceiveIbMessage(controllerAddress, message);
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

    proxy.ReceiveIbMessage(controllerAddress, ack);
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

    proxy.ReceiveIbMessage(controllerAddress, wus);
    proxy.ReceiveIbMessage(controllerAddress, wudop);
    proxy.ReceiveIbMessage(controllerAddress, casMts);
}

TEST_F(FrControllerProxyTest, call_controller_status_handler)
{
    proxy.RegisterControllerStatusHandler(bind_method(&callbacks, &Callbacks::ControllerStatusHandler));

    ControllerStatus status;
    status.timestamp = 14ms;
    status.pocState = PocState::Ready;

    EXPECT_CALL(callbacks, ControllerStatusHandler(&proxy, status))
        .Times(1);

    proxy.ReceiveIbMessage(controllerAddress, status);
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

    proxy.ReceiveIbMessage(controllerAddress, wus);
    proxy.ReceiveIbMessage(controllerAddress, wudop);
    proxy.ReceiveIbMessage(controllerAddress, casMts);
}

TEST_F(FrControllerProxyTest, call_symbol_ack_handler)
{
    proxy.RegisterSymbolAckHandler(bind_method(&callbacks, &Callbacks::SymbolAckHandler));

    FrSymbolAck ack;
    ack.channel = Channel::B;
    ack.pattern = SymbolPattern::CasMts;

    EXPECT_CALL(callbacks, SymbolAckHandler(&proxy, ack))
        .Times(1);

    proxy.ReceiveIbMessage(controllerAddress, ack);
}



} // namespace
