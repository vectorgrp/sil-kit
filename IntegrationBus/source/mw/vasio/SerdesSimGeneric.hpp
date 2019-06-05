// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "MessageBuffer.hpp"

#include "ib/sim/generic/GenericMessageDatatypes.hpp"

namespace ib {
namespace sim {
namespace generic {

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const GenericMessage& msg)
{
    buffer << msg.data;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, GenericMessage& msg)
{
    buffer >> msg.data;
    return buffer;
}

} // namespace generic    
} // namespace sim
} // namespace ib
