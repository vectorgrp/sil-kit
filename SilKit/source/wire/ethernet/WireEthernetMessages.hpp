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
    Util::SharedVector<uint8_t> raw; //!< The Ethernet raw frame without the frame check sequence
};

inline auto ToEthernetFrame(const WireEthernetFrame& wireEthernetFrame) -> EthernetFrame;
inline auto MakeWireEthernetFrame(const EthernetFrame& ethernetFrame) -> WireEthernetFrame;

struct WireEthernetFrameEvent
{
    std::chrono::nanoseconds timestamp; //!< Reception time
    WireEthernetFrame frame;            //!< The Ethernet frame
    TransmitDirection direction;        //!< Receive/Transmit direction
    void* userContext;                  //!< Optional pointer provided by user when sending the frame
};

inline auto ToEthernetFrameEvent(const WireEthernetFrameEvent& wireEthernetFrameEvent) -> EthernetFrameEvent;
inline auto MakeWireEthernetFrameEvent(const EthernetFrameEvent& ethernetFrameEvent) -> WireEthernetFrameEvent;

//! \brief Publishes status of the simulated Ethernet controller
struct EthernetStatus
{
    std::chrono::nanoseconds timestamp; //!< Timestamp of the status change.
    EthernetState state;                //!< State of the Ethernet controller.
    EthernetBitrate bitrate;            //!< Bit rate in kBit/sec of the link when in state LinkUp, otherwise zero.
};

//! \brief Mode for switching an Ethernet Controller on or off
enum class EthernetMode : uint8_t
{
    Inactive = 0, //!< The controller is inactive (default after reset).
    Active = 1,   //!< The controller is active.
};

//! \brief Set the Mode of the Ethernet Controller.
struct EthernetSetMode
{
    EthernetMode mode; //!< EthernetMode that the Ethernet controller should reach.
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
    return out << "EthernetStatus{" << "state=" << msg.state << "bitrate=" << msg.bitrate << " @" << timestamp.count()
               << "ms" << "}";
}

std::ostream& operator<<(std::ostream& out, const EthernetSetMode& msg)
{
    return out << "EthernetSetMode{" << msg.mode << "}";
}

} // namespace Ethernet
} // namespace Services
} // namespace SilKit
