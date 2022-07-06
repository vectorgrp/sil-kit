// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "DataSerdes.hpp"

namespace SilKit {
namespace Services {
namespace PubSub {

inline SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const DataMessageEvent& msg)
{
    buffer << msg.data
           << msg.timestamp;
    return buffer;
}
inline SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, DataMessageEvent& msg)
{
    buffer >> msg.data
           >> msg.timestamp;
    return buffer;
}
void Serialize(SilKit::Core::MessageBuffer& buffer, const DataMessageEvent& msg)
{
    buffer << msg;
    return;
}
void Deserialize(SilKit::Core::MessageBuffer& buffer, DataMessageEvent& out)
{
    buffer >> out;
}

} // namespace PubSub    
} // namespace Services
} // namespace SilKit
