// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "EthControllerProxy.hpp"

#include "ib/mw/IComAdapter.hpp"

namespace ib {
namespace sim {
namespace eth {

EthControllerProxy::EthControllerProxy(mw::IComAdapter* comAdapter, cfg::EthernetController config)
    : _comAdapter(comAdapter)
{

}

void EthControllerProxy::Activate()
{
    // Check if the Controller has already been activated
    if (_ethState != EthState::Inactive)
        return;

    EthSetMode msg { EthMode::Active };
    _comAdapter->SendIbMessage(_endpointAddr, msg);
}

void EthControllerProxy::Deactivate()
{
    // Check if the Controller has already been deactivated
    if (_ethState == EthState::Inactive)
        return;

    EthSetMode msg{ EthMode::Inactive };
    _comAdapter->SendIbMessage(_endpointAddr, msg);
}

auto EthControllerProxy::SendMessage(EthMessage msg) -> EthTxId
{
    auto txId = MakeTxId();
    msg.transmitId = txId;

    _tracer.Trace(tracing::Direction::Send,
        msg.timestamp, msg.ethFrame);

    _comAdapter->SendIbMessage(_endpointAddr, std::move(msg));


    return txId;
}

auto EthControllerProxy::SendFrame(EthFrame frame) -> EthTxId
{
    EthMessage msg;
    msg.ethFrame = std::move(frame);
    return SendMessage(std::move(msg));
}

auto EthControllerProxy::SendFrame(EthFrame frame, std::chrono::nanoseconds /*time stamp provided by VIBE netsim*/) -> EthTxId
{
    return SendFrame(std::move(frame));
}


void EthControllerProxy::RegisterReceiveMessageHandler(ReceiveMessageHandler handler)
{
    RegisterHandler(std::move(handler));
}

void EthControllerProxy::RegisterMessageAckHandler(MessageAckHandler handler)
{
    RegisterHandler(std::move(handler));
}

void EthControllerProxy::RegisterStateChangedHandler(StateChangedHandler handler)
{
    RegisterHandler(std::move(handler));
}

void EthControllerProxy::RegisterBitRateChangedHandler(BitRateChangedHandler handler)
{
    RegisterHandler(std::move(handler));
}


void EthControllerProxy::ReceiveIbMessage(mw::EndpointAddress from, const EthMessage& msg)
{
    if (from.participant == _endpointAddr.participant || from.endpoint != _endpointAddr.endpoint)
        return;

    _tracer.Trace(tracing::Direction::Receive,
        msg.timestamp, msg.ethFrame);

    CallHandlers(msg);
}

void EthControllerProxy::ReceiveIbMessage(mw::EndpointAddress from, const EthTransmitAcknowledge& msg)
{
    if (from.participant == _endpointAddr.participant || from.endpoint != _endpointAddr.endpoint)
        return;

    CallHandlers(msg);
}

void EthControllerProxy::ReceiveIbMessage(mw::EndpointAddress from, const EthStatus& msg)
{
    if (from.participant == _endpointAddr.participant || from.endpoint != _endpointAddr.endpoint)
        return;

    if (msg.state != _ethState)
    {
        _ethState = msg.state;
        CallHandlers(msg.state);
    }

    if (msg.bitRate != _ethBitRate)
    {
        _ethBitRate = msg.bitRate;
        CallHandlers(msg.bitRate);
    }
}

void EthControllerProxy::SetEndpointAddress(const mw::EndpointAddress& endpointAddress)
{
    _endpointAddr = endpointAddress;
}

auto EthControllerProxy::EndpointAddress() const -> const mw::EndpointAddress&
{
    return _endpointAddr;
}

template<typename MsgT>
void EthControllerProxy::RegisterHandler(CallbackT<MsgT>&& handler)
{
    auto&& handlers = std::get<CallbackVector<MsgT>>(_callbacks);
    handlers.push_back(std::forward<CallbackT<MsgT>>(handler));
}


template<typename MsgT>
void EthControllerProxy::CallHandlers(const MsgT& msg)
{
    auto&& handlers = std::get<CallbackVector<MsgT>>(_callbacks);
    for (auto&& handler : handlers)
    {
        handler(this, msg);
    }
}


} // namespace eth
} // namespace sim
} // namespace ib
