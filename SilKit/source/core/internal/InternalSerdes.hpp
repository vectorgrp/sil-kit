// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "MessageBuffer.hpp"
#include "EndpointAddress.hpp"

namespace SilKit {
namespace Core {

inline MessageBuffer& operator<<(MessageBuffer& buffer, const EndpointAddress& addr)
{
    buffer << addr.participant
           << addr.endpoint;
    return buffer;
}

inline MessageBuffer& operator>>(MessageBuffer& buffer, EndpointAddress& addr)
{
    buffer >> addr.participant
           >> addr.endpoint;
    return buffer;
}

} // namespace Core
} // namespace SilKit
