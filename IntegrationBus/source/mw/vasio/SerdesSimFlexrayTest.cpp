// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "SerdesSimFlexray.hpp"

#include <chrono>
#include "gtest/gtest.h"
#include "gmock/gmock.h"

using namespace std::chrono_literals;

TEST(MwVAsioSerdes, SimFlexray_FrMessage){
    using namespace ib::sim::fr;
    ib::mw::MessageBuffer buffer;

    FrMessage in;
    FrMessage out;

    in.timestamp = 12ns;
    in.channel = Channel::AB;

    Header header;

    header.flags = 1;
    header.frameId = 2;
    header.payloadLength = 3;
    header.headerCrc = 4;
    header.cycleCount = 5;

    in.frame.header = header;

    std::string message{"Hello from flexray!  msgId = 1 -------------"};
    in.frame.payload = std::vector<uint8_t>{message.begin(), message.end()};

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in.timestamp, out.timestamp);
    EXPECT_EQ(in.channel, out.channel);

    EXPECT_EQ(in.frame.header.flags, out.frame.header.flags);
    EXPECT_EQ(in.frame.header.frameId, out.frame.header.frameId);
    EXPECT_EQ(in.frame.header.payloadLength, out.frame.header.payloadLength);
    EXPECT_EQ(in.frame.header.headerCrc, out.frame.header.headerCrc);
    EXPECT_EQ(in.frame.header.cycleCount, out.frame.header.cycleCount);

    EXPECT_EQ(in.frame.payload, out.frame.payload);
}

TEST(MwVAsioSerdes, SimFlexray_FrMessageAck) {
    using namespace ib::sim::fr;
    ib::mw::MessageBuffer buffer;

    FrMessageAck in;
    FrMessageAck out;

    in.timestamp = 23ns;
    in.txBufferIndex = 20000;
    in.channel = Channel::B;

    Header header;

    header.flags = 1;
    header.frameId = 2;
    header.payloadLength = 3;
    header.headerCrc = 4;
    header.cycleCount = 5;

    in.frame.header = header;

    std::string message{"Hello acknowledgement sent!"};
    in.frame.payload = std::vector<uint8_t>{message.begin(), message.end()};

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in.timestamp, out.timestamp);
    EXPECT_EQ(in.txBufferIndex, out.txBufferIndex);
    EXPECT_EQ(in.channel, out.channel);

    EXPECT_EQ(in.frame.header.flags, out.frame.header.flags);
    EXPECT_EQ(in.frame.header.frameId, out.frame.header.frameId);
    EXPECT_EQ(in.frame.header.payloadLength, out.frame.header.payloadLength);
    EXPECT_EQ(in.frame.header.headerCrc, out.frame.header.headerCrc);
    EXPECT_EQ(in.frame.header.cycleCount, out.frame.header.cycleCount);

    EXPECT_EQ(in.frame.payload, out.frame.payload);
}

TEST(MwVAsioSerdes, SimFlexray_FrSymbol) {
    using namespace ib::sim::fr;
    ib::mw::MessageBuffer buffer;

    FrSymbol in;
    FrSymbol out;

    in.timestamp = 23ns;
    in.channel = Channel::B;
    in.pattern = SymbolPattern::Wus;

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in.timestamp, out.timestamp);
    EXPECT_EQ(in.channel, out.channel);
    EXPECT_EQ(in.pattern, out.pattern);
}

TEST(MwVAsioSerdes, SimFlexray_FrSymbolAck) {
    using namespace ib::sim::fr;
    ib::mw::MessageBuffer buffer;

    FrSymbolAck in;
    FrSymbolAck out;

    in.timestamp = 1ns;
    in.channel = Channel::A;
    in.pattern = SymbolPattern::Wudop;

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in.timestamp, out.timestamp);
    EXPECT_EQ(in.channel, out.channel);
    EXPECT_EQ(in.pattern, out.pattern);
}

