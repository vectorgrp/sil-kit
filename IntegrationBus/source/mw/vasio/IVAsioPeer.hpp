// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

namespace ib {
namespace mw {

class MessageBuffer;
struct VAsioMsgSubscriber;

class IVAsioPeer
{
public:
    virtual ~IVAsioPeer() = default;
    virtual void SendIbMsg(MessageBuffer buffer) = 0;
    virtual void Subscribe(VAsioMsgSubscriber subscriber) = 0;
};

} // mw
} // namespace ib
