// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <ostream>
#include <sstream>

#include "ib/exception.hpp"

#include "EthDatatypes.hpp"

namespace ib {
namespace sim {
namespace eth {

inline std::string to_string(EthTransmitStatus value);
inline std::string to_string(EthState value);
inline std::string to_string(EthMode value);

inline std::string to_string(const EthFrame& msg);
inline std::string to_string(const EthMessage& msg);
inline std::string to_string(const EthTransmitAcknowledge& msg);
inline std::string to_string(const EthStatus& msg);
inline std::string to_string(const EthSetMode& msg);


inline std::ostream& operator<<(std::ostream& out, EthTransmitStatus value);
inline std::ostream& operator<<(std::ostream& out, EthState value);
inline std::ostream& operator<<(std::ostream& out, EthMode value);

inline std::ostream& operator<<(std::ostream& out, const EthFrame& msg);
inline std::ostream& operator<<(std::ostream& out, const EthMessage& msg);
inline std::ostream& operator<<(std::ostream& out, const EthTransmitAcknowledge& msg);
inline std::ostream& operator<<(std::ostream& out, const EthStatus& msg);
inline std::ostream& operator<<(std::ostream& out, const EthSetMode& msg);
    

// ================================================================================
//  Inline Implementations
// ================================================================================
std::string to_string(EthTransmitStatus value)
{
    switch (value)
    {
    case  EthTransmitStatus::Transmitted:
        return "Transmitted";
    case  EthTransmitStatus::ControllerInactive:
        return "ControllerInactive";
    case  EthTransmitStatus::LinkDown:
        return "LinkDown";
    case  EthTransmitStatus::Dropped:
        return "Dropped";
    case  EthTransmitStatus::DuplicatedTransmitId:
        return "DuplicatedTransmitId";
    case  EthTransmitStatus::InvalidFrameFormat:
        return "InvalidFrameFormat";
    };
    throw ib::type_conversion_error{};
}

std::string to_string(EthState value)
{
    switch (value)
    {
    case EthState::Inactive:
        return "Inactive";
    case EthState::LinkDown:
        return "LinkDown";
    case EthState::LinkUp:
        return "LinkUp";
    };
    throw ib::type_conversion_error{};
}

std::string to_string(EthMode value)
{
    switch (value)
    {
    case EthMode::Inactive:
        return "Inactive";
    case EthMode::Active:
        return "Active";
    };
    throw ib::type_conversion_error{};
}

std::string to_string(const EthFrame& msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}


std::string to_string(const EthMessage& msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}

std::string to_string(const EthTransmitAcknowledge& msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}

std::string to_string(const EthStatus& msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}

std::string to_string(const EthSetMode& msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}


    
std::ostream& operator<<(std::ostream& out, EthTransmitStatus value)
{
    return out << to_string(value);
}
std::ostream& operator<<(std::ostream& out, EthState value)
{
    return out << to_string(value);
}
std::ostream& operator<<(std::ostream& out, EthMode value)
{
    return out << to_string(value);
}

std::ostream& operator<<(std::ostream& out, const EthFrame& msg)
{
    return out;
}

std::ostream& operator<<(std::ostream& out, const EthMessage& msg)
{
    auto timestamp = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(msg.timestamp);
    return out
        << "eth::EthMessage{txId=" << msg.transmitId
        << ", " << msg.ethFrame
        << " @" << timestamp.count() << "ms"
        << "}";
}

std::ostream& operator<<(std::ostream& out, const EthTransmitAcknowledge& msg)
{
    auto timestamp = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(msg.timestamp);
    return out
        << "eth::EthTransmitAcknowledge{txId=" << msg.transmitId
        << ", source="
        << ", status=" << msg.status
        << " @" << timestamp.count() << "ms"
        << "}";
}

std::ostream& operator<<(std::ostream& out, const EthStatus& msg)
{
    auto timestamp = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(msg.timestamp);
    return out
        << "eth::EthStatus{"
        << "state=" << msg.state
        << "bitrate=" << msg.bitRate
        << " @" << timestamp.count() << "ms"
        << "}";
}

std::ostream& operator<<(std::ostream& out, const EthSetMode& msg)
{
    return out
        << "eth::EthSetMode{" << msg.mode << "}";
}





} // namespace eth
} // namespace sim
} // namespace ib
