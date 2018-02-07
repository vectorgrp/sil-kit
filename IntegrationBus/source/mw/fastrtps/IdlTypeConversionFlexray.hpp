// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include <stdexcept>

#include "ib/sim/fr/FrDatatypes.hpp"

#include "idl/FlexRayTopics.h"
#include "idl/FlexRayTopicsPubSubTypes.h"


namespace ib {
namespace sim {
namespace fr {

inline auto to_idl(Channel msg) -> idl::Channel;
inline auto to_idl(ClockPeriod msg) -> idl::ClockPeriod;
inline auto to_idl(TransmissionMode msg) -> idl::TransmissionMode;
inline auto to_idl(SymbolPattern msg) -> idl::SymbolPattern;
inline auto to_idl(ChiCommand msg) -> idl::ChiCommand;
inline auto to_idl(PocState msg) -> idl::PocState;

inline auto to_idl(const Header& msg) -> idl::Header;
inline auto to_idl(const Frame& msg) -> idl::Frame;
inline auto to_idl(Frame&& msg) -> idl::Frame;
inline auto to_idl(const FrMessage& msg) -> idl::FrMessage;
inline auto to_idl(FrMessage&& msg) -> idl::FrMessage;
inline auto to_idl(const FrMessageAck& msg) -> idl::FrMessageAck;
inline auto to_idl(FrMessageAck&& msg) -> idl::FrMessageAck;
inline auto to_idl(const FrSymbol& msg) -> idl::FrSymbol;
inline auto to_idl(const FrSymbolAck& msg) -> idl::FrSymbolAck;
inline auto to_idl(const HostCommand& msg) -> idl::HostCommand;
inline auto to_idl(const ClusterParameters& msg) -> idl::ClusterParameters;
inline auto to_idl(const NodeParameters& msg) -> idl::NodeParameters;
inline auto to_idl(const TxBufferConfig& msg) -> idl::TxBufferConfig;
inline auto to_idl(const ControllerConfig& msg) -> idl::ControllerConfig;
inline auto to_idl(const TxBufferUpdate& msg) -> idl::TxBufferUpdate;
inline auto to_idl(TxBufferUpdate&& msg)->idl::TxBufferUpdate;
inline auto to_idl(const ControllerStatus& msg) -> idl::ControllerStatus;


namespace idl {
inline auto from_idl(Channel idl) -> fr::Channel;
inline auto from_idl(ClockPeriod idl) -> fr::ClockPeriod;
inline auto from_idl(TransmissionMode idl) -> fr::TransmissionMode;
inline auto from_idl(SymbolPattern idl) -> fr::SymbolPattern;
inline auto from_idl(ChiCommand idl) -> fr::ChiCommand;
inline auto from_idl(PocState idl) -> fr::PocState;

inline auto from_idl(Header&& idl) -> fr::Header;
inline auto from_idl(Frame&& idl) -> fr::Frame;
inline auto from_idl(FrMessage&& idl) -> fr::FrMessage;
inline auto from_idl(FrMessageAck&& idl) -> fr::FrMessageAck;
inline auto from_idl(FrSymbol&& idl) -> fr::FrSymbol;
inline auto from_idl(FrSymbolAck&& idl) -> fr::FrSymbolAck;
inline auto from_idl(HostCommand&& idl) -> fr::HostCommand;
inline auto from_idl(ClusterParameters&& idl) -> fr::ClusterParameters;
inline auto from_idl(NodeParameters&& idl) -> fr::NodeParameters;
inline auto from_idl(TxBufferConfig&& idl) -> fr::TxBufferConfig;
inline auto from_idl(ControllerConfig&& idl) -> fr::ControllerConfig;
inline auto from_idl(TxBufferUpdate&& idl) -> fr::TxBufferUpdate;
inline auto from_idl(ControllerStatus&& idl) -> fr::ControllerStatus;

} // namespace idl

// ================================================================================
//  Inline Implementations
// ================================================================================
auto to_idl(Channel msg) -> idl::Channel
{
    switch (msg)
    {
    case Channel::None:
        return idl::Channel::None;
    case Channel::A:
        return idl::Channel::A;
    case Channel::B:
        return idl::Channel::B;
    case Channel::AB:
        return idl::Channel::AB;
    default:
        throw std::logic_error("Unhandled case of ib::sim::fr::Channel in conversion to_idl.");
    }
}

auto idl::from_idl(idl::Channel idl) -> fr::Channel
{
    switch (idl)
    {
    case Channel::None:
        return fr::Channel::None;
    case Channel::A:
        return fr::Channel::A;
    case Channel::B:
        return fr::Channel::B;
    case Channel::AB:
        return fr::Channel::AB;
    default:
        throw std::logic_error("Unhandled case of ib::sim::fr::idl::Channel in conversion from_idl.");
    }
}

auto to_idl(ClockPeriod msg) -> idl::ClockPeriod
{
    switch (msg)
    {
    case ClockPeriod::T12_5NS:
        return idl::ClockPeriod::T12_5NS;
    case ClockPeriod::T25NS:
        return idl::ClockPeriod::T25NS;
    case ClockPeriod::T50NS:
        return idl::ClockPeriod::T50NS;
    default:
        throw std::logic_error("Unhandled case of ib::sim::ClockPeriod::Channel in conversion to_idl.");
    }
}

auto idl::from_idl(idl::ClockPeriod idl) -> fr::ClockPeriod
{
    switch (idl)
    {
    case idl::ClockPeriod::T12_5NS:
        return fr::ClockPeriod::T12_5NS;
    case idl::ClockPeriod::T25NS:
        return fr::ClockPeriod::T25NS;
    case idl::ClockPeriod::T50NS:
        return fr::ClockPeriod::T50NS;
    default:
        throw std::logic_error("Unhandled case of ib::sim::ClockPeriod::idl::Channel in conversion from_idl.");
    }
}

auto to_idl(TransmissionMode msg) -> idl::TransmissionMode
{
    switch (msg)
    {
    case TransmissionMode::SingleShot:
        return idl::TransmissionMode::SingleShot;
    case TransmissionMode::Continuous:
        return idl::TransmissionMode::Continuous;
    default:
        throw std::logic_error("Unhandled case of ib::sim::fr::TransmissionMode in conversion to_idl.");
    }
}

auto idl::from_idl(idl::TransmissionMode idl) -> fr::TransmissionMode
{
    switch (idl)
    {
    case TransmissionMode::SingleShot:
        return fr::TransmissionMode::SingleShot;
    case TransmissionMode::Continuous:
        return fr::TransmissionMode::Continuous;
    default:
        throw std::logic_error("Unhandled case of ib::sim::fr::idl::TransmissionMode in conversion from_idl.");
    }
}

auto to_idl(SymbolPattern msg) -> idl::SymbolPattern
{
    switch (msg)
    {
    case SymbolPattern::CasMts:
        return idl::SymbolPattern::CasMts;
    case SymbolPattern::Wus:
        return idl::SymbolPattern::Wus;
    case SymbolPattern::Wudop:
        return idl::SymbolPattern::Wudop;
    default:
        throw std::logic_error("Unhandled case of ib::sim::fr::SymbolPattern in conversion to_idl.");
    }
}

auto idl::from_idl(idl::SymbolPattern idl) -> fr::SymbolPattern
{
    switch (idl)
    {
    case idl::SymbolPattern::CasMts:
        return fr::SymbolPattern::CasMts;
    case idl::SymbolPattern::Wus:
        return fr::SymbolPattern::Wus;
    case idl::SymbolPattern::Wudop:
        return fr::SymbolPattern::Wudop;
    default:
        throw std::logic_error("Unhandled case of ib::sim::fr::idl::SymbolPattern in conversion from_idl.");
    }
}

auto to_idl(ChiCommand msg) -> idl::ChiCommand
{
    switch (msg)
    {
    case ChiCommand::RUN:
        return idl::ChiCommand::RUN;
    case ChiCommand::DEFERRED_HALT:
        return idl::ChiCommand::DEFERRED_HALT;
    case ChiCommand::FREEZE:
        return idl::ChiCommand::FREEZE;
    case ChiCommand::ALLOW_COLDSTART:
        return idl::ChiCommand::ALLOW_COLDSTART;
    case ChiCommand::ALL_SLOTS:
        return idl::ChiCommand::ALL_SLOTS;
    case ChiCommand::WAKEUP:
        return idl::ChiCommand::WAKEUP;
    default:
        throw std::logic_error("Unhandled case of ib::sim::fr::ChiCommand in conversion to_idl.");
    }
}

auto idl::from_idl(idl::ChiCommand idl) -> fr::ChiCommand
{
    switch (idl)
    {
    case idl::ChiCommand::RUN:
        return fr::ChiCommand::RUN;
    case idl::ChiCommand::DEFERRED_HALT:
        return fr::ChiCommand::DEFERRED_HALT;
    case idl::ChiCommand::FREEZE:
        return fr::ChiCommand::FREEZE;
    case idl::ChiCommand::ALLOW_COLDSTART:
        return fr::ChiCommand::ALLOW_COLDSTART;
    case idl::ChiCommand::ALL_SLOTS:
        return fr::ChiCommand::ALL_SLOTS;
    case idl::ChiCommand::WAKEUP:
        return fr::ChiCommand::WAKEUP;
    default:
        throw std::logic_error("Unhandled case of ib::sim::fr::idl::ChiCommand in conversion from_idl.");
    }
}

auto to_idl(PocState msg) -> idl::PocState
{
    switch (msg)
    {
    case PocState::DefaultConfig:
        return idl::PocState::DefaultConfig;
    case PocState::Config:
        return idl::PocState::Config;
    case PocState::Ready:
        return idl::PocState::Ready;
    case PocState::Startup:
        return idl::PocState::Startup;
    case PocState::Wakeup:
        return idl::PocState::Wakeup;
    case PocState::NormalActive:
        return idl::PocState::NormalActive;
    case PocState::NormalPassive:
        return idl::PocState::NormalPassive;
    case PocState::Halt:
        return idl::PocState::Halt;
    default:
        throw std::logic_error("Unhandled case of ib::sim::fr::PocState in conversion to_idl.");
    }
}

auto idl::from_idl(idl::PocState idl) -> fr::PocState
{
    switch (idl)
    {
    case idl::PocState::DefaultConfig:
        return fr::PocState::DefaultConfig;
    case idl::PocState::Config:
        return fr::PocState::Config;
    case idl::PocState::Ready:
        return fr::PocState::Ready;
    case idl::PocState::Startup:
        return fr::PocState::Startup;
    case idl::PocState::Wakeup:
        return fr::PocState::Wakeup;
    case idl::PocState::NormalActive:
        return fr::PocState::NormalActive;
    case idl::PocState::NormalPassive:
        return fr::PocState::NormalPassive;
    case idl::PocState::Halt:
        return fr::PocState::Halt;
    default:
        throw std::logic_error("Unhandled case of ib::sim::fr::idl::PocState in conversion from_idl.");
    }
}

auto to_idl(const Header& msg) -> idl::Header
{
    idl::Header idl;

    idl.flags(msg.flags);
    idl.frameId(msg.frameId);
    idl.payloadLength(msg.payloadLength);
    idl.headerCrc(msg.headerCrc);
    idl.cycleCount(msg.cycleCount);

    return idl;
}

auto idl::from_idl(idl::Header&& idl) -> fr::Header
{
    fr::Header msg;

    msg.flags = idl.flags();
    msg.frameId = idl.frameId();
    msg.payloadLength = idl.payloadLength();
    msg.headerCrc = idl.headerCrc();
    msg.cycleCount = idl.cycleCount();

    return msg;
}

auto to_idl(const Frame& msg) -> idl::Frame
{
    idl::Frame idl;

    idl.header(to_idl(msg.header));
    idl.payload(msg.payload);

    return idl;
}

auto to_idl(Frame&& msg) -> idl::Frame
{
    idl::Frame idl;

    idl.header(to_idl(std::move(msg.header)));
    idl.payload(std::move(msg.payload));

    return idl;
}

auto idl::from_idl(idl::Frame&& idl) -> fr::Frame
{
    fr::Frame msg;

    msg.header = from_idl(std::move(idl.header()));
    msg.payload = std::move(idl.payload());

    return msg;
}

auto to_idl(const FrMessage& msg) -> idl::FrMessage
{
    idl::FrMessage idl;

    idl.timeNs(msg.timestamp.count());
    idl.channel(to_idl(msg.channel));
    idl.frame(to_idl(msg.frame));

    return idl;
}

auto to_idl(FrMessage&& msg) -> idl::FrMessage
{
    idl::FrMessage idl;

    idl.timeNs(msg.timestamp.count());
    idl.channel(to_idl(std::move(msg.channel)));
    idl.frame(to_idl(std::move(msg.frame)));

    return idl;
}

auto idl::from_idl(idl::FrMessage&& idl) -> fr::FrMessage
{
    fr::FrMessage msg;

    msg.timestamp = std::chrono::nanoseconds{idl.timeNs()};
    msg.channel = from_idl(idl.channel());
    msg.frame = from_idl(std::move(idl.frame()));

    return msg;
}

auto to_idl(const FrMessageAck& msg) -> idl::FrMessageAck
{
    idl::FrMessageAck idl;

    idl.timeNs(msg.timestamp.count());
    idl.txBufferIndex(msg.txBufferIndex);
    idl.channel(to_idl(msg.channel));
    idl.frame(to_idl(msg.frame));

    return idl;
}

auto to_idl(FrMessageAck&& msg) -> idl::FrMessageAck
{
    idl::FrMessageAck idl;

    idl.timeNs(msg.timestamp.count());
    idl.txBufferIndex(msg.txBufferIndex);
    idl.channel(to_idl(msg.channel));
    idl.frame(to_idl(std::move(msg.frame)));

    return idl;
}

auto idl::from_idl(idl::FrMessageAck&& idl) -> fr::FrMessageAck
{
    fr::FrMessageAck msg;

    msg.timestamp = std::chrono::nanoseconds{idl.timeNs()};
    msg.txBufferIndex = idl.txBufferIndex();
    msg.channel = from_idl(idl.channel());
    msg.frame = from_idl(std::move(idl.frame()));

    return msg;
}

auto to_idl(const FrSymbol& msg) -> idl::FrSymbol
{
    idl::FrSymbol idl;

    idl.timeNs(msg.timestamp.count());
    idl.channel(to_idl(msg.channel));
    idl.pattern(to_idl(msg.pattern));

    return idl;
}

auto idl::from_idl(idl::FrSymbol&& idl) -> fr::FrSymbol
{
    fr::FrSymbol msg;

    msg.timestamp = std::chrono::nanoseconds{idl.timeNs()};
    msg.channel = from_idl(idl.channel());
    msg.pattern = from_idl(idl.pattern());

    return msg;
}

auto to_idl(const FrSymbolAck& msg) -> idl::FrSymbolAck
{
    idl::FrSymbolAck idl;

    idl.symbol(to_idl(static_cast<const FrSymbol&>(msg)));

    return idl;
}

auto idl::from_idl(idl::FrSymbolAck&& idl) -> fr::FrSymbolAck
{
    fr::FrSymbolAck msg;

    msg.timestamp = std::chrono::nanoseconds{ idl.symbol().timeNs() };
    msg.channel = from_idl(idl.symbol().channel());
    msg.pattern = from_idl(idl.symbol().pattern());

    return msg;
}

auto to_idl(const HostCommand& msg) -> idl::HostCommand
{
    idl::HostCommand idl;

    idl.command(to_idl(msg.command));

    return idl;
}

auto idl::from_idl(idl::HostCommand&& idl) -> fr::HostCommand
{
    fr::HostCommand msg;

    msg.command = from_idl(idl.command());

    return msg;
}

auto to_idl(const ClusterParameters& msg) -> idl::ClusterParameters
{
    idl::ClusterParameters idl;
    
    idl.gColdstartAttempts(msg.gColdstartAttempts);
    idl.gCycleCountMax(msg.gCycleCountMax);
    idl.gdActionPointOffset(msg.gdActionPointOffset);
    idl.gdDynamicSlotIdlePhase(msg.gdDynamicSlotIdlePhase);
    idl.gdMiniSlot(msg.gdMiniSlot);
    idl.gdMiniSlotActionPointOffset(msg.gdMiniSlotActionPointOffset);
    idl.gdStaticSlot(msg.gdStaticSlot);
    idl.gdSymbolWindow(msg.gdSymbolWindow);
    idl.gdSymbolWindowActionPointOffset(msg.gdSymbolWindowActionPointOffset);
    idl.gdTSSTransmitter(msg.gdTSSTransmitter);
    idl.gdWakeupTxActive(msg.gdWakeupTxActive);
    idl.gdWakeupTxIdle(msg.gdWakeupTxIdle);
    idl.gListenNoise(msg.gListenNoise);
    idl.gMacroPerCycle(msg.gMacroPerCycle);
    idl.gMaxWithoutClockCorrectionFatal(msg.gMaxWithoutClockCorrectionFatal);
    idl.gMaxWithoutClockCorrectionPassive(msg.gMaxWithoutClockCorrectionPassive);
    idl.gNumberOfMiniSlots(msg.gNumberOfMiniSlots);
    idl.gNumberOfStaticSlots(msg.gNumberOfStaticSlots);
    idl.gPayloadLengthStatic(msg.gPayloadLengthStatic);
    idl.gSyncFrameIDCountMax(msg.gSyncFrameIDCountMax);

    return idl;
}

auto idl::from_idl(idl::ClusterParameters&& idl) -> fr::ClusterParameters
{
    fr::ClusterParameters msg;
    
    msg.gColdstartAttempts = idl.gColdstartAttempts();
    msg.gCycleCountMax = idl.gCycleCountMax();
    msg.gdActionPointOffset = idl.gdActionPointOffset();
    msg.gdDynamicSlotIdlePhase = idl.gdDynamicSlotIdlePhase();
    msg.gdMiniSlot = idl.gdMiniSlot();
    msg.gdMiniSlotActionPointOffset = idl.gdMiniSlotActionPointOffset();
    msg.gdStaticSlot = idl.gdStaticSlot();
    msg.gdSymbolWindow = idl.gdSymbolWindow();
    msg.gdSymbolWindowActionPointOffset = idl.gdSymbolWindowActionPointOffset();
    msg.gdTSSTransmitter = idl.gdTSSTransmitter();
    msg.gdWakeupTxActive = idl.gdWakeupTxActive();
    msg.gdWakeupTxIdle = idl.gdWakeupTxIdle();
    msg.gListenNoise = idl.gListenNoise();
    msg.gMacroPerCycle = idl.gMacroPerCycle();
    msg.gMaxWithoutClockCorrectionFatal = idl.gMaxWithoutClockCorrectionFatal();
    msg.gMaxWithoutClockCorrectionPassive = idl.gMaxWithoutClockCorrectionPassive();
    msg.gNumberOfMiniSlots = idl.gNumberOfMiniSlots();
    msg.gNumberOfStaticSlots = idl.gNumberOfStaticSlots();
    msg.gPayloadLengthStatic = idl.gPayloadLengthStatic();
    msg.gSyncFrameIDCountMax = idl.gSyncFrameIDCountMax();

    return msg;
}

auto to_idl(const NodeParameters& msg) -> idl::NodeParameters
{
    idl::NodeParameters idl;

    idl.pAllowHaltDueToClock(msg.pAllowHaltDueToClock);
    idl.pAllowPassiveToActive(msg.pAllowPassiveToActive);
    idl.pChannels(to_idl(msg.pChannels));
    idl.pClusterDriftDamping(msg.pClusterDriftDamping);
    idl.pdAcceptedStartupRange(msg.pdAcceptedStartupRange);
    idl.pdListenTimeout(msg.pdListenTimeout);
    idl.pKeySlotId(msg.pKeySlotId);
    idl.pKeySlotOnlyEnabled(msg.pKeySlotOnlyEnabled);
    idl.pKeySlotUsedForStartup(msg.pKeySlotUsedForStartup);
    idl.pKeySlotUsedForSync(msg.pKeySlotUsedForSync);
    idl.pLatestTx(msg.pLatestTx);
    idl.pMacroInitialOffsetA(msg.pMacroInitialOffsetA);
    idl.pMacroInitialOffsetB(msg.pMacroInitialOffsetB);
    idl.pMicroInitialOffsetA(msg.pMicroInitialOffsetA);
    idl.pMicroInitialOffsetB(msg.pMicroInitialOffsetB);
    idl.pMicroPerCycle(msg.pMicroPerCycle);
    idl.pOffsetCorrectionOut(msg.pOffsetCorrectionOut);
    idl.pOffsetCorrectionStart(msg.pOffsetCorrectionStart);
    idl.pRateCorrectionOut(msg.pRateCorrectionOut);
    idl.pWakeupChannel(to_idl(msg.pWakeupChannel));
    idl.pWakeupPattern(msg.pWakeupPattern);
    idl.pdMicrotick(to_idl(msg.pdMicrotick));
    idl.pSamplesPerMicrotick(msg.pSamplesPerMicrotick);
    
    return idl;
}

auto idl::from_idl(idl::NodeParameters&& idl) -> fr::NodeParameters
{
    fr::NodeParameters msg;
        
    msg.pAllowHaltDueToClock = idl.pAllowHaltDueToClock();
    msg.pAllowPassiveToActive = idl.pAllowPassiveToActive();
    msg.pChannels = from_idl(idl.pChannels());
    msg.pClusterDriftDamping = idl.pClusterDriftDamping();
    msg.pdAcceptedStartupRange = idl.pdAcceptedStartupRange();
    msg.pdListenTimeout = idl.pdListenTimeout();
    msg.pKeySlotId = idl.pKeySlotId();
    msg.pKeySlotOnlyEnabled = idl.pKeySlotOnlyEnabled();
    msg.pKeySlotUsedForStartup = idl.pKeySlotUsedForStartup();
    msg.pKeySlotUsedForSync = idl.pKeySlotUsedForSync();
    msg.pLatestTx = idl.pLatestTx();
    msg.pMacroInitialOffsetA = idl.pMacroInitialOffsetA();
    msg.pMacroInitialOffsetB = idl.pMacroInitialOffsetB();
    msg.pMicroInitialOffsetA = idl.pMicroInitialOffsetA();
    msg.pMicroInitialOffsetB = idl.pMicroInitialOffsetB();
    msg.pMicroPerCycle = idl.pMicroPerCycle();
    msg.pOffsetCorrectionOut = idl.pOffsetCorrectionOut();
    msg.pOffsetCorrectionStart = idl.pOffsetCorrectionStart();
    msg.pRateCorrectionOut = idl.pRateCorrectionOut();
    msg.pWakeupChannel = from_idl(idl.pWakeupChannel());
    msg.pWakeupPattern = idl.pWakeupPattern(); 
    msg.pdMicrotick = from_idl(idl.pdMicrotick());
    msg.pSamplesPerMicrotick = idl.pSamplesPerMicrotick();

    return msg;
}

auto to_idl(const TxBufferConfig& msg) -> idl::TxBufferConfig
{
    idl::TxBufferConfig idl;

    idl.channels(to_idl(msg.channels));
    idl.slotId(msg.slotId);
    idl.offset(msg.offset);
    idl.repetition(msg.repetition);
    idl.hasPayloadPreambleIndicator(msg.hasPayloadPreambleIndicator);
    idl.headerCrc(msg.headerCrc);
    idl.transmissionMode(to_idl(msg.transmissionMode));

    return idl;
}

auto idl::from_idl(idl::TxBufferConfig&& idl) -> fr::TxBufferConfig
{
    fr::TxBufferConfig msg;

    msg.channels = from_idl(idl.channels());
    msg.slotId = idl.slotId();
    msg.offset = idl.offset();
    msg.repetition = idl.repetition();
    msg.hasPayloadPreambleIndicator = idl.hasPayloadPreambleIndicator();
    msg.headerCrc = idl.headerCrc();
    msg.transmissionMode = from_idl(idl.transmissionMode());

    return msg;
}

auto to_idl(const ControllerConfig& msg) -> idl::ControllerConfig
{
    idl::ControllerConfig idl;

    std::vector<idl::TxBufferConfig> bufferConfigs(msg.bufferConfigs.size());
    std::transform(msg.bufferConfigs.begin(), msg.bufferConfigs.end(), bufferConfigs.begin(),
        [](auto&& cfg) { return to_idl(cfg); }
    );

    idl.clusterParams(to_idl(msg.clusterParams));
    idl.nodeParams(to_idl(msg.nodeParams));
    idl.bufferConfigs(std::move(bufferConfigs));

    return idl;
}

auto idl::from_idl(idl::ControllerConfig&& idl) -> fr::ControllerConfig
{
    fr::ControllerConfig msg;

    std::vector<fr::TxBufferConfig> bufferConfigs(idl.bufferConfigs().size());
    std::transform(idl.bufferConfigs().begin(), idl.bufferConfigs().end(), bufferConfigs.begin(),
        [](auto&& cfg) { return from_idl(std::move(cfg)); }
    );

    msg.clusterParams = from_idl(std::move(idl.clusterParams()));
    msg.nodeParams = from_idl(std::move(idl.nodeParams()));
    msg.bufferConfigs = std::move(bufferConfigs);

    return msg;
}

auto to_idl(const TxBufferUpdate& msg) -> idl::TxBufferUpdate
{
    idl::TxBufferUpdate idl;

    idl.txBufferIndex(msg.txBufferIndex);
    idl.payloadDataValid(msg.payloadDataValid);
    idl.payload(msg.payload);

    return idl;
}

auto to_idl(TxBufferUpdate&& msg) -> idl::TxBufferUpdate
{
    idl::TxBufferUpdate idl;

    idl.txBufferIndex(msg.txBufferIndex);
    idl.payloadDataValid(msg.payloadDataValid);
    idl.payload(std::move(msg.payload));

    return idl;
}

auto idl::from_idl(idl::TxBufferUpdate&& idl) -> fr::TxBufferUpdate
{
    fr::TxBufferUpdate msg;

    msg.txBufferIndex = idl.txBufferIndex();
    msg.payloadDataValid = idl.payloadDataValid();
    msg.payload = std::move(idl.payload());

    return msg;
}

auto to_idl(const ControllerStatus& msg) -> idl::ControllerStatus
{
    idl::ControllerStatus idl;

    idl.timeNs(msg.timestamp.count());
    idl.pocState(to_idl(msg.pocState));

    return idl;
}

auto idl::from_idl(idl::ControllerStatus&& idl) -> fr::ControllerStatus
{
    fr::ControllerStatus msg;

    msg.timestamp = std::chrono::nanoseconds{idl.timeNs()};
    msg.pocState = from_idl(idl.pocState());

    return msg;
}


} // namespace fr
} // namespace sim
} // namespace ib
