// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <sstream>

#include "TraceMessage.hpp"

namespace SilKit {

inline std::string to_string(const TraceMessage&);
inline std::string to_string(TraceMessageType);

inline std::ostream& operator<<(std::ostream& out, const TraceMessage&);
inline std::ostream& operator<<(std::ostream& out, TraceMessageType);

// ================================================================================
//  Inline Implementations
// ================================================================================

std::ostream& operator<<(std::ostream& out, const TraceMessage& msg)
{
    return out << "TraceMessage<" << msg.Type() << ">";
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
    case TraceMessageType::EthernetFrame:
        return "EthernetFrame";
    case TraceMessageType::CanFrameEvent:
        return "CanFrameEvent";
    case TraceMessageType::LinFrame:
        return "LinFrame";
    case TraceMessageType::DataMessageEvent:
        return "DataMessageEvent";
    case TraceMessageType::FlexrayFrameEvent:
        return "FlaxrayFrameEvent";
    default:
        throw SilKitError("Unknown TraceMessage::Type in operator<<!");
    }
}

} // namespace SilKit
