// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "LoggerMessage.hpp"

#include "VAsioTransmitter.hpp"
#include "traits/SilKitMsgTraits.hpp"
#include "MessageTracing.hpp"

#include "IMessageReceiver.hpp"
#include "TimeSyncService.hpp"

namespace SilKit {
namespace Core {

template <class MsgT>
class SilKitLink
{
public:
    using ReceiverT = IMessageReceiver<MsgT>;

public:
    // ----------------------------------------
    // Constructors and Destructor
    SilKitLink(std::string name, Services::Logging::ILoggerInternal* logger,
               Services::Orchestration::ITimeProvider* timeProvider);


public:
    // ----------------------------------------
    // Public methods
    static constexpr auto MsgTypeName() -> const char*
    {
        return SilKitMsgTraits<MsgT>::TypeName();
    }
    static constexpr auto MessageSerdesName() -> const char*
    {
        return SilKitMsgTraits<MsgT>::SerdesName();
    }
    inline auto Name() const -> const std::string&
    {
        return _name;
    }

    void AddLocalReceiver(ReceiverT* receiver);
    void AddRemoteReceiver(IVAsioPeer* peer, EndpointId remoteIdx);
    void RemoveRemoteReceiver(IVAsioPeer* peer);
    size_t GetNumberOfRemoteReceivers();
    std::vector<std::string> GetParticipantNamesOfRemoteReceivers();

    void DistributeRemoteSilKitMessage(const IServiceEndpoint* from, MsgT&& msg);
    void DistributeLocalSilKitMessage(const IServiceEndpoint* from, const MsgT& msg);

    void SetHistoryLength(size_t history);

    void DispatchSilKitMessageToTarget(const IServiceEndpoint* from, const std::string& targetParticipantName,
                                       const MsgT& msg);

private:
    // ----------------------------------------
    // private methods
    void DispatchSilKitMessage(ReceiverT* to, const IServiceEndpoint* from, const MsgT& msg);
    void DistributeToSelf(const IServiceEndpoint* from, const MsgT& msg);

private:
    // ----------------------------------------
    // private members
    std::string _name;
    Services::Logging::ILoggerInternal* _logger;
    Services::Orchestration::ITimeProvider* _timeProvider;

    std::vector<ReceiverT*> _localReceivers;
    VAsioTransmitter<MsgT> _vasioTransmitter;
};

// ================================================================================
//  Inline Implementations
// ================================================================================
template <class MsgT>
SilKitLink<MsgT>::SilKitLink(std::string name, Services::Logging::ILoggerInternal* logger,
                             Services::Orchestration::ITimeProvider* timeProvider)

    : _name{std::move(name)}
    , _logger{logger}
    , _timeProvider{timeProvider}
{
}

template <class MsgT>
void SilKitLink<MsgT>::AddLocalReceiver(ReceiverT* receiver)
{
    if (std::find(_localReceivers.begin(), _localReceivers.end(), receiver) != _localReceivers.end())
        return;
    _localReceivers.push_back(receiver);
}

template <class MsgT>
void SilKitLink<MsgT>::AddRemoteReceiver(IVAsioPeer* peer, EndpointId remoteIdx)
{
    _vasioTransmitter.AddRemoteReceiver(peer, remoteIdx);
}
template <class MsgT>
void SilKitLink<MsgT>::RemoveRemoteReceiver(IVAsioPeer* peer)
{
    _vasioTransmitter.RemoveRemoteReceiver(peer);
}
template <class MsgT>
auto SilKitLink<MsgT>::GetNumberOfRemoteReceivers() -> size_t
{
    return _vasioTransmitter.GetNumberOfRemoteReceivers();
}

template <class MsgT>
auto SilKitLink<MsgT>::GetParticipantNamesOfRemoteReceivers() -> std::vector<std::string>
{
    return _vasioTransmitter.GetParticipantNamesOfRemoteReceivers();
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
void SetTimestamp(MsgT& /*msg*/, std::chrono::nanoseconds /*value*/,
                  std::enable_if_t<!HasTimestamp<MsgT>::value, bool> = false)
{
}

// Distribute incoming (= from remote) SilKitMessages to local receivers
template <class MsgT>
void SilKitLink<MsgT>::DistributeRemoteSilKitMessage(const IServiceEndpoint* from, MsgT&& msg)
{
    if (_timeProvider->IsSynchronizingVirtualTime())
    {
        SetTimestamp(msg, _timeProvider->Now());
    }

    for (auto&& receiver : _localReceivers)
    {
        DispatchSilKitMessage(receiver, from, msg);
    }
}

// Distribute outgoing (= from local) SilKitMessages to local (via _localReceivers) and remote (via transmitter per MsgT) receivers
template <class MsgT>
void SilKitLink<MsgT>::DistributeLocalSilKitMessage(const IServiceEndpoint* from, const MsgT& msg)
{
    // NB: Messages must be dispatched to remote receivers first.
    // Otherwise, messages that may be produced during the internal dispatch will be dispatched to remote receivers first.
    // As a result, the messages may be delivered in the wrong order (possibly even reversed)
    DispatchSilKitMessage(&_vasioTransmitter, from, msg);
    DistributeToSelf(from, msg);
}

template <class MsgT>
void SilKitLink<MsgT>::DistributeToSelf(const IServiceEndpoint* from, const MsgT& msg)
{
    if constexpr (SilKitMsgTraits<MsgT>::IsSelfDeliveryForbidden())
    {
        return;
    }
    else
    {
        for (auto&& receiver : _localReceivers)
        {
            auto* receiverId = dynamic_cast<const IServiceEndpoint*>(receiver);
            if constexpr (!SilKitMsgTraits<MsgT>::IsSelfDeliveryEnforced())
            {
                if (receiverId->GetServiceDescriptor() == from->GetServiceDescriptor())
                    continue;
            }
            // Trace reception of self delivery
            Services::TraceRx(_logger, receiverId, msg, from->GetServiceDescriptor());

            DispatchSilKitMessage(receiver, from, msg);
        }
    }
}

// Dispatcher for outgoing SilKitMessages
template <class MsgT>
void SilKitLink<MsgT>::DispatchSilKitMessage(ReceiverT* to, const IServiceEndpoint* from, const MsgT& msg)
{
    try
    {
        to->ReceiveMsg(from, msg);
    }
    catch (const std::exception& e)
    {
        Services::Logging::Warn(_logger, "Callback for {}[\"{}\"] threw an exception: {}", MsgTypeName(), Name(),
                                e.what());
    }
    catch (...)
    {
        Services::Logging::Warn(_logger, "Callback for {}[\"{}\"] threw an unknown exception", MsgTypeName(), Name());
    }
}

template <class MsgT>
void SilKitLink<MsgT>::DispatchSilKitMessageToTarget(const IServiceEndpoint* from,
                                                     const std::string& targetParticipantName, const MsgT& msg)
{
    if (from->GetServiceDescriptor().GetParticipantName() == targetParticipantName)
    {
        DistributeToSelf(from, msg);
    }
    else
    {
        _vasioTransmitter.SendMessageToTarget(from, targetParticipantName, msg);
    }
}

template <class MsgT>
void SilKitLink<MsgT>::SetHistoryLength(size_t history)
{
    _vasioTransmitter.SetHistoryLength(history);
}

} // namespace Core
} // namespace SilKit
