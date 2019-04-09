// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "MessageBuffer.hpp"

#include "ib/mw/EndpointAddress.hpp"

namespace ib {
namespace mw {

MessageBuffer& operator<<(MessageBuffer& buffer, const EndpointAddress& addr)
{
    buffer << addr.participant
           << addr.endpoint;
    return buffer;
}
    

MessageBuffer& operator>>(MessageBuffer& buffer, EndpointAddress& addr)
{
    buffer >> addr.participant
           >> addr.endpoint;
    return buffer;
}

    
} // mw
} // namespace ib
