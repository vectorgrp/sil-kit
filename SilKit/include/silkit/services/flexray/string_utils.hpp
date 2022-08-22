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

#pragma once

#include <ostream>
#include <sstream>

#include "silkit/participant/exception.hpp"
#include "silkit/util/PrintableHexString.hpp"

#include "FlexrayDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace Flexray {

inline std::string to_string(FlexrayChannel channel);
inline std::string to_string(FlexrayClockPeriod period);
inline std::string to_string(FlexrayTransmissionMode mode);
inline std::string to_string(FlexraySymbolPattern pattern);
inline std::string to_string(FlexrayPocState state);
inline std::string to_string(const FlexrayHeader& header);
inline std::string to_string(const FlexraySymbolEvent& symbol);
inline std::string to_string(const FlexraySymbolTransmitEvent& symbol);
inline std::string to_string(const FlexrayCycleStartEvent& cycleStart);
inline std::string to_string(const FlexrayFrameEvent& msg);
inline std::string to_string(const FlexrayFrameTransmitEvent& msg);
inline std::string to_string(const FlexrayControllerConfig& msg);
inline std::string to_string(const FlexrayTxBufferConfig& msg);
inline std::string to_string(const FlexrayTxBufferUpdate& msg);
inline std::string to_string(const FlexrayPocStatusEvent& msg);
inline std::string to_string(FlexraySlotModeType msg);
inline std::string to_string(FlexrayErrorModeType msg);
inline std::string to_string(FlexrayStartupStateType msg);
inline std::string to_string(FlexrayWakeupStatusType msg);

inline std::ostream& operator<<(std::ostream& out, FlexrayChannel channel);
inline std::ostream& operator<<(std::ostream& out, FlexrayClockPeriod period);
inline std::ostream& operator<<(std::ostream& out, FlexrayTransmissionMode mode);
inline std::ostream& operator<<(std::ostream& out, FlexraySymbolPattern pattern);
inline std::ostream& operator<<(std::ostream& out, FlexrayPocState state);

inline std::ostream& operator<<(std::ostream& out, const FlexrayHeader& header);
inline std::ostream& operator<<(std::ostream& out, const FlexraySymbolEvent& symbol);
inline std::ostream& operator<<(std::ostream& out, const FlexraySymbolTransmitEvent& symbol);
inline std::ostream& operator<<(std::ostream& out, const FlexrayCycleStartEvent& cycleStart);
inline std::ostream& operator<<(std::ostream& out, const FlexrayFrameEvent& msg);
inline std::ostream& operator<<(std::ostream& out, const FlexrayFrameTransmitEvent& msg);
inline std::ostream& operator<<(std::ostream& out, const FlexrayControllerConfig& msg);
inline std::ostream& operator<<(std::ostream& out, const FlexrayTxBufferConfig& msg);
inline std::ostream& operator<<(std::ostream& out, const FlexrayTxBufferUpdate& msg);
inline std::ostream& operator<<(std::ostream& out, const FlexrayPocStatusEvent& msg);
inline std::ostream& operator<<(std::ostream& out, FlexraySlotModeType msg);
inline std::ostream& operator<<(std::ostream& out, FlexrayErrorModeType msg);
inline std::ostream& operator<<(std::ostream& out, FlexrayStartupStateType msg);
inline std::ostream& operator<<(std::ostream& out, FlexrayWakeupStatusType msg);
    

// ================================================================================
//  Inline Implementations
// ================================================================================
std::string to_string(FlexrayChannel channel)
{
    std::stringstream out;
    out << channel;
    return out.str();
}

std::string to_string(FlexrayClockPeriod period)
{
    switch (period)
    {
    case FlexrayClockPeriod::T12_5NS:
        return "12.5ns";
    case FlexrayClockPeriod::T25NS:
        return "25ns";
    case FlexrayClockPeriod::T50NS:
        return "50ns";
    };
    throw SilKit::TypeConversionError{};
}

std::string to_string(FlexrayTransmissionMode mode)
{
    switch (mode)
    {
    case FlexrayTransmissionMode::SingleShot:
        return "SingleShot";
    case FlexrayTransmissionMode::Continuous:
        return "Continuous";
    };
    throw SilKit::TypeConversionError{};
}

std::string to_string(FlexraySymbolPattern pattern)
{
    std::stringstream out;
    out << pattern;
    return out.str();
}

std::string to_string(FlexrayPocState state)
{
    switch (state)
    {
    case FlexrayPocState::DefaultConfig:
        return "DefaultConfig";
    case FlexrayPocState::Config:
        return "Config";
    case FlexrayPocState::Ready:
        return "Ready";
    case FlexrayPocState::Startup:
        return "Startup";
    case FlexrayPocState::Wakeup:
        return "Wakeup";
    case FlexrayPocState::NormalActive:
        return "NormalActive";
    case FlexrayPocState::NormalPassive:
        return "NormalPassive";
    case FlexrayPocState::Halt:
        return "Halt";
    }
    throw SilKit::TypeConversionError{};
}

