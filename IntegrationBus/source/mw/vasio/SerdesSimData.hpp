// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "MessageBuffer.hpp"

#include "ib/sim/data/DataMessageDatatypes.hpp"

namespace ib {
namespace sim {
namespace data {

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const DataMessageEvent& msg)
{
    buffer << msg.data
           << msg.timestamp;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, DataMessageEvent& msg)
{
    buffer >> msg.data
           >> msg.timestamp;
    return buffer;
}

} // namespace data    
} // namespace sim
} // namespace ib
