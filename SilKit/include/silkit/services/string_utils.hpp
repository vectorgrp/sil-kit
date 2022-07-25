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
