// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/IIbMessageReceiver.hpp"
#include "VAsioTransmitter.hpp"
#include "IbMsgTraits.hpp"

namespace ib {
namespace mw {

template <class MsgT>
class IbLink
{
public:
    using ReceiverT = IIbMessageReceiver<MsgT>;
    
public:
    // ----------------------------------------
    // Constructors and Destructor
    IbLink(std::string name);

public:
    // ----------------------------------------
    // Public methods
    static constexpr auto MsgTypeName() -> const char* { return IbMsgTraits<MsgT>::TypeName(); }
    inline auto Name() const -> const std::string& { return _name; }

    void AddLocalReceiver(ReceiverT* receiver);
    void AddRemoteReceiver(IVAsioPeer* peer, uint16_t remoteIdx);

    void DistributeRemoteIbMessage(EndpointAddress from, const MsgT& msg);
    void DistributeLocalIbMessage(EndpointAddress from, const MsgT& msg);

private:
    // ----------------------------------------
    // private methods
    void DispatchIbMessage(ReceiverT* to, EndpointAddress from, const MsgT& msg);

private:
    // ----------------------------------------
    // private members
    std::string _name;

    std::vector<ReceiverT*> _localReceivers;
    VAsioTransmitter<MsgT> _vasioTransmitter;
};

// ================================================================================
//  Inline Implementations
// ================================================================================
template <class MsgT>
IbLink<MsgT>::IbLink(std::string name)
    : _name{std::move(name)}
{
}

template <class MsgT>
void IbLink<MsgT>::AddLocalReceiver(ReceiverT* receiver)
{
    if (std::find(_localReceivers.begin(), _localReceivers.end(), receiver) != _localReceivers.end()) return;
    _localReceivers.push_back(receiver);
}

template <class MsgT>
void IbLink<MsgT>::AddRemoteReceiver(IVAsioPeer* peer, uint16_t remoteIdx)
{
    _vasioTransmitter.AddRemoteReceiver(peer, remoteIdx);
}

template <class MsgT>
void IbLink<MsgT>::DistributeRemoteIbMessage(EndpointAddress from, const MsgT& msg)
{
    for (auto&& receiver : _localReceivers)
    {
        DispatchIbMessage(receiver, from, msg);
    }
}
    
template <class MsgT>
void IbLink<MsgT>::DistributeLocalIbMessage(EndpointAddress from, const MsgT& msg)
{
    for (auto&& receiver : _localReceivers)
    {
        DispatchIbMessage(receiver, from, msg);
    }
    DispatchIbMessage(&_vasioTransmitter, from, msg);
}

template <class MsgT>
void IbLink<MsgT>::DispatchIbMessage(ReceiverT* to, EndpointAddress from, const MsgT& msg)
{
    try
    {
        to->ReceiveIbMessage(from, msg);
    }
    catch (const std::exception& e)
    {
        std::cerr
            << "WARNING: Callback for "
            << MsgTypeName()
            << "[\"" << Name() << "\"]"
            << " threw an exception: " << e.what() << "." << std::endl;
    }
    catch (...)
    {
        std::cerr
            << "WARNING: Callback for "
            << MsgTypeName()
            << "[\"" << Name() << "\"]"
            << " threw an unknown exception." << std::endl;
    }
}
    
    

} // namespace mw
} // namespace ib
