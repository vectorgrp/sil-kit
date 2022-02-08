// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "MessageBuffer.hpp"

#include "ib/sim/data/DataMessageDatatypes.hpp"

namespace ib {
namespace sim {
namespace data {

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const DataMessage& msg)
{
    buffer << msg.data;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, DataMessage& msg)
{
    buffer >> msg.data;
    return buffer;
}

} // namespace data    
} // namespace sim
} // namespace ib
