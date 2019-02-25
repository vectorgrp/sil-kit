// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include <iostream>
#include <sstream>

#include "EndpointAddress.hpp"

namespace ib {
namespace mw {

inline std::string to_string(const ib::mw::EndpointAddress& ibAddress);

inline std::ostream& operator<<(std::ostream& out, const ib::mw::EndpointAddress& ibAddress);

// ================================================================================
//  Inline Implementations
// ================================================================================
std::string to_string(const ib::mw::EndpointAddress& ibAddress)
{
    std::stringstream outStream;
    outStream << ibAddress;
    return outStream.str();
}

std::ostream& operator<<(std::ostream& out, const ib::mw::EndpointAddress& ibAddress)
{
    out << "Addr{" << ibAddress.participant
        << ", " << ibAddress.endpoint
        << "}";
    return out;
}

} // namespace mw
} // namespace ib
