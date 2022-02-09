// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <ostream>
#include <sstream>

#include "ib/exception.hpp"
#include "ib/util/PrintableHexString.hpp"

#include "FrDatatypes.hpp"

namespace ib {
namespace sim {
namespace fr {

inline std::string to_string(Channel channel);
inline std::string to_string(ClockPeriod period);
inline std::string to_string(TransmissionMode mode);
inline std::string to_string(ChiCommand command);
inline std::string to_string(SymbolPattern pattern);
inline std::string to_string(PocState state);
inline std::string to_string(const Header& header);
inline std::string to_string(const FrSymbol& symbol);
inline std::string to_string(const FrSymbolAck& symbol);
inline std::string to_string(const CycleStart& cycleStart);
inline std::string to_string(const FrMessage& msg);
inline std::string to_string(const FrMessageAck& msg);
inline std::string to_string(const HostCommand& msg);
inline std::string to_string(const ControllerConfig& msg);
inline std::string to_string(const TxBufferConfig& msg);
inline std::string to_string(const TxBufferConfigUpdate& msg);
inline std::string to_string(const TxBufferUpdate& msg);
inline std::string to_string(const PocStatus& msg);
inline std::string to_string(SlotModeType msg);
inline std::string to_string(ErrorModeType msg);
inline std::string to_string(StartupStateType msg);
inline std::string to_string(WakeupStatusType msg);


inline std::ostream& operator<<(std::ostream& out, Channel channel);
inline std::ostream& operator<<(std::ostream& out, ClockPeriod period);
inline std::ostream& operator<<(std::ostream& out, TransmissionMode mode);
inline std::ostream& operator<<(std::ostream& out, ChiCommand command);
inline std::ostream& operator<<(std::ostream& out, SymbolPattern pattern);
inline std::ostream& operator<<(std::ostream& out, PocState state);

inline std::ostream& operator<<(std::ostream& out, const Header& header);
inline std::ostream& operator<<(std::ostream& out, const FrSymbol& symbol);
inline std::ostream& operator<<(std::ostream& out, const FrSymbolAck& symbol);
inline std::ostream& operator<<(std::ostream& out, const CycleStart& cycleStart);
inline std::ostream& operator<<(std::ostream& out, const FrMessage& msg);
inline std::ostream& operator<<(std::ostream& out, const FrMessageAck& msg);
inline std::ostream& operator<<(std::ostream& out, const HostCommand& msg);
inline std::ostream& operator<<(std::ostream& out, const ControllerConfig& msg);
inline std::ostream& operator<<(std::ostream& out, const TxBufferConfig& msg);
inline std::ostream& operator<<(std::ostream& out, const TxBufferConfigUpdate& msg);
inline std::ostream& operator<<(std::ostream& out, const TxBufferUpdate& msg);
inline std::ostream& operator<<(std::ostream& out, const PocStatus& msg);
inline std::ostream& operator<<(std::ostream& out, SlotModeType msg);
inline std::ostream& operator<<(std::ostream& out, ErrorModeType msg);
inline std::ostream& operator<<(std::ostream& out, StartupStateType msg);
inline std::ostream& operator<<(std::ostream& out, WakeupStatusType msg);
    

// ================================================================================
//  Inline Implementations
// ================================================================================
std::string to_string(Channel channel)
{
    std::stringstream out;
    out << channel;
    return out.str();
}

std::string to_string(ClockPeriod period)
{
    switch (period)
    {
    case ClockPeriod::T12_5NS:
        return "12.5ns";
    case ClockPeriod::T25NS:
        return "25ns";
    case ClockPeriod::T50NS:
        return "50ns";
    };
    throw ib::type_conversion_error{};
}

std::string to_string(TransmissionMode mode)
{
    switch (mode)
    {
    case TransmissionMode::SingleShot:
        return "SingleShot";
    case TransmissionMode::Continuous:
        return "Continuous";
    };
    throw ib::type_conversion_error{};
}
    
std::string to_string(ChiCommand command)
{
    switch (command)
    {
    case ChiCommand::RUN:
        return "RUN";
    case ChiCommand::DEFERRED_HALT:
        return "DEFERRED_HALT";
    case ChiCommand::FREEZE:
        return "FREEZE";
    case ChiCommand::ALLOW_COLDSTART:
        return "ALLOW_COLDSTART";
    case ChiCommand::ALL_SLOTS:
        return "ALL_SLOTS";
    case ChiCommand::WAKEUP:
        return "WAKEUP";
    };
    throw ib::type_conversion_error{};
}
    
std::string to_string(SymbolPattern pattern)
{
    std::stringstream out;
    out << pattern;
    return out.str();
}
    
std::string to_string(PocState state)
{
    switch (state)
    {
    case PocState::DefaultConfig:
        return "DefaultConfig";
    case PocState::Config:
        return "Config";
    case PocState::Ready:
        return "Ready";
    case PocState::Startup:
        return "Startup";
    case PocState::Wakeup:
        return "Wakeup";
    case PocState::NormalActive:
        return "NormalActive";
    case PocState::NormalPassive:
        return "NormalPassive";
    case PocState::Halt:
        return "Halt";
    }
    throw ib::type_conversion_error{};
}

std::string to_string(const Header& header)
{
    std::stringstream out;
    out << header;
    return out.str();
}
    
std::string to_string(const FrSymbol& symbol)
{
    std::stringstream out;
    out << symbol;
    return out.str();
}
    
std::string to_string(const FrSymbolAck& symbol)
{
    std::stringstream out;
    out << symbol;
    return out.str();
}

std::string to_string(const CycleStart& cycleStart)
{
    std::stringstream out;
    out << cycleStart;
    return out.str();
}

std::string to_string(const FrMessage& msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}

std::string to_string(const FrMessageAck& msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}

std::string to_string(const HostCommand& msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}

std::string to_string(const ControllerConfig& msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}

std::string to_string(const TxBufferConfig& msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}

std::string to_string(const TxBufferConfigUpdate& msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}

std::string to_string(const TxBufferUpdate& msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}

std::string to_string(const PocStatus& msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}

std::string to_string(SlotModeType msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}

std::string to_string(ErrorModeType msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}

std::string to_string(StartupStateType msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}

std::string to_string(WakeupStatusType msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}


    
std::ostream& operator<<(std::ostream& out, Channel channel)
{
    switch (channel)
    {
    case Channel::A:
        return out << "A";
    case Channel::B:
        return out << "B";
    case Channel::AB:
        return out << "AB";
    case Channel::None:
        return out << "None";
    default:
        return out << "Channel=" << static_cast<uint32_t>(channel);
    }
}
std::ostream& operator<<(std::ostream& out, ClockPeriod period)
{
    return out << to_string(period);
}
std::ostream& operator<<(std::ostream& out, TransmissionMode mode)
{
    return out << to_string(mode);
}
std::ostream& operator<<(std::ostream& out, ChiCommand command)
{
    return out << to_string(command);
}
std::ostream& operator<<(std::ostream& out, SymbolPattern pattern)
{
    switch (pattern)
    {
    case SymbolPattern::CasMts:
        return out << "CasMts";
    case SymbolPattern::Wus:
        return out << "Wus";
    case SymbolPattern::Wudop:
        return out << "Wudop";
    default:
        return out << "SymbolPattern=" << static_cast<uint32_t>(pattern);
    }
}
std::ostream& operator<<(std::ostream& out, PocState state)
{
    return out << to_string(state);
}
std::ostream& operator<<(std::ostream& out, const Header& header)
{
    return out
        << "fr::Header{f=["
        << (header.IsSet(Header::Flag::SuFIndicator) ? "U" : "-")
        << (header.IsSet(Header::Flag::SyFIndicator) ? "Y" : "-")
        << (header.IsSet(Header::Flag::NFIndicator) ? "-" : "N")
        << (header.IsSet(Header::Flag::PPIndicator) ? "P" : "-")
        << "],s=" << header.frameId
        << ",l=" << (uint32_t)header.payloadLength
        << ",crc=" << std::hex << header.headerCrc << std::dec
        << ",c=" << (uint32_t)header.cycleCount
        << "}";
}
std::ostream& operator<<(std::ostream& out, const FrSymbol& symbol)
{
    auto timestamp = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(symbol.timestamp);
    return out
        << "fr::FrSymbol{pattern=" << symbol.pattern
        << ", channel=" << symbol.channel
        << " @ " << timestamp.count() << "ms}";
}

std::ostream& operator<<(std::ostream& out, const FrSymbolAck& symbol)
{
    auto timestamp = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(symbol.timestamp);
    return out
        << "fr::FrSymbolAck{pattern=" << symbol.pattern
        << ", channel=" << symbol.channel
        << " @ " << timestamp.count() << "ms}";
}

std::ostream& operator<<(std::ostream& out, const CycleStart& cycleStart)
{
    auto timestamp = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(cycleStart.timestamp);
    return out
        << "fr::CycleStart{t=" << timestamp.count()
        << "ms, cycleCounter=" << static_cast<uint32_t>(cycleStart.cycleCounter)
        << "}";
};

std::ostream& operator<<(std::ostream& out, const FrMessage& msg)
{
    auto timestamp = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(msg.timestamp);
    out << "fr::FrMessage{"
        << "ch=" << msg.channel
        << ", " << msg.frame.header
        << " @" << timestamp.count() << "ms";
    if (msg.frame.header.IsSet(Header::Flag::NFIndicator))
    {
        // if payload is valid, provide it as hex dump
        out << ", payload="
            << util::AsHexString(msg.frame.payload).WithSeparator(" ").WithMaxLength(msg.frame.header.payloadLength);
    }
    return out << "}";
}

std::ostream& operator<<(std::ostream& out, const FrMessageAck& msg)
{
    auto timestamp = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(msg.timestamp);
    return out << "fr::FrMessageAck{"
        << msg.frame.header
        << ", ch=" << msg.channel
        << ", txBuffer=" << msg.txBufferIndex
        << " @" << timestamp.count() << "ms}";
}

std::ostream& operator<<(std::ostream& out, const HostCommand& msg)
{
    return out << "fr::HostCommand{"
        << msg.command
        << "}";
}

std::ostream& operator<<(std::ostream& out, const ControllerConfig& /*msg*/)
{
    return out << "fr::ControllerConfig{...}";
}

std::ostream& operator<<(std::ostream& out, const TxBufferConfig& msg)
{
    return out << "fr::TxBufferConfig{"
        << "ch=" << msg.channels
        << ", slot=" << msg.slotId
        << (msg.hasPayloadPreambleIndicator ? ", PP" : "")
        << ", crc=" << msg.headerCrc
        << ", off=" << msg.offset
        << ", rep=" << msg.repetition
        << ", txMode=" << msg.transmissionMode
        << "}";
}

std::ostream& operator<<(std::ostream& out, const TxBufferConfigUpdate& msg)
{
    return out << "fr::TxBufferConfigUpdate{"
        << "idx=" << msg.txBufferIndex
        << " " << msg.txBufferConfig
        << "}";
}

std::ostream& operator<<(std::ostream& out, const TxBufferUpdate& msg)
{
    out << "fr::TxBufferUpdate{"
        << "idx=" << msg.txBufferIndex
        << ", payloadValid=";

    if (msg.payloadDataValid)
    {
        out << "t, payload=["
            << util::AsHexString(msg.payload).WithSeparator(" ").WithMaxLength(8)
            << "], payloadSize=" << msg.payload.size();
    }
    else
    {
        out << "f";
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, const PocStatus& msg)
{
    auto timestamp = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(msg.timestamp);
    return out << "fr::POCStatus{"
        << "State=" << msg.state
        << ", Freeze=" << msg.freeze
        << " @" << timestamp.count() << "ms}";
}

std::ostream& operator<<(std::ostream& out, SlotModeType msg)
{
    switch(msg)
    {
    case SlotModeType::KeySlot:
        out << "KeySlot"; break;

    case SlotModeType::AllPending:
        out << "AllPending"; break;

    case SlotModeType::All:
        out << "All"; break;

    default:
        throw ib::type_conversion_error{};
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, ErrorModeType msg)
{
    switch (msg)
    {
    case ErrorModeType::Active:
        out << "Active"; break;

    case ErrorModeType::Passive:
        out << "Passive"; break;

    case ErrorModeType::CommHalt:
        out << "CommHalt"; break;


    default:
        throw ib::type_conversion_error{};
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, StartupStateType msg)
{
    switch (msg)
    {
    case StartupStateType::Undefined:
        out << "Undefined"; break;

    case StartupStateType::ColdStartListen:
        out << "ColdStartListen"; break;

    case StartupStateType::IntegrationColdstartCheck:
        out << "IntegrationColdstartCheck"; break;

    case StartupStateType::ColdStartJoin:
        out << "ColdStartJoin"; break;

    case StartupStateType::ColdStartCollisionResolution:
        out << "ColdStartCollisionResolution"; break;

    case StartupStateType::ColdStartConsistencyCheck:
        out << "ColdStartConsistencyCheck"; break;

    case StartupStateType::IntegrationListen:
        out << "IntegrationListen"; break;

    case StartupStateType::InitializeSchedule:
        out << "InitializeSchedule"; break;

    case StartupStateType::IntegrationConsistencyCheck:
        out << "IntegrationConsistencyCheck"; break;

    case StartupStateType::ColdStartGap:
        out << "ColdStartGap"; break;

    case StartupStateType::ExternalStartup:
        out << "ExternalStartup"; break;

    default:
        throw ib::type_conversion_error{};
    }

    return out;
}

std::ostream& operator<<(std::ostream& out, WakeupStatusType msg)
{
    switch (msg)
    {
    case WakeupStatusType::Undefined:
        out << "Undefined"; break;

    case WakeupStatusType::ReceivedHeader:
        out << "ReceivedHeader"; break;

    case WakeupStatusType::ReceivedWup:
        out << "ReceivedWup"; break;

    case WakeupStatusType::CollisionHeader:
        out << "CollisionHeader"; break;

    case WakeupStatusType::CollisionWup:
        out << "CollisionWup"; break;

    case WakeupStatusType::CollisionUnknown:
        out << "CollisionUnknown"; break;

    case WakeupStatusType::Transmitted:
        out << "Transmitted"; break;
    default:
        throw ib::type_conversion_error{};
    }

    return out;
}




} // namespace fr
} // namespace sim
} // namespace ib
