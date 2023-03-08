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
    case TraceMessageType::EthernetFrame: return "EthernetFrame";
    case TraceMessageType::CanFrameEvent: return "CanFrameEvent";
    case TraceMessageType::LinFrame: return "LinFrame";
    case TraceMessageType::DataMessageEvent: return "DataMessageEvent";
    case TraceMessageType::FlexrayFrameEvent: return "FlaxrayFrameEvent";
    default: throw SilKitError("Unknown TraceMessage::Type in operator<<!");
    }
}

} // namespace SilKit
