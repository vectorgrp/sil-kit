// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "CanController.hpp"

#include <algorithm>
#include "ib/mw/IComAdapter.hpp"

namespace ib {
namespace sim {
namespace can {


CanController::CanController(mw::IComAdapter* comAdapter)
: _comAdapter(comAdapter)
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

    _pendingAcks.emplace_back(msgCopy.canId, msgCopy.transmitId);

    _comAdapter->SendIbMessage(_endpointAddr, msgCopy);
    return msgCopy.transmitId;
}

auto CanController::SendMessage(CanMessage&& msg) -> CanTxId
{
    auto txId = MakeTxId();
    msg.transmitId = txId;

    _pendingAcks.emplace_back(msg.canId, msg.transmitId);

    _comAdapter->SendIbMessage(_endpointAddr, std::move(msg));
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

void CanController::ReceiveIbMessage(ib::mw::EndpointAddress from, const CanMessage& msg)
{
    if (from == _endpointAddr)
        return;

    CallHandlers(msg);

    CanTransmitAcknowledge ack{msg.transmitId, msg.canId, msg.timestamp, CanTransmitStatus::Transmitted};
    _comAdapter->SendIbMessage(_endpointAddr, ack);
}

void CanController::ReceiveIbMessage(ib::mw::EndpointAddress from, const CanTransmitAcknowledge& msg)
{
    if (from == _endpointAddr)
        return;

    auto pendingAcksIter = std::find(_pendingAcks.begin(), _pendingAcks.end(), std::make_pair(msg.canId, msg.transmitId));
    if (pendingAcksIter != _pendingAcks.end())
    {
        _pendingAcks.erase(pendingAcksIter);
        CallHandlers(msg);
    }
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
    _endpointAddr = endpointAddress;
}

auto CanController::EndpointAddress() const -> const ::ib::mw::EndpointAddress&
{
    return _endpointAddr;
}

} // namespace can
} // namespace sim
} // namespace ib
