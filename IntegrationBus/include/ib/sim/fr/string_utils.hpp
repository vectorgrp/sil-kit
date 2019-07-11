// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <ostream>
#include <sstream>

#include "ib/exception.hpp"

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
    

// ================================================================================
//  Inline Implementations
// ================================================================================
std::string to_string(Channel channel)
{
    switch (channel)
    {
    case  Channel::None:
        return "None";
    case  Channel::A:
        return "A";
    case  Channel::B:
        return "B";
    case  Channel::AB:
        return "AB";
    };
    throw ib::type_conversion_error{};
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
    switch (pattern)
    {
    case SymbolPattern::CasMts:
        return "CasMts";
    case SymbolPattern::Wus:
        return "Wus";
    case SymbolPattern::Wudop:
        return "Wudop";
    };
    throw ib::type_conversion_error{};
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

    
std::ostream& operator<<(std::ostream& out, Channel channel)
{
    return out << to_string(channel);
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
    return out << to_string(pattern);
}
std::ostream& operator<<(std::ostream& out, PocState state)
{
    return out << to_string(state);
}
std::ostream& operator<<(std::ostream& out, const Header& header)
{
    return out
        << "Header{f=["
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
    auto seconds = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1>>>(symbol.timestamp);
    return out
        << "FrSymbol{t=" << seconds.count()
        << "s, channel=" << symbol.channel
        << ", pattern=" << symbol.pattern
        << "}";
}

std::ostream& operator<<(std::ostream& out, const fr::FrSymbolAck& symbol)
{
    auto seconds = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1>>>(symbol.timestamp);
    return out
        << "FrSymbolAck{t=" << seconds.count()
        << "s, channel=" << symbol.channel
        << ", pattern=" << symbol.pattern
        << "}";
}

std::ostream& operator<<(std::ostream& out, const fr::CycleStart& cycleStart)
{
    auto seconds = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1>>>(cycleStart.timestamp);
    return out
        << "CycleStart{t=" << seconds.count()
        << "s, cycleCounter=" << static_cast<uint32_t>(cycleStart.cycleCounter)
        << "}";
};




} // namespace fr
} // namespace sim
} // namespace ib
