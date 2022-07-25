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

#include "silkit/services/flexray/FlexrayDatatypes.hpp"
#include "silkit/services/flexray/string_utils.hpp"

#include "SharedVector.hpp"

#include <chrono>
#include <vector>

namespace SilKit {
namespace Services {
namespace Flexray {

struct WireFlexrayFrame
{
    FlexrayHeader header; //!< Header flags, slot, crc, and cycle indidcators
    Util::SharedVector<uint8_t> payload; //!< Raw payload containing 0 to 254 bytes
};

inline auto ToFlexrayFrame(const WireFlexrayFrame& wireFlexrayFramea) -> FlexrayFrame;
inline auto MakeWireFlexrayFrame(const FlexrayFrame& flexrayFrame) -> WireFlexrayFrame;

// Receive a frame from the Bus.
struct WireFlexrayFrameEvent
{
    std::chrono::nanoseconds timestamp; //!< Time at end of frame transmission
    FlexrayChannel channel; //!< FlexRay channel A or B. (Valid values: FlexrayChannel::A, FlexrayChannel::B).
    WireFlexrayFrame frame; //!< Received FlexRay frame
};

inline auto ToFlexrayFrameEvent(const WireFlexrayFrameEvent& wireFlexrayFrameEvent) -> FlexrayFrameEvent;
inline auto MakeWireFlexrayFrameEvent(const FlexrayFrameEvent& flexrayFrameEvent) -> WireFlexrayFrameEvent;

struct WireFlexrayFrameTransmitEvent
{
    std::chrono::nanoseconds timestamp; //!< Time at end of frame transmission
    uint16_t txBufferIndex; //!< Tx buffer, that was used for the transmission
    FlexrayChannel channel; //!< FlexRay channel A or B. (Valid values: FlexrayChannel::A, FlexrayChannel::B).
    WireFlexrayFrame frame; //!< Copy of the FlexRay frame that was successfully transmitted
};

inline auto ToFlexrayFrameTransmitEvent(const WireFlexrayFrameTransmitEvent& wireFlexrayFrameTransmitEvent)
    -> FlexrayFrameTransmitEvent;
inline auto MakeWireFlexrayFrameTransmitEvent(const FlexrayFrameTransmitEvent& flexrayFrameTransmitEvent)
    -> WireFlexrayFrameTransmitEvent;

//! Update the content of a FlexRay TX-Buffer
struct WireFlexrayTxBufferUpdate
{
    //! Index of the TX Buffers according to the configured buffers (cf. FlexrayControllerConfig).
    uint16_t txBufferIndex;

    //! Payload data valid flag
    bool payloadDataValid;

