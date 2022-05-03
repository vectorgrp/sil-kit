// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <iomanip>
#include <sstream>

#include "ib/exception.hpp"
#include "datatypes.hpp"

namespace ib {
namespace sim {

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
    throw ib::TypeConversionError{};
}

std::ostream& operator<<(std::ostream& out, TransmitDirection direction)
{
    try
    {
        return out << to_string(direction);
    }
    catch (const ib::TypeConversionError&)
    {
        return out << "TransmitDirection{" << static_cast<uint8_t>(direction) << "}";
    }
}
    
} // namespace sim
} // namespace ib
