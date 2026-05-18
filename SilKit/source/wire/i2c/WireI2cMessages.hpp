// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/services/i2c/I2cDatatypes.hpp"
#include "silkit/services/i2c/string_utils.hpp"

#include "SharedVector.hpp"

#include <chrono>

namespace SilKit {
namespace Services {
namespace I2c {

struct WireI2cFrame
{
    I2cAddress                  address;     //!< Target slave address
    I2cAddressMode              addressMode; //!< 7-bit or 10-bit addressing
    I2cTransferDirection        direction;   //!< Write or Read
    Util::SharedVector<uint8_t> data;        //!< Payload for Write transfers or slave Read response
};

inline auto ToI2cFrame(const WireI2cFrame& wire) -> I2cFrame;
inline auto MakeWireI2cFrame(const I2cFrame& frame) -> WireI2cFrame;

/*! \brief Event carrying an I2C frame as observed on the bus. */
struct WireI2cFrameEvent
{
    std::chrono::nanoseconds timestamp;   //!< Simulation time of the transfer
    WireI2cFrame             frame;       //!< The I2C frame
    TransmitDirection        direction;   //!< TX (sent by this controller) or RX (received)
    void*                    userContext; //!< Optional pointer provided by user when sending the frame
};

inline auto ToI2cFrameEvent(const WireI2cFrameEvent& wire) -> I2cFrameEvent;
inline auto MakeWireI2cFrameEvent(const I2cFrameEvent& event) -> WireI2cFrameEvent;

/*! \brief Acknowledgment sent from the trivial simulator back to the master. */
struct I2cAcknowledge
{
    std::chrono::nanoseconds timestamp;   //!< Simulation time of the acknowledgment
    I2cAddress               address;     //!< Slave address of the completed transfer
    I2cTransmitStatus        status;      //!< Outcome of the transfer
    void*                    userContext; //!< User pointer passed through from SendFrame
};

/*! \brief Controller configuration broadcast during Init(). */
struct WireI2cControllerConfig
{
    I2cControllerMode mode;             //!< Master or Slave
    I2cAddress        slaveAddress;     //!< Own address when in Slave mode; ignored for Master
    I2cAddressMode    slaveAddressMode; //!< Address width when in Slave mode; ignored for Master
    I2cSpeedMode      speedMode;        //!< Bus speed (informational)
};

inline bool operator==(const WireI2cFrame& lhs, const WireI2cFrame& rhs);
inline bool operator==(const WireI2cFrameEvent& lhs, const WireI2cFrameEvent& rhs);
inline bool operator==(const I2cAcknowledge& lhs, const I2cAcknowledge& rhs);
inline bool operator==(const WireI2cControllerConfig& lhs, const WireI2cControllerConfig& rhs);

inline std::string to_string(const WireI2cFrame& frame);
inline std::string to_string(const WireI2cFrameEvent& event);
inline std::string to_string(const I2cAcknowledge& ack);
inline std::string to_string(const WireI2cControllerConfig& config);

inline std::ostream& operator<<(std::ostream& out, const WireI2cFrame& frame);
inline std::ostream& operator<<(std::ostream& out, const WireI2cFrameEvent& event);
inline std::ostream& operator<<(std::ostream& out, const I2cAcknowledge& ack);
inline std::ostream& operator<<(std::ostream& out, const WireI2cControllerConfig& config);

// ================================================================================
//  Inline Implementations
// ================================================================================

auto ToI2cFrame(const WireI2cFrame& wire) -> I2cFrame
{
    return {wire.address, wire.addressMode, wire.direction, wire.data.AsSpan()};
}

auto MakeWireI2cFrame(const I2cFrame& frame) -> WireI2cFrame
{
    return {frame.address, frame.addressMode, frame.direction, frame.data};
}

auto ToI2cFrameEvent(const WireI2cFrameEvent& wire) -> I2cFrameEvent
{
    return {wire.timestamp, ToI2cFrame(wire.frame), wire.direction, wire.userContext};
}

auto MakeWireI2cFrameEvent(const I2cFrameEvent& event) -> WireI2cFrameEvent
{
    return {event.timestamp, MakeWireI2cFrame(event.frame), event.direction, event.userContext};
}

bool operator==(const WireI2cFrame& lhs, const WireI2cFrame& rhs)
{
    return lhs.address == rhs.address && lhs.addressMode == rhs.addressMode && lhs.direction == rhs.direction
           && Util::ItemsAreEqual(lhs.data, rhs.data);
}

bool operator==(const WireI2cFrameEvent& lhs, const WireI2cFrameEvent& rhs)
{
    return lhs.timestamp == rhs.timestamp && lhs.frame == rhs.frame && lhs.direction == rhs.direction
           && lhs.userContext == rhs.userContext;
}

bool operator==(const I2cAcknowledge& lhs, const I2cAcknowledge& rhs)
{
    return lhs.timestamp == rhs.timestamp && lhs.address == rhs.address && lhs.status == rhs.status
           && lhs.userContext == rhs.userContext;
}

bool operator==(const WireI2cControllerConfig& lhs, const WireI2cControllerConfig& rhs)
{
    return lhs.mode == rhs.mode && lhs.slaveAddress == rhs.slaveAddress
           && lhs.slaveAddressMode == rhs.slaveAddressMode && lhs.speedMode == rhs.speedMode;
}

std::ostream& operator<<(std::ostream& out, const WireI2cFrame& frame)
{
    return out << ToI2cFrame(frame);
}

std::ostream& operator<<(std::ostream& out, const WireI2cFrameEvent& event)
{
    return out << ToI2cFrameEvent(event);
}

std::ostream& operator<<(std::ostream& out, const I2cAcknowledge& ack)
{
    auto timestamp = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(ack.timestamp);
    return out << "I2c::I2cAcknowledge{address=0x" << std::hex << ack.address << std::dec
               << ", status=" << ack.status << " @" << timestamp.count() << "ms}";
}

std::ostream& operator<<(std::ostream& out, const WireI2cControllerConfig& config)
{
    return out << "I2c::WireI2cControllerConfig{mode=" << config.mode << ", slaveAddress=0x" << std::hex
               << config.slaveAddress << std::dec << ", slaveAddressMode=" << config.slaveAddressMode
               << ", speedMode=" << config.speedMode << "}";
}

std::string to_string(const WireI2cFrame& frame)
{
    return to_string(ToI2cFrame(frame));
}

std::string to_string(const WireI2cFrameEvent& event)
{
    std::stringstream out;
    out << event;
    return out.str();
}

std::string to_string(const I2cAcknowledge& ack)
{
    std::stringstream out;
    out << ack;
    return out.str();
}

std::string to_string(const WireI2cControllerConfig& config)
{
    std::stringstream out;
    out << config;
    return out.str();
}

} // namespace I2c
} // namespace Services
} // namespace SilKit
