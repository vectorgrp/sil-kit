// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.

#pragma once

#include <sstream>
#include "ib/extensions/TraceMessage.hpp"

namespace ib {
namespace extensions {

inline std::string to_string(const TraceMessage&);
inline std::string to_string(TraceMessageType);

inline std::ostream& operator<<(std::ostream& out, const TraceMessage&);
inline std::ostream& operator<<(std::ostream& out, TraceMessageType);

// ================================================================================
//  Inline Implementations
// ================================================================================
std::ostream& operator<<(std::ostream& out, const TraceMessage& msg)
{
    return out << "TraceMessage<"<< msg.Type()  << ">";
}

std::ostream& operator<<(std::ostream& out, TraceMessageType type)
{
    return out << to_string(type);
}

std::string to_string(const TraceMessage& msg)
{
    std::stringstream ss;
    ss << msg;
    return ss.str();
}

std::string to_string(TraceMessageType type)
{
    switch (type)
    {
    case TraceMessageType::EthFrame: return "EthFrame"; 
    case TraceMessageType::CanMessage: return "CanMessage"; 
    case TraceMessageType::LinFrame: return "LinFrame"; 
    case TraceMessageType::GenericMessage: return "GenericMessage"; 
    case TraceMessageType::FrMessage: return "FrMessage";
    default:
        throw std::runtime_error("Unknown TraceMessage::Type in operator<<!");
    }
}
} //end namespace extensions
} //end namespace ib
