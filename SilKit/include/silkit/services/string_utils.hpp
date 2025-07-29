// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <iomanip>
#include <sstream>

#include "silkit/participant/exception.hpp"
#include "datatypes.hpp"

namespace SilKit {
namespace Services {

inline std::string to_string(TransmitDirection direction);

inline std::ostream& operator<<(std::ostream& out, TransmitDirection direction);

// ================================================================================
//  Inline Implementations
// ================================================================================
std::string to_string(TransmitDirection direction)
{
    switch (direction)
    {
    case TransmitDirection::Undefined:
        return "Undefined";
    case TransmitDirection::TX:
        return "TX";
    case TransmitDirection::RX:
        return "RX";
    case TransmitDirection::TXRX:
        return "TXRX";
    }
    throw SilKit::TypeConversionError{};
}

std::ostream& operator<<(std::ostream& out, TransmitDirection direction)
{
    try
    {
        return out << to_string(direction);
    }
    catch (const SilKit::TypeConversionError&)
    {
        return out << "TransmitDirection{" << static_cast<uint8_t>(direction) << "}";
    }
}

} // namespace Services
} // namespace SilKit
