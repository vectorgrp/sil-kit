// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "EthController.hpp"
#include "PcapSink.hpp"

#include "ib/mw/logging/ILogger.hpp"

#include <algorithm>

namespace ib {
namespace sim {
namespace eth {

EthController::EthController(mw::IComAdapterInternal* comAdapter, cfg::EthernetController config, mw::sync::ITimeProvider* timeProvider)
    : _comAdapter{comAdapter}
    , _timeProvider{timeProvider}
{
}

void EthController::Activate()
{
    // only supported when using a Network Simulator --> implement in EthControllerProxy
}

void EthController::Deactivate()
{
    // only supported when using a Network Simulator --> implement in EthControllerProxy
}

auto EthController::SendMessage(EthMessage msg) -> EthTxId
{
    auto txId = MakeTxId();
    _pendingAcks.emplace_back(msg.ethFrame.GetSourceMac(), txId);

    msg.transmitId = txId;

    _tracer.Trace(extensions::Direction::Send, msg.timestamp, msg.ethFrame);

    _comAdapter->SendIbMessage(this, std::move(msg));

    EthTransmitAcknowledge ack;
    ack.timestamp = msg.timestamp;
    ack.transmitId = msg.transmitId;
    ack.sourceMac = msg.ethFrame.GetSourceMac();
    ack.status = EthTransmitStatus::Transmitted;
    CallHandlers(ack);

    return txId;
}

auto EthController::SendFrame(EthFrame frame) -> EthTxId
{
    return SendFrame(std::move(frame), _timeProvider->Now());
}

auto EthController::SendFrame(EthFrame frame, std::chrono::nanoseconds timestamp) -> EthTxId
{
    EthMessage msg{};
    msg.timestamp = timestamp;
    msg.ethFrame = std::move(frame);
    return SendMessage(std::move(msg));
}

void EthController::RegisterReceiveMessageHandler(ReceiveMessageHandler handler)
{
    RegisterHandler(handler);
}

void EthController::RegisterMessageAckHandler(MessageAckHandler handler)
{
    RegisterHandler(handler);
}

void EthController::RegisterStateChangedHandler(StateChangedHandler /*handler*/)
{
    // only supported when using a Network Simulator --> implement in EthControllerProxy
}

void EthController::RegisterBitRateChangedHandler(BitRateChangedHandler /*handler*/)
{
    // only supported when using a Network Simulator --> implement in EthControllerProxy
}


void EthController::ReceiveIbMessage(const IIbServiceEndpoint* from, const EthMessage& msg)
{
    if (AllowMessageProcessing(from->GetServiceDescriptor(), _serviceDescriptor))
        return;

    _tracer.Trace(extensions::Direction::Receive, msg.timestamp, msg.ethFrame);

    CallHandlers(msg);
}

void EthController::SetEndpointAddress(const mw::EndpointAddress& endpointAddress)
{
    _serviceDescriptor.legacyEpa = endpointAddress;
}

auto EthController::EndpointAddress() const -> const mw::EndpointAddress&
{
    return _serviceDescriptor.legacyEpa;
}

template<typename MsgT>
void EthController::RegisterHandler(CallbackT<MsgT> handler)
{
    auto&& handlers = std::get<CallbackVector<MsgT>>(_callbacks);
    handlers.push_back(handler);
}


template<typename MsgT>
void EthController::CallHandlers(const MsgT& msg)
{
    auto&& handlers = std::get<CallbackVector<MsgT>>(_callbacks);
    for (auto&& handler : handlers)
    {
        handler(this, msg);
    }
}

void EthController::SetTimeProvider(ib::mw::sync::ITimeProvider* timeProvider)
{
    _timeProvider = timeProvider;
}
} // namespace eth
} // namespace sim
} // namespace ib