std::string to_string(const FlexrayHeader& header)
{
    std::stringstream out;
    out << header;
    return out.str();
}

std::string to_string(const FlexraySymbolEvent& symbol)
{
    std::stringstream out;
    out << symbol;
    return out.str();
}

std::string to_string(const FlexraySymbolTransmitEvent& symbol)
{
    std::stringstream out;
    out << symbol;
    return out.str();
}

std::string to_string(const FlexrayCycleStartEvent& cycleStart)
{
    std::stringstream out;
    out << cycleStart;
    return out.str();
}

std::string to_string(const FlexrayFrameEvent& msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}

std::string to_string(const FlexrayFrameTransmitEvent& msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}

std::string to_string(const FlexrayControllerConfig& msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}

std::string to_string(const FlexrayTxBufferConfig& msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}

std::string to_string(const FlexrayTxBufferUpdate& msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}

std::string to_string(const FlexrayPocStatusEvent& msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}

std::string to_string(FlexraySlotModeType msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}

std::string to_string(FlexrayErrorModeType msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}

std::string to_string(FlexrayStartupStateType msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}

std::string to_string(FlexrayWakeupStatusType msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}

std::ostream& operator<<(std::ostream& out, FlexrayChannel channel)
{
    switch (channel)
    {
    case FlexrayChannel::A:
        return out << "A";
    case FlexrayChannel::B:
        return out << "B";
    case FlexrayChannel::AB:
        return out << "AB";
    case FlexrayChannel::None:
        return out << "None";
    default:
        return out << "FlexrayChannel=" << static_cast<uint32_t>(channel);
    }
}
std::ostream& operator<<(std::ostream& out, FlexrayClockPeriod period)
{
    return out << to_string(period);
}
std::ostream& operator<<(std::ostream& out, FlexrayTransmissionMode mode)
{
    return out << to_string(mode);
}
std::ostream& operator<<(std::ostream& out, FlexraySymbolPattern pattern)
{
    switch (pattern)
    {
    case FlexraySymbolPattern::CasMts:
        return out << "CasMts";
    case FlexraySymbolPattern::Wus:
        return out << "Wus";
    case FlexraySymbolPattern::Wudop:
        return out << "Wudop";
    default:
        return out << "FlexraySymbolPattern=" << static_cast<uint32_t>(pattern);
    }
}
std::ostream& operator<<(std::ostream& out, FlexrayPocState state)
{
    return out << to_string(state);
}
std::ostream& operator<<(std::ostream& out, const FlexrayHeader& header)
{
    using FlagMask = FlexrayHeader::FlagMask;
    using Flag = FlexrayHeader::Flag;
    return out
        << "Flexray::FlexrayHeader{f=["
        << ((header.flags & static_cast<FlagMask>(Flag::SuFIndicator)) ? "U" : "-")
        << ((header.flags & static_cast<FlagMask>(Flag::SyFIndicator)) ? "Y" : "-")
        << ((header.flags & static_cast<FlagMask>(Flag::NFIndicator)) ? "-" : "N")
        << ((header.flags & static_cast<FlagMask>(Flag::PPIndicator)) ? "P" : "-")
        << "],s=" << header.frameId
        << ",l=" << (uint32_t)header.payloadLength
        << ",crc=" << std::hex << header.headerCrc << std::dec
        << ",c=" << (uint32_t)header.cycleCount
        << "}";
}
std::ostream& operator<<(std::ostream& out, const FlexraySymbolEvent& symbol)
{
    auto timestamp = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(symbol.timestamp);
    return out
        << "Flexray::FlexraySymbolEvent{pattern=" << symbol.pattern
        << ", channel=" << symbol.channel
        << " @ " << timestamp.count() << "ms}";
}

std::ostream& operator<<(std::ostream& out, const FlexraySymbolTransmitEvent& symbol)
{
    auto timestamp = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(symbol.timestamp);
    return out
        << "Flexray::FlexraySymbolTransmitEvent{pattern=" << symbol.pattern
        << ", channel=" << symbol.channel
        << " @ " << timestamp.count() << "ms}";
}

std::ostream& operator<<(std::ostream& out, const FlexrayCycleStartEvent& cycleStart)
{
    auto timestamp = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(cycleStart.timestamp);
    return out
        << "Flexray::FlexrayCycleStartEvent{t=" << timestamp.count()
        << "ms, cycleCounter=" << static_cast<uint32_t>(cycleStart.cycleCounter)
        << "}";
}

std::ostream& operator<<(std::ostream& out, const FlexrayFrameEvent& msg)
{
    auto timestamp = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(msg.timestamp);
    out << "Flexray::FlexrayFrameEvent{"
        << "ch=" << msg.channel
        << ", " << msg.frame.header
        << " @" << timestamp.count() << "ms";
    if (msg.frame.header.flags & static_cast<FlexrayHeader::FlagMask>(FlexrayHeader::Flag::NFIndicator))
    {
        // if payload is valid, provide it as hex dump
        out << ", payload="
            << Util::AsHexString(msg.frame.payload).WithSeparator(" ").WithMaxLength(msg.frame.header.payloadLength);
    }
    return out << "}";
}

