// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/logging/ILogger.hpp"

#include "VAsioTransmitter.hpp"
#include "traits/IbMsgTraits.hpp"

#include "IIbMessageReceiver.hpp"
#include "TimeSyncService.hpp"

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
    IbLink(std::string name, logging::ILogger* logger, sync::TimeSyncService* timeSyncService);

public:
    // ----------------------------------------
    // Public methods
    static constexpr auto MsgTypeName() -> const char* { return IbMsgTraits<MsgT>::TypeName(); }
    static constexpr auto MessageSerdesName() -> const char* { return IbMsgTraits<MsgT>::SerdesName(); }
    inline auto Name() const -> const std::string& { return _name; }

    void AddLocalReceiver(ReceiverT* receiver);
    void AddRemoteReceiver(IVAsioPeer* peer, EndpointId remoteIdx);

    void DistributeRemoteIbMessage(const IIbServiceEndpoint* from, MsgT&& msg);
    void DistributeLocalIbMessage(const IIbServiceEndpoint* from, const MsgT& msg);

    void SetHistoryLength(size_t history);

    void DispatchIbMessageToTarget(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const MsgT& msg);

    void SetTimeSyncService(sync::TimeSyncService* timeSyncService);

private:
    // ----------------------------------------
    // private methods
    void DispatchIbMessage(ReceiverT* to, const IIbServiceEndpoint* from, const MsgT& msg);

private:
    // ----------------------------------------
    // private members
    std::string _name;
    logging::ILogger* _logger;
    sync::TimeSyncService* _timeSyncService;

    std::vector<ReceiverT*> _localReceivers;
    VAsioTransmitter<MsgT> _vasioTransmitter;
};

// ================================================================================
//  Inline Implementations
// ================================================================================
template <class MsgT>
IbLink<MsgT>::IbLink(std::string name, logging::ILogger* logger, sync::TimeSyncService* timeSyncService)
    : _name{std::move(name)}
    , _logger{logger}
    , _timeSyncService{timeSyncService}
{
}

template <class MsgT>
void IbLink<MsgT>::AddLocalReceiver(ReceiverT* receiver)
{
    if (std::find(_localReceivers.begin(), _localReceivers.end(), receiver) != _localReceivers.end()) return;
    _localReceivers.push_back(receiver);
}

template <class MsgT>
void IbLink<MsgT>::AddRemoteReceiver(IVAsioPeer* peer, EndpointId remoteIdx)
{
    _vasioTransmitter.AddRemoteReceiver(peer, remoteIdx);
}


// ==================================================================
//  Function template using the trait to select the implementation
// ==================================================================

template <typename MsgT>
void SetTimestamp(MsgT& msg, std::chrono::nanoseconds value, std::enable_if_t<HasTimestamp<MsgT>::value, bool> = true)
{
    if (msg.timestamp == std::chrono::nanoseconds::duration::min())
    {
        msg.timestamp = value;
    }
}

template <typename MsgT>
void SetTimestamp(MsgT& /*msg*/, std::chrono::nanoseconds /*value*/, std::enable_if_t<!HasTimestamp<MsgT>::value, bool> = false)
{
}

// Distribute incoming (= from remote) IbMessages to local receivers
template <class MsgT>
void IbLink<MsgT>::DistributeRemoteIbMessage(const IIbServiceEndpoint* from, MsgT&& msg)
{
    if (_timeSyncService && _timeSyncService->IsSynchronized())
    {
        SetTimestamp(msg, _timeSyncService->Now());
    }

    for (auto&& receiver : _localReceivers)
    {
        DispatchIbMessage(receiver, from, msg);
    }
}

// Distribute outgoing (= from local) IbMessages to local (via _localReceivers) and remote (via transmitter per MsgT) receivers
template <class MsgT>
void IbLink<MsgT>::DistributeLocalIbMessage(const IIbServiceEndpoint* from, const MsgT& msg)
{
    // NB: Messages must be dispatched to remote receivers first.
    // Otherwise, messages that may be produced during the internal dispatch will be dispatched to remote receivers first.
    // As a result, the messages may be delivered in the wrong order (possibly even reversed)
    DispatchIbMessage(&_vasioTransmitter, from, msg);
    for (auto&& receiver : _localReceivers)
    {
        auto* receiverId = dynamic_cast<const IIbServiceEndpoint*>(receiver);
        // C++ 17 -> if constexpr
        if (!IbMsgTraits<MsgT>::IsSelfDeliveryEnforced())
        {
            if (receiverId->GetServiceDescriptor() == from->GetServiceDescriptor()) continue;
        }
        DispatchIbMessage(receiver, from, msg);
    }
}

// Dispatcher for outgoing IbMessages
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
    _vasioTransmitter.SendMessageToTarget(from, targetParticipantName, msg);
}

template <class MsgT>
void IbLink<MsgT>::SetTimeSyncService(sync::TimeSyncService* timeSyncService)
{
    _timeSyncService = timeSyncService;
}

template <class MsgT>
void IbLink<MsgT>::SetHistoryLength(size_t history)
{
    _vasioTransmitter.SetHistoryLength(history);
}

} // namespace mw
} // namespace ib
