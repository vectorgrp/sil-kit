// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/IIbMessageReceiver.hpp"

#include "VAsioMsgKind.hpp"
#include "IVAsioPeer.hpp"

namespace ib {
namespace mw {

template <class MsgT>
class VAsioTransmitter : public IIbMessageReceiver<MsgT>
{
public:
    // ----------------------------------------
    // Public methods
    void AddRemoteReceiver(IVAsioPeer* peer, uint16_t remoteIdx);

public:
    // ----------------------------------------
    // Public interface methods
    void ReceiveIbMessage(EndpointAddress from, const MsgT& msg) override
    {
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

// ================================================================================
//  Inline Implementations
// ================================================================================
template <class MsgT>
void VAsioTransmitter<MsgT>::AddRemoteReceiver(IVAsioPeer* peer, uint16_t remoteIdx)
{
    _remoteReceivers.push_back({peer, remoteIdx});
}

} // mw
} // namespace ib
