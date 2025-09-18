// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/services/ethernet/EthernetDatatypes.hpp"
#include "silkit/services/ethernet/string_utils.hpp"

#include "SharedVector.hpp"

#include <chrono>
#include <vector>

namespace SilKit {
namespace Services {
namespace Ethernet {

struct WireEthernetFrame
{
    //! The Ethernet raw frame without the frame check sequence
    Util::SharedVector<uint8_t> raw;
};

inline auto ToEthernetFrame(const WireEthernetFrame& wireEthernetFrame) -> EthernetFrame;
inline auto MakeWireEthernetFrame(const EthernetFrame& ethernetFrame) -> WireEthernetFrame;

struct WireEthernetFrameEvent
{
    //! Reception time
    std::chrono::nanoseconds timestamp;
    //! The Ethernet frame
    WireEthernetFrame frame;
    //! Receive/Transmit direction
    TransmitDirection direction;
    //! Optional pointer provided by user when sending the frame
    void* userContext;
};

inline auto ToEthernetFrameEvent(const WireEthernetFrameEvent& wireEthernetFrameEvent) -> EthernetFrameEvent;
inline auto MakeWireEthernetFrameEvent(const EthernetFrameEvent& ethernetFrameEvent) -> WireEthernetFrameEvent;

//! \brief Publishes status of the simulated Ethernet controller
struct EthernetStatus
{
    //! Timestamp of the status change.
    std::chrono::nanoseconds timestamp;
    //! State of the Ethernet controller.
    EthernetState state;
    //! Bit rate in kBit/sec of the link when in state LinkUp, otherwise zero.
    EthernetBitrate bitrate;
};

//! \brief Mode for switching an Ethernet Controller on or off
enum class EthernetMode : uint8_t
{
    //! The controller is inactive (default after reset).
    Inactive = 0,
    //! The controller is active.
    Active = 1,
};

//! \brief Set the Mode of the Ethernet Controller.
struct EthernetSetMode
{
    //! EthernetMode that the Ethernet controller should reach.
    EthernetMode mode;
};

inline std::string to_string(const WireEthernetFrame& msg);
inline std::string to_string(EthernetMode value);
inline std::string to_string(const EthernetStatus& msg);
inline std::string to_string(const EthernetSetMode& msg);

inline std::ostream& operator<<(std::ostream& out, const WireEthernetFrameEvent& msg);
inline std::ostream& operator<<(std::ostream& out, const WireEthernetFrame& msg);
inline std::ostream& operator<<(std::ostream& out, EthernetMode value);
inline std::ostream& operator<<(std::ostream& out, const EthernetStatus& msg);
inline std::ostream& operator<<(std::ostream& out, const EthernetSetMode& msg);

// ================================================================================
//  Inline Implementations
// ================================================================================

auto ToEthernetFrame(const WireEthernetFrame& wireEthernetFrame) -> EthernetFrame
{
    return {wireEthernetFrame.raw.AsSpan()};
}

auto MakeWireEthernetFrame(const EthernetFrame& ethernetFrame) -> WireEthernetFrame
{
    constexpr static const size_t minimumEthernetFrameSizeWithoutFcs = 60;
    return {Util::SharedVector<uint8_t>{ethernetFrame.raw, minimumEthernetFrameSizeWithoutFcs}};
}

auto ToEthernetFrameEvent(const WireEthernetFrameEvent& wireEthernetFrameEvent) -> EthernetFrameEvent
{
    return {wireEthernetFrameEvent.timestamp, ToEthernetFrame(wireEthernetFrameEvent.frame),
            wireEthernetFrameEvent.direction, wireEthernetFrameEvent.userContext};
}

auto MakeWireEthernetFrameEvent(const EthernetFrameEvent& ethernetFrameEvent) -> WireEthernetFrameEvent
{
    return {ethernetFrameEvent.timestamp, MakeWireEthernetFrame(ethernetFrameEvent.frame), ethernetFrameEvent.direction,
            ethernetFrameEvent.userContext};
}

std::string to_string(const WireEthernetFrame& msg)
{
    return to_string(ToEthernetFrame(msg));
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
    throw SilKit::TypeConversionError{};
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

std::ostream& operator<<(std::ostream& out, const WireEthernetFrameEvent& msg)
{
    return out << ToEthernetFrameEvent(msg);
}

std::ostream& operator<<(std::ostream& out, const WireEthernetFrame& msg)
{
    return out << to_string(msg);
}

std::ostream& operator<<(std::ostream& out, EthernetMode value)
{
    return out << to_string(value);
}

std::ostream& operator<<(std::ostream& out, const EthernetStatus& msg)
{
    auto timestamp = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(msg.timestamp);
    return out << "EthernetStatus{"
               << "state=" << msg.state << "bitrate=" << msg.bitrate << " @" << timestamp.count() << "ms"
               << "}";
}

std::ostream& operator<<(std::ostream& out, const EthernetSetMode& msg)
{
    return out << "EthernetSetMode{" << msg.mode << "}";
}

} // namespace Ethernet
} // namespace Services
} // namespace SilKit
