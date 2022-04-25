// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <ostream>
#include <sstream>

#include "ib/exception.hpp"
#include "ib/util/PrintableHexString.hpp"

#include "EthernetDatatypes.hpp"

namespace ib {
namespace sim {
namespace eth {

inline std::string to_string(EthernetTransmitStatus value);
inline std::string to_string(EthernetState value);
inline std::string to_string(EthernetMode value);

inline std::string to_string(const EthernetFrame& msg);
inline std::string to_string(const EthernetFrameEvent& msg);
inline std::string to_string(const EthernetFrameTransmitEvent& msg);
inline std::string to_string(const EthernetStatus& msg);
inline std::string to_string(const EthernetSetMode& msg);


inline std::ostream& operator<<(std::ostream& out, EthernetTransmitStatus value);
inline std::ostream& operator<<(std::ostream& out, EthernetState value);
inline std::ostream& operator<<(std::ostream& out, EthernetMode value);

inline std::ostream& operator<<(std::ostream& out, const EthernetFrame& msg);
inline std::ostream& operator<<(std::ostream& out, const EthernetFrameEvent& msg);
inline std::ostream& operator<<(std::ostream& out, const EthernetFrameTransmitEvent& msg);
inline std::ostream& operator<<(std::ostream& out, const EthernetStatus& msg);
inline std::ostream& operator<<(std::ostream& out, const EthernetSetMode& msg);

// ================================================================================
//  Inline Implementations
// ================================================================================
std::string to_string(EthernetTransmitStatus value)
{
    switch (value)
    {
    case  EthernetTransmitStatus::Transmitted:
        return "Transmitted";
    case  EthernetTransmitStatus::ControllerInactive:
        return "ControllerInactive";
    case  EthernetTransmitStatus::LinkDown:
        return "LinkDown";
    case  EthernetTransmitStatus::Dropped:
        return "Dropped";
    case  EthernetTransmitStatus::DuplicatedTransmitId:
        return "DuplicatedTransmitId";
    case  EthernetTransmitStatus::InvalidFrameFormat:
        return "InvalidFrameFormat";
    };
    throw ib::TypeConversionError{};
}

std::string to_string(EthernetState value)
{
    switch (value)
    {
    case EthernetState::Inactive:
        return "Inactive";
    case EthernetState::LinkDown:
        return "LinkDown";
    case EthernetState::LinkUp:
        return "LinkUp";
    };
    throw ib::TypeConversionError{};
}

std::string to_string(EthernetMode value)
{
    switch (value)
    {
    case EthernetMode::Inactive:
        return "Inactive";
    case EthernetMode::Active:
        return "Active";
    };
    throw ib::TypeConversionError{};
}

std::string to_string(const EthernetFrame& msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}


std::string to_string(const EthernetFrameEvent& msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}

std::string to_string(const EthernetFrameTransmitEvent& msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}

std::string to_string(const EthernetStatus& msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}

std::string to_string(const EthernetSetMode& msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}


    
std::ostream& operator<<(std::ostream& out, EthernetTransmitStatus value)
{
    return out << to_string(value);
}
std::ostream& operator<<(std::ostream& out, EthernetState value)
{
    return out << to_string(value);
}
std::ostream& operator<<(std::ostream& out, EthernetMode value)
{
    return out << to_string(value);
}

std::ostream& operator<<(std::ostream& out, const EthernetFrame& msg)
{
    if (msg.GetFrameSize() == 0)
    {
        return out
            << "EthernetFrame{size=0}";
    }
    else
    {
        return out
            << "EthernetFrame{src=" << util::AsHexString(msg.GetSourceMac()).WithSeparator(":")
            << ", dst=" << util::AsHexString(msg.GetDestinationMac()).WithSeparator(":")
            << ", size=" << msg.GetFrameSize()
            << ", payload=[" << util::AsHexString(msg.GetPayload()).WithSeparator(" ").WithMaxLength(8)
            << "]}";
    }
}

std::ostream& operator<<(std::ostream& out, const EthernetFrameEvent& msg)
{
    auto timestamp = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(msg.timestamp);
    return out
        << "eth::EthernetFrameEvent{txId=" << msg.transmitId
        << ", " << msg.ethFrame
        << " @" << timestamp.count() << "ms"
        << "}";
}

std::ostream& operator<<(std::ostream& out, const EthernetFrameTransmitEvent& msg)
{
    auto timestamp = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(msg.timestamp);
    out
        << "eth::EthernetFrameTransmitEvent{txId=" << msg.transmitId
        << ", src=" << util::AsHexString(msg.sourceMac).WithSeparator(":")
        << ", status=" << msg.status
        << " @" << timestamp.count() << "ms"
        << "}";

    return out;
}

std::ostream& operator<<(std::ostream& out, const EthernetStatus& msg)
{
    auto timestamp = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(msg.timestamp);
    return out
        << "eth::EthernetStatus{"
        << "state=" << msg.state
        << "bitrate=" << msg.bitrate
        << " @" << timestamp.count() << "ms"
        << "}";
}

std::ostream& operator<<(std::ostream& out, const EthernetSetMode& msg)
{
    return out
        << "eth::EthernetSetMode{" << msg.mode << "}";
}


} // namespace eth
} // namespace sim
} // namespace ib
