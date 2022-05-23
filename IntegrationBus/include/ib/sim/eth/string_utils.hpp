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

std::ostream& operator<<(std::ostream& out, const EthernetFrame& frame)
{
    if (frame.raw.size() == 0)
    {
        return out
            << "EthernetFrame{size=0}";
    }
    else
    {
        out << "EthernetFrame{size=" << frame.raw.size();
        if (frame.raw.size() >= 2 * sizeof(EthernetMac))
        {
            EthernetMac destinationMac;
            EthernetMac sourceMac;
            std::copy(
                frame.raw.begin(),
                frame.raw.begin() + sizeof(EthernetMac), destinationMac.begin());
            std::copy(
                frame.raw.begin() + sizeof(EthernetMac),
                frame.raw.begin() + 2 * sizeof(EthernetMac), sourceMac.begin());

            out << ", src = " << util::AsHexString(sourceMac).WithSeparator(":")
                << ", dst=" << util::AsHexString(destinationMac).WithSeparator(":");
        }
        return out
            << ", data=[" << util::AsHexString(frame.raw).WithSeparator(" ").WithMaxLength(8)
            << "]}";
    }
}

std::ostream& operator<<(std::ostream& out, const EthernetFrameEvent& msg)
{
    auto timestamp = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(msg.timestamp);
    return out
        << "EthernetFrameEvent{txId=" << msg.transmitId
        << ", " << msg.frame
        << " @" << timestamp.count() << "ms"
        << "}";
}

std::ostream& operator<<(std::ostream& out, const EthernetFrameTransmitEvent& msg)
{
    auto timestamp = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(msg.timestamp);
    out
        << "EthernetFrameTransmitEvent{txId=" << msg.transmitId
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
        << "EthernetStatus{"
        << "state=" << msg.state
        << "bitrate=" << msg.bitrate
        << " @" << timestamp.count() << "ms"
        << "}";
}

std::ostream& operator<<(std::ostream& out, const EthernetSetMode& msg)
{
    return out
        << "EthernetSetMode{" << msg.mode << "}";
}

} // namespace eth
} // namespace sim
} // namespace ib
