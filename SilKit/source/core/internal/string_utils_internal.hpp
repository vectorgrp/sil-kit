// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <iostream>
#include <sstream>

#include "EndpointAddress.hpp"

namespace SilKit {
namespace Core {

inline std::string to_string(const SilKit::Core::EndpointAddress& address);

inline std::ostream& operator<<(std::ostream& out, const SilKit::Core::EndpointAddress& address);

// ================================================================================
//  Inline Implementations
// ================================================================================

std::string to_string(const SilKit::Core::EndpointAddress& address)
{
    std::stringstream outStream;
    outStream << address;
    return outStream.str();
}

std::ostream& operator<<(std::ostream& out, const SilKit::Core::EndpointAddress& address)
{
    out << "Addr{" << address.participant << ", " << address.endpoint << "}";
    return out;
}

} // namespace Core
} // namespace SilKit