std::ostream& operator<<(std::ostream& out, const FlexrayFrameTransmitEvent& msg)
{
    auto timestamp = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(msg.timestamp);
    return out << "Flexray::FlexrayFrameTransmitEvent{"
        << msg.frame.header
        << ", ch=" << msg.channel
        << ", txBuffer=" << msg.txBufferIndex
        << " @" << timestamp.count() << "ms}";
}

std::ostream& operator<<(std::ostream& out, const FlexrayControllerConfig& /*msg*/)
{
    return out << "Flexray::FlexrayControllerConfig{...}";
}

std::ostream& operator<<(std::ostream& out, const FlexrayTxBufferConfig& msg)
{
    return out << "Flexray::FlexrayTxBufferConfig{"
        << "ch=" << msg.channels
        << ", slot=" << msg.slotId
        << (msg.hasPayloadPreambleIndicator ? ", PP" : "")
        << ", crc=" << msg.headerCrc
        << ", off=" << msg.offset
        << ", rep=" << msg.repetition
        << ", txMode=" << msg.transmissionMode
        << "}";
}

std::ostream& operator<<(std::ostream& out, const FlexrayTxBufferUpdate& msg)
{
    out << "Flexray::FlexrayTxBufferUpdate{"
        << "idx=" << msg.txBufferIndex
        << ", payloadValid=";

    if (msg.payloadDataValid)
    {
        out << "t, payload=["
            << Util::AsHexString(msg.payload).WithSeparator(" ").WithMaxLength(8)
            << "], payloadSize=" << msg.payload.size();
    }
    else
    {
        out << "f";
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, const FlexrayPocStatusEvent& msg)
{
    auto timestamp = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(msg.timestamp);
    return out << "Flexray::POCStatus{"
        << "State=" << msg.state
        << ", Freeze=" << msg.freeze
        << " @" << timestamp.count() << "ms}";
}

std::ostream& operator<<(std::ostream& out, FlexraySlotModeType msg)
{
    switch(msg)
    {
    case FlexraySlotModeType::KeySlot:
        out << "KeySlot"; break;

    case FlexraySlotModeType::AllPending:
        out << "AllPending"; break;

    case FlexraySlotModeType::All:
        out << "All"; break;

    default:
        throw SilKit::TypeConversionError{};
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, FlexrayErrorModeType msg)
{
    switch (msg)
    {
    case FlexrayErrorModeType::Active:
        out << "Active"; break;

    case FlexrayErrorModeType::Passive:
        out << "Passive"; break;

    case FlexrayErrorModeType::CommHalt:
        out << "CommHalt"; break;


    default:
        throw SilKit::TypeConversionError{};
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, FlexrayStartupStateType msg)
{
    switch (msg)
    {
    case FlexrayStartupStateType::Undefined:
        out << "Undefined"; break;

    case FlexrayStartupStateType::ColdStartListen:
        out << "ColdStartListen"; break;

    case FlexrayStartupStateType::IntegrationColdstartCheck:
        out << "IntegrationColdstartCheck"; break;

    case FlexrayStartupStateType::ColdStartJoin:
        out << "ColdStartJoin"; break;

    case FlexrayStartupStateType::ColdStartCollisionResolution:
        out << "ColdStartCollisionResolution"; break;

    case FlexrayStartupStateType::ColdStartConsistencyCheck:
        out << "ColdStartConsistencyCheck"; break;

    case FlexrayStartupStateType::IntegrationListen:
        out << "IntegrationListen"; break;

    case FlexrayStartupStateType::InitializeSchedule:
        out << "InitializeSchedule"; break;

    case FlexrayStartupStateType::IntegrationConsistencyCheck:
        out << "IntegrationConsistencyCheck"; break;

    case FlexrayStartupStateType::ColdStartGap:
        out << "ColdStartGap"; break;

    case FlexrayStartupStateType::ExternalStartup:
        out << "ExternalStartup"; break;

    default:
        throw SilKit::TypeConversionError{};
    }

    return out;
}

std::ostream& operator<<(std::ostream& out, FlexrayWakeupStatusType msg)
{
    switch (msg)
    {
    case FlexrayWakeupStatusType::Undefined:
        out << "Undefined"; break;

    case FlexrayWakeupStatusType::ReceivedHeader:
        out << "ReceivedHeader"; break;

    case FlexrayWakeupStatusType::ReceivedWup:
        out << "ReceivedWup"; break;

    case FlexrayWakeupStatusType::CollisionHeader:
        out << "CollisionHeader"; break;

    case FlexrayWakeupStatusType::CollisionWup:
        out << "CollisionWup"; break;

    case FlexrayWakeupStatusType::CollisionUnknown:
        out << "CollisionUnknown"; break;

    case FlexrayWakeupStatusType::Transmitted:
        out << "Transmitted"; break;
    default:
        throw SilKit::TypeConversionError{};
    }

    return out;
}




} // namespace Flexray
} // namespace Services
} // namespace SilKit
