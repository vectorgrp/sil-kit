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
        _hasValue = true;
    }
    void NotifyPeer(IVAsioPeer* peer, uint16_t remoteIdx)
    {
        if (!_hasValue)
            return;
        
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
    bool _hasValue{false};
};


struct RemoteReceiver {
    IVAsioPeer* peer;
    uint16_t remoteIdx;
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
        RemoteReceiver remoteReceiver;
        remoteReceiver.peer = peer;
        remoteReceiver.remoteIdx = remoteIdx;

        if (_remoteReceivers.end() != std::find(_remoteReceivers.begin(), _remoteReceivers.end(), remoteReceiver))
            return;

        _remoteReceivers.push_back(remoteReceiver);
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
    // private members
    std::vector<RemoteReceiver> _remoteReceivers;
};

// ================================================================================
//  Inline Implementations
// ================================================================================
inline bool operator==(const RemoteReceiver& lhs, const RemoteReceiver& rhs)
{
    return lhs.peer == rhs.peer
        && lhs.remoteIdx == rhs.remoteIdx;
}


} // mw
} // namespace ib
