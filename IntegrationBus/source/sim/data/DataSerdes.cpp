// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "DataSerdes.hpp"

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
void Serialize(ib::mw::MessageBuffer& buffer, const DataMessageEvent& msg)
{
    buffer << msg;
    return;
}
void Deserialize(ib::mw::MessageBuffer& buffer, DataMessageEvent& out)
{
    buffer >> out;
}

} // namespace data    
} // namespace sim
} // namespace ib
