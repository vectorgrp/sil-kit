// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/logging/ILogger.hpp"

#include "VAsioTransmitter.hpp"
#include "traits/IbMsgTraits.hpp"

#include "IIbMessageReceiver.hpp"

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
    IbLink(std::string name, logging::ILogger* logger);

public:
    // ----------------------------------------
    // Public methods
    static constexpr auto MsgTypeName() -> const char* { return IbMsgTraits<MsgT>::TypeName(); }
    inline auto Name() const -> const std::string& { return _name; }

    void AddLocalReceiver(ReceiverT* receiver);
    void AddRemoteReceiver(IVAsioPeer* peer, uint16_t remoteIdx);

    void DistributeRemoteIbMessage(const IIbServiceEndpoint* from, const MsgT& msg);
    void DistributeLocalIbMessage(const IIbServiceEndpoint* sender, const MsgT& msg);

    void DispatchIbMessageToTarget(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const MsgT& msg);

private:
    // ----------------------------------------
    // private methods
    void DispatchIbMessage(ReceiverT* to, const IIbServiceEndpoint* from, const MsgT& msg);

private:
    // ----------------------------------------
    // private members
    std::string _name;
    logging::ILogger* _logger;

    std::vector<ReceiverT*> _localReceivers;
    VAsioTransmitter<MsgT> _vasioTransmitter;
};

// ================================================================================
//  Inline Implementations
// ================================================================================
template <class MsgT>
IbLink<MsgT>::IbLink(std::string name, logging::ILogger* logger)
    : _name{std::move(name)}
    , _logger{logger}
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
void IbLink<MsgT>::DistributeRemoteIbMessage(const IIbServiceEndpoint* from, const MsgT& msg)
{
    for (auto&& receiver : _localReceivers)
    {
        DispatchIbMessage(receiver, from, msg);
    }
}

template <class MsgT>
void IbLink<MsgT>::DistributeLocalIbMessage(const IIbServiceEndpoint* from, const MsgT& msg)
{
    for (auto&& receiver : _localReceivers)
    {
        auto* receiverId = dynamic_cast<const IIbServiceEndpoint*>(receiver);
        // C++ 17 -> if constexpr
        if (!IbMsgTraits<MsgT>::IsSelfDeliveryEnforced())
        {
          if (receiverId == from) continue;
        }
        DispatchIbMessage(receiver, from, msg);
    }
    DispatchIbMessage(&_vasioTransmitter, from, msg);
}

template <class MsgT>
void IbLink<MsgT>::DispatchIbMessage(ReceiverT* to, const IIbServiceEndpoint* from, const MsgT& msg)
{
    try
    {
        to->ReceiveIbMessage(from, msg);
    }
    catch (const std::exception& e)
    {
        _logger->Warn("Callback for {}[\"{}\"] threw an exception: {}", MsgTypeName(), Name(), e.what());
    }
    catch (...)
    {
        _logger->Warn("Callback for {}[\"{}\"] threw an unknown exception", MsgTypeName(), Name());
    }
}

template <class MsgT>
void IbLink<MsgT>::DispatchIbMessageToTarget(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const MsgT& msg)
{
    _vasioTransmitter->SendMessageToTarget(from, targetParticipantName, msg);
}


} // namespace mw
} // namespace ib
