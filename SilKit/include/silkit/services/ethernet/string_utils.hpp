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
#include "silkit/services/string_utils.hpp"

#include "EthernetDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace Ethernet {

inline std::string to_string(EthernetTransmitStatus value);
inline std::string to_string(EthernetState value);

inline std::string to_string(const EthernetFrame& msg);
inline std::string to_string(const EthernetFrameEvent& msg);
inline std::string to_string(const EthernetFrameTransmitEvent& msg);

inline std::ostream& operator<<(std::ostream& out, EthernetTransmitStatus value);
inline std::ostream& operator<<(std::ostream& out, EthernetState value);

inline std::ostream& operator<<(std::ostream& out, const EthernetFrame& msg);
inline std::ostream& operator<<(std::ostream& out, const EthernetFrameEvent& msg);
inline std::ostream& operator<<(std::ostream& out, const EthernetFrameTransmitEvent& msg);

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
    case  EthernetTransmitStatus::InvalidFrameFormat:
        return "InvalidFrameFormat";
    };
    throw SilKit::TypeConversionError{};
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
    throw SilKit::TypeConversionError{};
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

std::ostream& operator<<(std::ostream& out, EthernetTransmitStatus value)
{
    return out << to_string(value);
}

std::ostream& operator<<(std::ostream& out, EthernetState value)
{
    return out << to_string(value);
}

std::ostream& operator<<(std::ostream& out, const EthernetFrame& msg)
{
    if (msg.raw.size() == 0)
    {
        return out
            << "EthernetFrame{size=0}";
    }
    else
    {
        using EthernetMac = std::array<uint8_t, 6>;

        out << "EthernetFrame{size=" << msg.raw.size();
        if (msg.raw.size() >= 2 * sizeof(EthernetMac))
        {
            EthernetMac destinationMac;
            EthernetMac sourceMac;
            std::copy(
                msg.raw.begin(),
                msg.raw.begin() + sizeof(EthernetMac), destinationMac.begin());
            std::copy(
                msg.raw.begin() + sizeof(EthernetMac),
                msg.raw.begin() + 2 * sizeof(EthernetMac), sourceMac.begin());

            out << ", src = " << Util::AsHexString(sourceMac).WithSeparator(":")
                << ", dst=" << Util::AsHexString(destinationMac).WithSeparator(":");
        }
        return out
            << ", data=[" << Util::AsHexString(msg.raw).WithSeparator(" ").WithMaxLength(8)
            << "]}";
    }
}

std::ostream& operator<<(std::ostream& out, const EthernetFrameEvent& msg)
{
    auto timestamp = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(msg.timestamp);
    return out
        << "EthernetFrameEvent{" << msg.frame
        << ", direction=" << msg.direction
        << ", userContext=" << msg.userContext
        << " @" << timestamp.count() << "ms"
        << "}";
}

std::ostream& operator<<(std::ostream& out, const EthernetFrameTransmitEvent& msg)
{
    auto timestamp = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(msg.timestamp);
    out
        << "EthernetFrameTransmitEvent{status=" << msg.status
        << ", userContext=" << msg.userContext
        << " @" << timestamp.count() << "ms"
        << "}";

    return out;
}

} // namespace Ethernet
} // namespace Services
} // namespace SilKit