    //! Raw payload containing 0 to 254 bytes.
    Util::SharedVector<uint8_t> payload;
};

inline auto ToFlexrayTxBufferUpdate(const WireFlexrayTxBufferUpdate& wireFlexrayTxBufferUpdate)
    -> FlexrayTxBufferUpdate;
inline auto MakeWireFlexrayTxBufferUpdate(const FlexrayTxBufferUpdate& flexrayTxBufferUpdate)
    -> WireFlexrayTxBufferUpdate;

//! Update the configuration of a particular FlexRay TX-Buffer
struct FlexrayTxBufferConfigUpdate
{
    //! Index of the TX-Buffers according to the configured buffers (cf. FlexrayControllerConfig).
    uint16_t txBufferIndex;
    //! The new configuration of the Tx-Buffer
    FlexrayTxBufferConfig txBufferConfig;
};

enum class FlexrayChiCommand : uint8_t
{
    RUN, //!< ChiCommand RUN
    DEFERRED_HALT, //!< ChiCommand DEFERRED_HALT
    FREEZE, //!< ChiCommand FREEZE
    ALLOW_COLDSTART, //!< ChiCommand ALLOW_COLDSTART
    ALL_SLOTS, //!< ChiCommand ALL_SLOTS
    WAKEUP //!< ChiCommand WAKEUP
};

struct FlexrayHostCommand
{
    FlexrayChiCommand command;
};

inline std::string to_string(const WireFlexrayFrameEvent& msg);
inline std::string to_string(const WireFlexrayFrameTransmitEvent& msg);
inline std::string to_string(const WireFlexrayTxBufferUpdate& msg);
inline std::string to_string(const FlexrayTxBufferConfigUpdate& msg);
inline std::string to_string(FlexrayChiCommand command);
inline std::string to_string(const FlexrayHostCommand& msg);

inline std::ostream& operator<<(std::ostream& out, const WireFlexrayFrameEvent& msg);
inline std::ostream& operator<<(std::ostream& out, const WireFlexrayFrameTransmitEvent& msg);
inline std::ostream& operator<<(std::ostream& out, const WireFlexrayTxBufferUpdate& msg);
inline std::ostream& operator<<(std::ostream& out, const FlexrayTxBufferConfigUpdate& msg);
inline std::ostream& operator<<(std::ostream& out, FlexrayChiCommand command);
inline std::ostream& operator<<(std::ostream& out, const FlexrayHostCommand& msg);

// ================================================================================
//  Inline Implementations
// ================================================================================

auto ToFlexrayFrame(const WireFlexrayFrame& wireFlexrayFramea) -> FlexrayFrame
{
    return {wireFlexrayFramea.header, wireFlexrayFramea.payload.AsSpan()};
}

auto MakeWireFlexrayFrame(const FlexrayFrame& flexrayFrame) -> WireFlexrayFrame
{
    return {flexrayFrame.header, flexrayFrame.payload};
}

auto ToFlexrayFrameEvent(const WireFlexrayFrameEvent& wireFlexrayFrameEvent) -> FlexrayFrameEvent
{
    return {wireFlexrayFrameEvent.timestamp, wireFlexrayFrameEvent.channel,
            ToFlexrayFrame(wireFlexrayFrameEvent.frame)};
}

auto MakeWireFlexrayFrameEvent(const FlexrayFrameEvent& flexrayFrameEvent) -> WireFlexrayFrameEvent
{
    return {flexrayFrameEvent.timestamp, flexrayFrameEvent.channel, MakeWireFlexrayFrame(flexrayFrameEvent.frame)};
}

auto ToFlexrayFrameTransmitEvent(const WireFlexrayFrameTransmitEvent& wireFlexrayFrameTransmitEvent)
    -> FlexrayFrameTransmitEvent
{
    return {wireFlexrayFrameTransmitEvent.timestamp, wireFlexrayFrameTransmitEvent.txBufferIndex,
            wireFlexrayFrameTransmitEvent.channel, ToFlexrayFrame(wireFlexrayFrameTransmitEvent.frame)};
}

auto MakeWireFlexrayFrameTransmitEvent(const FlexrayFrameTransmitEvent& flexrayFrameTransmitEvent)
    -> WireFlexrayFrameTransmitEvent
{
    return {flexrayFrameTransmitEvent.timestamp, flexrayFrameTransmitEvent.txBufferIndex,
            flexrayFrameTransmitEvent.channel, MakeWireFlexrayFrame(flexrayFrameTransmitEvent.frame)};
}

auto ToFlexrayTxBufferUpdate(const WireFlexrayTxBufferUpdate& wireFlexrayTxBufferUpdate) -> FlexrayTxBufferUpdate
{
    return {wireFlexrayTxBufferUpdate.txBufferIndex, wireFlexrayTxBufferUpdate.payloadDataValid,
            wireFlexrayTxBufferUpdate.payload.AsSpan()};
}

auto MakeWireFlexrayTxBufferUpdate(const FlexrayTxBufferUpdate& flexrayTxBufferUpdate) -> WireFlexrayTxBufferUpdate
{
    return {flexrayTxBufferUpdate.txBufferIndex, flexrayTxBufferUpdate.payloadDataValid, flexrayTxBufferUpdate.payload};
}

std::string to_string(const WireFlexrayFrameEvent& msg)
{
    return to_string(ToFlexrayFrameEvent(msg));
}

std::string to_string(const WireFlexrayFrameTransmitEvent& msg)
{
    return to_string(ToFlexrayFrameTransmitEvent(msg));
}

std::string to_string(const WireFlexrayTxBufferUpdate& msg)
{
    return to_string(ToFlexrayTxBufferUpdate(msg));
}

std::string to_string(const FlexrayTxBufferConfigUpdate& msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}

std::string to_string(FlexrayChiCommand command)
{
    switch (command)
    {
    case FlexrayChiCommand::RUN: return "RUN";
    case FlexrayChiCommand::DEFERRED_HALT: return "DEFERRED_HALT";
    case FlexrayChiCommand::FREEZE: return "FREEZE";
    case FlexrayChiCommand::ALLOW_COLDSTART: return "ALLOW_COLDSTART";
    case FlexrayChiCommand::ALL_SLOTS: return "ALL_SLOTS";
    case FlexrayChiCommand::WAKEUP: return "WAKEUP";
    };
    throw SilKit::TypeConversionError{};
}

std::string to_string(const FlexrayHostCommand& msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}

std::ostream& operator<<(std::ostream& out, const WireFlexrayFrameEvent& msg)
{
    return out << ToFlexrayFrameEvent(msg);
}

std::ostream& operator<<(std::ostream& out, const WireFlexrayFrameTransmitEvent& msg)
{
    return out << ToFlexrayFrameTransmitEvent(msg);
}

std::ostream& operator<<(std::ostream& out, const WireFlexrayTxBufferUpdate& msg)
{
    return out << ToFlexrayTxBufferUpdate(msg);
}

std::ostream& operator<<(std::ostream& out, const FlexrayTxBufferConfigUpdate& msg)
{
    return out << "fr::FlexrayTxBufferConfigUpdate{"
               << "idx=" << msg.txBufferIndex << " " << msg.txBufferConfig << "}";
}

std::ostream& operator<<(std::ostream& out, FlexrayChiCommand command)
{
    return out << to_string(command);
}

std::ostream& operator<<(std::ostream& out, const FlexrayHostCommand& msg)
{
    return out << "fr::FlexrayHostCommand{" << msg.command << "}";
}

} // namespace Flexray
} // namespace Services
} // namespace SilKit
