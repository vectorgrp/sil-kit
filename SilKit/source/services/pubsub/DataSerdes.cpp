// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "DataSerdes.hpp"

namespace SilKit {
namespace Services {
namespace PubSub {

inline SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const WireDataMessageEvent& msg)
{
    buffer << msg.data << msg.timestamp;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, WireDataMessageEvent& msg)
{
    buffer >> msg.data >> msg.timestamp;
    return buffer;
}

void Serialize(SilKit::Core::MessageBuffer& buffer, const WireDataMessageEvent& msg)
{
    buffer << msg;
    return;
}

void Deserialize(SilKit::Core::MessageBuffer& buffer, WireDataMessageEvent& out)
{
    buffer >> out;
}

} // namespace PubSub
} // namespace Services
} // namespace SilKit
