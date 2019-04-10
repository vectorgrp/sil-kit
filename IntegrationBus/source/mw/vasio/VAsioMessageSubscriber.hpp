// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "MessageBuffer.hpp"

namespace ib {
namespace mw {

struct VAsioMsgSubscriber
{
    uint16_t    receiverIdx;
    std::string linkName;
    std::string msgTypeName;
};

inline MessageBuffer& operator<<(MessageBuffer& buffer, const VAsioMsgSubscriber& subscriber)
{
    buffer << subscriber.receiverIdx
           << subscriber.linkName
           << subscriber.msgTypeName;
    return buffer;
}

inline MessageBuffer& operator>>(MessageBuffer& buffer, VAsioMsgSubscriber& subscriber)
{
    buffer >> subscriber.receiverIdx
           >> subscriber.linkName
           >> subscriber.msgTypeName;
    return buffer;
}

} // mw
} // namespace ib
