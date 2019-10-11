// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/IIbMessageReceiver.hpp"

#include "VAsioMsgKind.hpp"
#include "IVAsioPeer.hpp"
#include <type_traits>

namespace ib {
namespace mw {

//auxiliary class for conditional compilation using ib message traits
template<typename MsgT, std::size_t MsgHistSize>
struct MessageHistory {};
// MessageHistory<.., 0>: message history is disabled
template<typename MsgT> struct MessageHistory<MsgT, 0>
{
    void Save(EndpointAddress, const MsgT& ) {}
    void NotifyPeer(IVAsioPeer*, uint16_t) {}
};
// MessageHistory<.., 1>: save last message and notify peers about it
template<typename MsgT> struct MessageHistory<MsgT, 1>
{
    void Save(EndpointAddress from , const MsgT& msg)
    {
        _from = from;
        _last = msg;
    }
    void NotifyPeer(IVAsioPeer* peer, uint16_t remoteIdx)
    {
        ib::mw::MessageBuffer buffer;
        uint32_t msgSizePlaceholder{ 0u };
        buffer
            << msgSizePlaceholder
            << VAsioMsgKind::IbSimMsg
            << remoteIdx
            << _from
            << _last
            ;
        peer->SendIbMsg(std::move(buffer));
    }
private:
    MsgT _last;
    EndpointAddress _from;
};


template <class MsgT>
class VAsioTransmitter : public IIbMessageReceiver<MsgT>
{
    using History = MessageHistory<MsgT, IbMsgTraits<MsgT>::HistSize()>;
    History _hist;
public:
    // ----------------------------------------
    // Public methods
    void AddRemoteReceiver(IVAsioPeer* peer, uint16_t remoteIdx)
    {
        _remoteReceivers.push_back({peer, remoteIdx});
        _hist.NotifyPeer(peer, remoteIdx);
    }


public:
    // ----------------------------------------
    // Public interface methods
    void ReceiveIbMessage(EndpointAddress from, const MsgT& msg) override
    {
        _hist.Save(from, msg);
        for (auto& receiver : _remoteReceivers)
        {
            ib::mw::MessageBuffer buffer;
            uint32_t msgSizePlaceholder{0u};
            buffer
                << msgSizePlaceholder
                << VAsioMsgKind::IbSimMsg
                << receiver.remoteIdx
                << from << msg;
            receiver.peer->SendIbMsg(std::move(buffer));
        }
    }

private:
    // ----------------------------------------
    // private data types
    struct RemoteReceiver {
        IVAsioPeer* peer;
        uint16_t remoteIdx;
    };

private:
    // ----------------------------------------
    // private members
    std::vector<RemoteReceiver> _remoteReceivers;
};

} // mw
} // namespace ib
