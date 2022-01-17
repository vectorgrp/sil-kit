// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once


#include "VAsioMsgKind.hpp"
#include "IVAsioPeer.hpp"
#include <type_traits>

#include "IIbMessageReceiver.hpp"
#include "IIbServiceEndpoint.hpp"
#include "traits/IbMsgTraits.hpp"

namespace ib {
namespace mw {

//auxiliary class for conditional compilation using ib message traits
template<typename MsgT, std::size_t MsgHistSize>
struct MessageHistory {};
// MessageHistory<.., 0>: message history is disabled
template<typename MsgT> struct MessageHistory<MsgT, 0>
{
    void Save(const IIbServiceEndpoint*, const MsgT& ) {}
    void NotifyPeer(IVAsioPeer*, uint16_t) {}
};
// MessageHistory<.., 1>: save last message and notify peers about it
template<typename MsgT> struct MessageHistory<MsgT, 1>
{
    void Save(const IIbServiceEndpoint* from , const MsgT& msg)
    {
        _from = from->GetServiceDescriptor().legacyEpa;
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
class VAsioTransmitter 
    : public IIbMessageReceiver<MsgT>
    , public IIbServiceEndpoint
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


        _serviceDescriptor.participantName = peer->GetUri().participantName;
        _remoteReceivers.push_back(remoteReceiver);
        _hist.NotifyPeer(peer, remoteIdx);
    }

    void SendMessageToTarget(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const MsgT& msg)
    {
        // TODO  what do we do with the history for targeted messaging?
        _hist.Save(from, msg);
        auto&& receiverIter = std::find_if(_remoteReceivers.begin(), _remoteReceivers.end(), [targetParticipantName](auto&& receiver) 
            {
                return receiver->peer->GetUri().participantName == targetParticipantName;
            });
        if (receiverIter == _remoteReceivers.end())
        {
          std::cout << "Error: Tried to send targeted message to participant '" << targetParticipantName << "', which is not a valid remote receiver." << std::endl;
          assert(false);
        }
        ib::mw::MessageBuffer buffer;
        uint32_t msgSizePlaceholder{0u};
        buffer
            << msgSizePlaceholder
            << VAsioMsgKind::IbSimMsg
            << (*receiverIter)->remoteIdx
            << to_endpointAddress(from->GetServiceDescriptor())
            << msg;
        (*receiverIter).peer->SendIbMsg(std::move(buffer));
    }


public:
    // ----------------------------------------
    // Public interface methods
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const MsgT& msg) override
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
                << to_endpointAddress(from->GetServiceDescriptor())
                << msg;
            receiver.peer->SendIbMsg(std::move(buffer));
        }
    }

    // IIbServiceEndpoint
    void SetServiceDescriptor(const ServiceDescriptor& serviceDescriptor) override
    {
        _serviceDescriptor = serviceDescriptor;
    }
    auto GetServiceDescriptor() const -> const ServiceDescriptor& override
    {
        return _serviceDescriptor;
    }
private:
    // ----------------------------------------
    // private members
    std::vector<RemoteReceiver> _remoteReceivers;
    ServiceDescriptor _serviceDescriptor;
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