TEST(MwVAsioSerdes, SimFlexray_CycleStart) {
    using namespace ib::sim::fr;
    ib::mw::MessageBuffer buffer;

    CycleStart in;
    CycleStart out;

    in.timestamp = 1ns;
    in.cycleCounter = 32;

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in.timestamp, out.timestamp);
    EXPECT_EQ(in.cycleCounter, out.cycleCounter);
}

TEST(MwVAsioSerdes, SimFlexray_HostCommand) {
    using namespace ib::sim::fr;
    ib::mw::MessageBuffer buffer;

    HostCommand in;
    HostCommand out;

    in.command = ChiCommand::DEFERRED_HALT;

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in.command, out.command);
}

TEST(MwVAsioSerdes, SimFlexray_ControllerConfig) {
    using namespace ib::sim::fr;
    ib::mw::MessageBuffer buffer;

    ControllerConfig in;
    ControllerConfig out;

    in.clusterParams.gColdstartAttempts = 1;
    in.clusterParams.gCycleCountMax = 2;
    in.clusterParams.gdActionPointOffset = 3;
    in.clusterParams.gdDynamicSlotIdlePhase = 7;
    in.clusterParams.gdMiniSlot = 7;
    in.clusterParams.gdMiniSlotActionPointOffset = 7;
    in.clusterParams.gdStaticSlot = 7;
    in.clusterParams.gdSymbolWindow = 7;
    in.clusterParams.gdSymbolWindowActionPointOffset = 7;
    in.clusterParams.gdTSSTransmitter = 7;
    in.clusterParams.gdWakeupTxActive = 7;
    in.clusterParams.gdWakeupTxIdle = 7;
    in.clusterParams.gListenNoise = 7;
    in.clusterParams.gMacroPerCycle = 7;
    in.clusterParams.gMaxWithoutClockCorrectionFatal = 7;
    in.clusterParams.gMaxWithoutClockCorrectionPassive = 7;
    in.clusterParams.gNumberOfMiniSlots = 7;
    in.clusterParams.gNumberOfStaticSlots = 7;
    in.clusterParams.gPayloadLengthStatic = 7;
    in.clusterParams.gSyncFrameIDCountMax = 7;

    in.nodeParams.pAllowHaltDueToClock = 7;
    in.nodeParams.pAllowPassiveToActive = 7;
    in.nodeParams.pChannels = Channel::AB;
    in.nodeParams.pClusterDriftDamping = 7;
    in.nodeParams.pdAcceptedStartupRange = 7;
    in.nodeParams.pdListenTimeout = 7;
    in.nodeParams.pdMicrotick = ClockPeriod::T25NS;
    in.nodeParams.pKeySlotId = 7;
    in.nodeParams.pKeySlotOnlyEnabled = 7;
    in.nodeParams.pKeySlotUsedForStartup = 7;
    in.nodeParams.pKeySlotUsedForSync = 7;
    in.nodeParams.pLatestTx = 7;
    in.nodeParams.pMacroInitialOffsetA = 7;
    in.nodeParams.pMacroInitialOffsetB = 7;
    in.nodeParams.pMicroInitialOffsetA = 7;
    in.nodeParams.pMicroInitialOffsetB = 7;
    in.nodeParams.pMicroPerCycle = 7;
    in.nodeParams.pOffsetCorrectionOut = 7;
    in.nodeParams.pOffsetCorrectionStart = 7;
    in.nodeParams.pRateCorrectionOut = 7;
    in.nodeParams.pSamplesPerMicrotick = 7;
    in.nodeParams.pWakeupChannel = Channel::B;
    in.nodeParams.pWakeupPattern = 7;

    TxBufferConfig conf1;
    conf1.channels = Channel::A;
    conf1.hasPayloadPreambleIndicator = false;
    conf1.headerCrc = 12345;
    conf1.offset = 1;
    conf1.repetition = 1;
    conf1.slotId = 1;
    conf1.transmissionMode = TransmissionMode::SingleShot;

    in.bufferConfigs = std::vector<TxBufferConfig>{conf1};

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in.clusterParams.gColdstartAttempts, out.clusterParams.gColdstartAttempts);
    EXPECT_EQ(in.clusterParams.gCycleCountMax, out.clusterParams.gCycleCountMax);
    EXPECT_EQ(in.clusterParams.gdActionPointOffset, out.clusterParams.gdActionPointOffset);
    EXPECT_EQ(in.clusterParams.gdDynamicSlotIdlePhase, out.clusterParams.gdDynamicSlotIdlePhase);
    EXPECT_EQ(in.clusterParams.gdMiniSlot, out.clusterParams.gdMiniSlot);
    EXPECT_EQ(in.clusterParams.gdMiniSlotActionPointOffset, out.clusterParams.gdMiniSlotActionPointOffset);
    EXPECT_EQ(in.clusterParams.gdStaticSlot, out.clusterParams.gdStaticSlot);
    EXPECT_EQ(in.clusterParams.gdSymbolWindow, out.clusterParams.gdSymbolWindow);
    EXPECT_EQ(in.clusterParams.gdSymbolWindowActionPointOffset, out.clusterParams.gdSymbolWindowActionPointOffset);
    EXPECT_EQ(in.clusterParams.gdTSSTransmitter, out.clusterParams.gdTSSTransmitter);
    EXPECT_EQ(in.clusterParams.gdWakeupTxActive, out.clusterParams.gdWakeupTxActive);
    EXPECT_EQ(in.clusterParams.gdWakeupTxIdle, out.clusterParams.gdWakeupTxIdle);
    EXPECT_EQ(in.clusterParams.gListenNoise, out.clusterParams.gListenNoise);
    EXPECT_EQ(in.clusterParams.gMacroPerCycle, out.clusterParams.gMacroPerCycle);
    EXPECT_EQ(in.clusterParams.gMaxWithoutClockCorrectionFatal, out.clusterParams.gMaxWithoutClockCorrectionFatal);
    EXPECT_EQ(in.clusterParams.gMaxWithoutClockCorrectionPassive, out.clusterParams.gMaxWithoutClockCorrectionPassive);
    EXPECT_EQ(in.clusterParams.gNumberOfMiniSlots, out.clusterParams.gNumberOfMiniSlots);
    EXPECT_EQ(in.clusterParams.gNumberOfStaticSlots, out.clusterParams.gNumberOfStaticSlots);
    EXPECT_EQ(in.clusterParams.gPayloadLengthStatic, out.clusterParams.gPayloadLengthStatic);
    EXPECT_EQ(in.clusterParams.gSyncFrameIDCountMax, out.clusterParams.gSyncFrameIDCountMax);

    EXPECT_EQ(in.nodeParams.pAllowHaltDueToClock, out.nodeParams.pAllowHaltDueToClock);
    EXPECT_EQ(in.nodeParams.pAllowPassiveToActive, out.nodeParams.pAllowPassiveToActive);
    EXPECT_EQ(in.nodeParams.pChannels, out.nodeParams.pChannels);
    EXPECT_EQ(in.nodeParams.pClusterDriftDamping, out.nodeParams.pClusterDriftDamping);
    EXPECT_EQ(in.nodeParams.pdAcceptedStartupRange, out.nodeParams.pdAcceptedStartupRange);
    EXPECT_EQ(in.nodeParams.pdListenTimeout, out.nodeParams.pdListenTimeout);
    EXPECT_EQ(in.nodeParams.pdMicrotick, out.nodeParams.pdMicrotick);
    EXPECT_EQ(in.nodeParams.pKeySlotId, out.nodeParams.pKeySlotId);
    EXPECT_EQ(in.nodeParams.pKeySlotOnlyEnabled, out.nodeParams.pKeySlotOnlyEnabled);
    EXPECT_EQ(in.nodeParams.pKeySlotUsedForStartup, out.nodeParams.pKeySlotUsedForStartup);
    EXPECT_EQ(in.nodeParams.pKeySlotUsedForSync, out.nodeParams.pKeySlotUsedForSync);
    EXPECT_EQ(in.nodeParams.pLatestTx, out.nodeParams.pLatestTx);
    EXPECT_EQ(in.nodeParams.pMacroInitialOffsetA, out.nodeParams.pMacroInitialOffsetA);
    EXPECT_EQ(in.nodeParams.pMacroInitialOffsetB, out.nodeParams.pMacroInitialOffsetB);
    EXPECT_EQ(in.nodeParams.pMicroInitialOffsetA, out.nodeParams.pMicroInitialOffsetA);
    EXPECT_EQ(in.nodeParams.pMicroInitialOffsetB, out.nodeParams.pMicroInitialOffsetB);
    EXPECT_EQ(in.nodeParams.pMicroPerCycle, out.nodeParams.pMicroPerCycle);
    EXPECT_EQ(in.nodeParams.pOffsetCorrectionOut, out.nodeParams.pOffsetCorrectionOut);
    EXPECT_EQ(in.nodeParams.pOffsetCorrectionStart, out.nodeParams.pOffsetCorrectionStart);
    EXPECT_EQ(in.nodeParams.pRateCorrectionOut, out.nodeParams.pRateCorrectionOut);
    EXPECT_EQ(in.nodeParams.pSamplesPerMicrotick, out.nodeParams.pSamplesPerMicrotick);
    EXPECT_EQ(in.nodeParams.pWakeupChannel, out.nodeParams.pWakeupChannel);
    EXPECT_EQ(in.nodeParams.pWakeupPattern, out.nodeParams.pWakeupPattern);

    EXPECT_EQ(in.bufferConfigs.size(), out.bufferConfigs.size());

    for (uint8_t i = 0; i < in.bufferConfigs.size(); i++)
    {
        EXPECT_EQ(in.bufferConfigs[i].channels, out.bufferConfigs[i].channels);
        EXPECT_EQ(in.bufferConfigs[i].hasPayloadPreambleIndicator, out.bufferConfigs[i].hasPayloadPreambleIndicator);
        EXPECT_EQ(in.bufferConfigs[i].headerCrc, out.bufferConfigs[i].headerCrc);
        EXPECT_EQ(in.bufferConfigs[i].offset, out.bufferConfigs[i].offset);
        EXPECT_EQ(in.bufferConfigs[i].repetition, out.bufferConfigs[i].repetition);
        EXPECT_EQ(in.bufferConfigs[i].slotId, out.bufferConfigs[i].slotId);
        EXPECT_EQ(in.bufferConfigs[i].transmissionMode, out.bufferConfigs[i].transmissionMode);
    }
}

TEST(MwVAsioSerdes, SimFlexray_TxBufferUpdate) {
    using namespace ib::sim::fr;
    ib::mw::MessageBuffer buffer;

    TxBufferUpdate in;
    TxBufferUpdate out;

    in.txBufferIndex = 23000;
    in.payloadDataValid = true;
    in.payload = std::vector<uint8_t>{1, 2, 3};

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in.txBufferIndex, out.txBufferIndex);
    EXPECT_EQ(in.payloadDataValid, out.payloadDataValid);
    EXPECT_EQ(in.payload, out.payload);
}

TEST(MwVAsioSerdes, SimFlexray_ControllerStatus) {
    using namespace ib::sim::fr;
    ib::mw::MessageBuffer buffer;

    ControllerStatus in;
    ControllerStatus out;

    in.timestamp = 230ns;
    in.pocState = PocState::NormalPassive;

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in.timestamp, out.timestamp);
    EXPECT_EQ(in.pocState, out.pocState);
}