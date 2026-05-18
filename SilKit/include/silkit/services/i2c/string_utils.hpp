// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <ostream>
#include <sstream>

#include "I2cDatatypes.hpp"

#include "silkit/participant/exception.hpp"
#include "silkit/util/PrintableHexString.hpp"
#include "silkit/services/string_utils.hpp"

namespace SilKit {
namespace Services {
namespace I2c {

inline std::string to_string(I2cAddressMode mode);
inline std::string to_string(I2cTransferDirection direction);
inline std::string to_string(I2cSpeedMode mode);
inline std::string to_string(I2cTransmitStatus status);
inline std::string to_string(I2cControllerMode mode);

inline std::string to_string(const I2cFrame& frame);
inline std::string to_string(const I2cFrameEvent& event);
inline std::string to_string(const I2cFrameTransmitEvent& event);

inline std::ostream& operator<<(std::ostream& out, I2cAddressMode mode);
inline std::ostream& operator<<(std::ostream& out, I2cTransferDirection direction);
inline std::ostream& operator<<(std::ostream& out, I2cSpeedMode mode);
inline std::ostream& operator<<(std::ostream& out, I2cTransmitStatus status);
inline std::ostream& operator<<(std::ostream& out, I2cControllerMode mode);

inline std::ostream& operator<<(std::ostream& out, const I2cFrame& frame);
inline std::ostream& operator<<(std::ostream& out, const I2cFrameEvent& event);
inline std::ostream& operator<<(std::ostream& out, const I2cFrameTransmitEvent& event);

// ================================================================================
//  Inline Implementations
// ================================================================================

std::string to_string(I2cAddressMode mode)
{
    switch (mode)
    {
    case I2cAddressMode::AddressMode7Bit:
        return "7Bit";
    case I2cAddressMode::AddressMode10Bit:
        return "10Bit";
    }
    throw SilKit::TypeConversionError{};
}

std::string to_string(I2cTransferDirection direction)
{
    switch (direction)
    {
    case I2cTransferDirection::Write:
        return "Write";
    case I2cTransferDirection::Read:
        return "Read";
    }
    throw SilKit::TypeConversionError{};
}

std::string to_string(I2cSpeedMode mode)
{
    switch (mode)
    {
    case I2cSpeedMode::Standard:
        return "Standard";
    case I2cSpeedMode::Fast:
        return "Fast";
    case I2cSpeedMode::FastPlus:
        return "FastPlus";
    case I2cSpeedMode::HighSpeed:
        return "HighSpeed";
    }
    throw SilKit::TypeConversionError{};
}

std::string to_string(I2cTransmitStatus status)
{
    switch (status)
    {
    case I2cTransmitStatus::Transmitted:
        return "Transmitted";
    case I2cTransmitStatus::AddressNak:
        return "AddressNak";
    case I2cTransmitStatus::DataNak:
        return "DataNak";
    case I2cTransmitStatus::ArbitrationLost:
        return "ArbitrationLost";
    case I2cTransmitStatus::BusError:
        return "BusError";
    }
    throw SilKit::TypeConversionError{};
}

std::string to_string(I2cControllerMode mode)
{
    switch (mode)
    {
    case I2cControllerMode::Inactive:
        return "Inactive";
    case I2cControllerMode::Master:
        return "Master";
    case I2cControllerMode::Slave:
        return "Slave";
    }
    throw SilKit::TypeConversionError{};
}

std::string to_string(const I2cFrame& frame)
{
    std::stringstream outStream;
    outStream << frame;
    return outStream.str();
}

std::string to_string(const I2cFrameEvent& event)
{
    std::stringstream outStream;
    outStream << event;
    return outStream.str();
}

std::string to_string(const I2cFrameTransmitEvent& event)
{
    std::stringstream outStream;
    outStream << event;
    return outStream.str();
}

std::ostream& operator<<(std::ostream& out, I2cAddressMode mode)
{
    return out << to_string(mode);
}

std::ostream& operator<<(std::ostream& out, I2cTransferDirection direction)
{
    return out << to_string(direction);
}

std::ostream& operator<<(std::ostream& out, I2cSpeedMode mode)
{
    return out << to_string(mode);
}

std::ostream& operator<<(std::ostream& out, I2cTransmitStatus status)
{
    return out << to_string(status);
}

std::ostream& operator<<(std::ostream& out, I2cControllerMode mode)
{
    return out << to_string(mode);
}

std::ostream& operator<<(std::ostream& out, const I2cFrame& frame)
{
    return out << "I2c::I2cFrame{address=0x" << std::hex << frame.address << std::dec
               << ", addressMode=" << frame.addressMode << ", direction=" << frame.direction << ", data=["
               << Util::AsHexString(frame.data).WithSeparator(" ").WithMaxLength(8)
               << "], data.size=" << frame.data.size() << "}";
}

std::ostream& operator<<(std::ostream& out, const I2cFrameEvent& event)
{
    auto timestamp = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(event.timestamp);
    return out << "I2c::I2cFrameEvent{userContext=" << event.userContext << ", direction=" << event.direction
               << ", frame=" << event.frame << " @" << timestamp.count() << "ms}";
}

std::ostream& operator<<(std::ostream& out, const I2cFrameTransmitEvent& event)
{
    auto timestamp = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(event.timestamp);
    return out << "I2c::I2cFrameTransmitEvent{address=0x" << std::hex << event.address << std::dec
               << ", status=" << event.status << " @" << timestamp.count() << "ms}";
}

} // namespace I2c
} // namespace Services
} // namespace SilKit
