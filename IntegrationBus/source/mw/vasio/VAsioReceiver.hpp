// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/IIbMessageReceiver.hpp"

#include "VAsioMessageSubscriber.hpp"

namespace ib {
namespace mw {

class MessageBuffer;

struct IVAsioReceiver
{
    virtual ~IVAsioReceiver() = default;
    auto& GetDescriptor() const { return _subscriber; }
    auto& GetDescriptor() { return _subscriber; }
    void SetReceiverIdx(uint16_t receiverIdx) { _subscriber.receiverIdx = receiverIdx; }

    virtual void ReceiveMsg(MessageBuffer&& buffer) = 0;

private:
    VAsioMsgSubscriber _subscriber;
};

template <class MsgT>
struct VAsioReceiver : public IVAsioReceiver
{
    auto AddReceiver(ib::mw::IIbMessageReceiver<MsgT>* receiver)
    {
        _receivers.push_back(receiver);
    }
    virtual void ReceiveMsg(MessageBuffer&& buffer)
    {
        EndpointAddress from;
        MsgT msg;
        buffer >> from >> msg;

        for (auto&& receiver : _receivers)
        {
            try
            {
                receiver->ReceiveIbMessage(from, msg);
            }
            catch (const std::exception& e)
            {
                std::cerr
                    << "WARNING: Callback for "
                    << GetDescriptor().msgTypeName
                    << "[\"" << GetDescriptor().linkName << "\"]"
                    << " threw an exception: " << e.what() << "." << std::endl;
            }
            catch (...)
            {
                std::cerr
                    << "WARNING: Callback for "
                    << GetDescriptor().msgTypeName
                    << "[\"" << GetDescriptor().linkName << "\"]"
                    << " threw an unknown exception." << std::endl;
            }
        }
    }
    std::vector<ib::mw::IIbMessageReceiver<MsgT>*> _receivers;
};

} // mw
} // namespace ib
