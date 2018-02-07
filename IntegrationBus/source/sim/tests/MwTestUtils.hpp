// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include <iostream>

#include "ib/mw/EndpointAddress.hpp"

inline std::ostream& operator<<(std::ostream& out, const ib::mw::EndpointAddress& ibAddress);



// ================================================================================
//  Inline Implementations
// ================================================================================
std::ostream& operator<<(std::ostream& out, const ib::mw::EndpointAddress& ibAddress)
{
    out << "{" << ibAddress.participant
        << ", " << ibAddress.endpoint
        << "}";
    return out;
}
