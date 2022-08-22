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

#include "CanDatatypes.hpp"

#include "silkit/participant/exception.hpp"
#include "silkit/util/PrintableHexString.hpp"
#include "silkit/services/string_utils.hpp"

namespace SilKit {
namespace Services {
namespace Can {

inline std::string to_string(CanFrameFlag flags);
inline std::string to_string(CanControllerState state);
inline std::string to_string(CanErrorState state);
inline std::string to_string(CanTransmitStatus status);

inline std::string to_string(const CanFrame& msg);
inline std::string to_string(const CanFrameEvent& msg);
inline std::string to_string(const CanFrameTransmitEvent& ack);

inline std::ostream& operator<<(std::ostream& out, CanFrameFlag flag);
inline std::ostream& operator<<(std::ostream& out, CanControllerState state);
inline std::ostream& operator<<(std::ostream& out, CanErrorState state);
inline std::ostream& operator<<(std::ostream& out, CanTransmitStatus status);

inline std::ostream& operator<<(std::ostream& out, const CanFrame& msg);
inline std::ostream& operator<<(std::ostream& out, const CanFrameEvent& msg);
inline std::ostream& operator<<(std::ostream& out, const CanFrameTransmitEvent& status);

// ================================================================================
//  Inline Implementations
// ================================================================================

std::string to_string(CanFrameFlag flags)
{
    std::stringstream outStream;
    outStream << flags;
    return outStream.str();
}

std::string to_string(CanControllerState state)
{
    switch (state)
    {
    case CanControllerState::Uninit:
        return "Uninit";
    case CanControllerState::Stopped:
        return "Stopped";
    case CanControllerState::Started:
        return "Started";
    case CanControllerState::Sleep:
        return "Sleep";
    };
    throw SilKit::TypeConversionError{};
}

std::string to_string(CanErrorState state)
{
    switch (state)
    {
    case CanErrorState::NotAvailable:
        return "NotAvailable";
    case CanErrorState::ErrorActive:
        return "ErrorActive";
    case CanErrorState::ErrorPassive:
        return "ErrorPassive";
    case CanErrorState::BusOff:
        return "BusOff";
    }
    throw SilKit::TypeConversionError{};
}

std::string to_string(CanTransmitStatus status)
{
    switch (status)
    {
    case CanTransmitStatus::Transmitted:
        return "Transmitted";
    case CanTransmitStatus::Canceled:
        return "Canceled";
    case CanTransmitStatus::TransmitQueueFull:
        return "TransmitQueueFull";
    }
    throw SilKit::TypeConversionError{};
}

std::string to_string(const CanFrame& msg)
{
    std::stringstream outStream;
    outStream << msg;
    return outStream.str();
}

std::string to_string(const CanFrameEvent& msg)
{
    std::stringstream outStream;
    outStream << msg;
    return outStream.str();
}

std::string to_string(const CanFrameTransmitEvent& ack)
{
    std::stringstream outStream;
    outStream << ack;
    return outStream.str();
}

std::ostream& operator<<(std::ostream& out, CanFrameFlag flag)
{
    switch (flag)
    {
    case CanFrameFlag::Ide: return out << "ide";
    case CanFrameFlag::Rtr: return out << "rtr";
    case CanFrameFlag::Fdf: return out << "fdf";
    case CanFrameFlag::Brs: return out << "brs";
    case CanFrameFlag::Esi: return out << "esi";
    case CanFrameFlag::Xlf: return out << "xlf";
    case CanFrameFlag::Sec: return out << "sec";
    }

    return out << "unknown(" << static_cast<uint32_t>(flag) << ")";
}

std::ostream& operator<<(std::ostream& out, CanControllerState state)
{
    return out << to_string(state);
}

std::ostream& operator<<(std::ostream& out, CanErrorState state)
{
    return out << to_string(state);
}

std::ostream& operator<<(std::ostream& out, CanTransmitStatus status)
{
    return out << to_string(status);
}

std::ostream& operator<<(std::ostream& out, const CanFrame& msg)
{
    out << "Can::CanFrame{"
        << ", canId=" << msg.canId
        << ", flags=";

    auto printFlag = [firstFlag = true, &out, &msg](const CanFrameFlag flag) mutable {
        if (!(msg.flags & static_cast<CanFrameFlagMask>(flag)))
        {
            return;
        }

        if (firstFlag)
        {
            firstFlag = false;
        }
        else
        {
            out << ",";
        }

        out << flag;
    };

    // print all flags (including potentially unknown ones)
    out << '[';
    for (uint32_t bit = 0; bit != 32u; ++bit)
    {
        printFlag(static_cast<CanFrameFlag>(BIT(bit)));
    }
    out << ']';

    return out
        << ", dlc=" << static_cast<uint32_t>(msg.dlc)
        << ", sdt=" << static_cast<uint32_t>(msg.sdt)
        << ", vcid=" << static_cast<uint32_t>(msg.vcid)
        << ", af=" << static_cast<uint32_t>(msg.af)
        << ", data=[" << Util::AsHexString(msg.dataField).WithSeparator(" ").WithMaxLength(8)
        << "], data.size=" << msg.dataField.size() << "}";
}

std::ostream& operator<<(std::ostream& out, const CanFrameEvent& msg)
{
    auto timestamp = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(msg.timestamp);
    return out
        << "Can::CanFrameEvent{userContext=" << msg.userContext
        << ", direction=" << msg.direction
        << ", frame=" << msg.frame
        << " @" << timestamp.count() << "ms}";
}

std::ostream& operator<<(std::ostream& out, const CanFrameTransmitEvent& status)
{
    auto timestamp = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(status.timestamp);
    return out
        << "Can::CanTtransmitAcknowledge{status=" << status.status
        << " @" << timestamp.count() << "ms}";
}

} // namespace Can
} // namespace Services
} // namespace SilKit
