// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "CanController.hpp"

#include <algorithm>

namespace ib {
namespace sim {
namespace can {


CanController::CanController(mw::IComAdapterInternal* comAdapter, mw::sync::ITimeProvider* timeProvider)
    : _comAdapter{comAdapter}
    , _timeProvider{timeProvider}
{
}

void CanController::SetBaudRate(uint32_t /*rate*/, uint32_t /*fdRate*/)
{
}

void CanController::Reset()
{
}

void CanController::Start()
{
}

void CanController::Stop()
{
}

void CanController::Sleep()
{
}

auto CanController::SendMessage(const CanMessage& msg) -> CanTxId
{
    auto msgCopy = msg;
    msgCopy.transmitId = MakeTxId();

    _tracer.Trace(extensions::Direction::Send, _timeProvider->Now(), msg);


    _comAdapter->SendIbMessage(this, msgCopy);

    // instantly call transmit acknowledge
    CanTransmitAcknowledge ack;
    ack.canId = msg.canId;
    ack.status = CanTransmitStatus::Transmitted;
    ack.transmitId = msgCopy.transmitId;
    ack.timestamp = msg.timestamp;
    CallHandlers(ack);

    return msgCopy.transmitId;
}

auto CanController::SendMessage(CanMessage&& msg) -> CanTxId
{
    auto txId = MakeTxId();
    msg.transmitId = txId;

    _tracer.Trace(extensions::Direction::Send, _timeProvider->Now(), msg);

    _comAdapter->SendIbMessage(this, std::move(msg));
    
    // instantly call transmit acknowledge
    CanTransmitAcknowledge ack;
    ack.canId = msg.canId;
    ack.status = CanTransmitStatus::Transmitted;
    ack.transmitId = msg.transmitId;
    ack.timestamp = msg.timestamp;
    CallHandlers(ack);

    return txId;
}

void CanController::RegisterReceiveMessageHandler(ReceiveMessageHandler handler)
{
    RegisterHandler(handler);
}

void CanController::RegisterStateChangedHandler(StateChangedHandler /*handler*/)
{
}

void CanController::RegisterErrorStateChangedHandler(ErrorStateChangedHandler /*handler*/)
{
}

void CanController::RegisterTransmitStatusHandler(MessageStatusHandler handler)
{
    RegisterHandler(handler);
}

template<typename MsgT>
void CanController::RegisterHandler(CallbackT<MsgT> handler)
{
    auto&& handlers = std::get<CallbackVector<MsgT>>(_callbacks);
    handlers.push_back(handler);
}

void CanController::ReceiveIbMessage(const IIbServiceEndpoint* from, const CanMessage& msg)
{
    if (AllowMessageProcessing(from->GetServiceDescriptor(), _serviceDescriptor))
    {
        return;
    }
    CallHandlers(msg);

    _tracer.Trace(extensions::Direction::Receive, _timeProvider->Now(), msg);
}

template<typename MsgT>
void CanController::CallHandlers(const MsgT& msg)
{
    auto&& handlers = std::get<CallbackVector<MsgT>>(_callbacks);
    for (auto&& handler : handlers)
    {
        handler(this, msg);
    }
}

void CanController::SetEndpointAddress(const ::ib::mw::EndpointAddress& endpointAddress)
{
    _serviceDescriptor.legacyEpa = endpointAddress;
}

auto CanController::EndpointAddress() const -> const ::ib::mw::EndpointAddress&
{
    return _serviceDescriptor.legacyEpa;
}

void CanController::SetTimeProvider(ib::mw::sync::ITimeProvider* timeProvider)
{
    _timeProvider = timeProvider;
}

} // namespace can
} // namespace sim
} // namespace ib
