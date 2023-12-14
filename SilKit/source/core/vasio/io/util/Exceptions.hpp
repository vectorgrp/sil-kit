// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/participant/exception.hpp"
#include <exception>


namespace VSilKit {


struct InvalidStateError : SilKit::SilKitError
{
    InvalidStateError()
        : SilKit::SilKitError{"object in in an invalid state"}
    {
    }
};


struct InvalidAsioEndpointProtocolFamily : SilKit::SilKitError
{
    InvalidAsioEndpointProtocolFamily()
        : SilKit::SilKitError{"invalid asio endpoint protocol family"}
    {
    }
};


} // namespace VSilKit


namespace SilKit {
namespace Core {
using VSilKit::InvalidStateError;
using VSilKit::InvalidAsioEndpointProtocolFamily;
} // namespace Core
} // namespace SilKit
