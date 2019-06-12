// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "VAsioMsgKind.hpp"

namespace ib {
namespace mw {

template <class MsgT>
struct VAsioSender {
    void SendIbMessage(EndpointAddress from, const MsgT& msg)
    {
        for (auto&& remote : _remoteReceivers)
        {
            ib::mw::MessageBuffer buffer;
            uint32_t msgSizePlaceholder{0u};
            buffer
                << msgSizePlaceholder
                << VAsioMsgKind::IbSimMsg << remote.receiverIdx << from << msg;
            remote.peer->SendIbMsg(std::move(buffer));
        }
    }
    void AddSubscriber(uint16_t receiverIdx, IVAsioPeer* peer)
    {
        _remoteReceivers.emplace_back(RemoteReceiver{receiverIdx, peer});
    }
    
private:
    struct RemoteReceiver {
        uint16_t receiverIdx;
        IVAsioPeer* peer;
    };
    std::vector<RemoteReceiver> _remoteReceivers;
};

} // mw
} // namespace ib
