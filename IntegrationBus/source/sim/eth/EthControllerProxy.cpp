// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "EthControllerProxy.hpp"


namespace ib {
namespace sim {
namespace eth {

EthControllerProxy::EthControllerProxy(mw::IComAdapterInternal* comAdapter, cfg::EthernetController config)
    : _comAdapter(comAdapter)
{

}

void EthControllerProxy::Activate()
{
    // Check if the Controller has already been activated
    if (_ethState != EthState::Inactive)
        return;

    EthSetMode msg { EthMode::Active };
    _comAdapter->SendIbMessage(this, msg);
}

void EthControllerProxy::Deactivate()
{
    // Check if the Controller has already been deactivated
    if (_ethState == EthState::Inactive)
        return;

    EthSetMode msg{ EthMode::Inactive };
    _comAdapter->SendIbMessage(this, msg);
}

auto EthControllerProxy::SendMessage(EthMessage msg) -> EthTxId
{
    auto txId = MakeTxId();
    msg.transmitId = txId;

    // we keep a copy until the transmission was acknowledged before tracing the message
    _transmittedMessages[msg.transmitId] = msg.ethFrame;

    _comAdapter->SendIbMessage(this, std::move(msg));


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


void EthControllerProxy::ReceiveIbMessage(const IIbServiceEndpoint* from, const EthMessage& msg)
{
    _tracer.Trace(ib::sim::TransmitDirection::RX,
        msg.timestamp, msg.ethFrame);

    CallHandlers(msg);
}

void EthControllerProxy::ReceiveIbMessage(const IIbServiceEndpoint* from, const EthTransmitAcknowledge& msg)
{
    auto transmittedMsg = _transmittedMessages.find(msg.transmitId);
    if (transmittedMsg != _transmittedMessages.end())
    {
        if (msg.status == EthTransmitStatus::Transmitted)
        {
            _tracer.Trace(ib::sim::TransmitDirection::TX, msg.timestamp,
                transmittedMsg->second);
        }

        _transmittedMessages.erase(msg.transmitId);
    }

    CallHandlers(msg);
}

void EthControllerProxy::ReceiveIbMessage(const IIbServiceEndpoint* from, const EthStatus& msg)
{
    // In case we are in early startup, ensure we tell our participants the bit rate first
    // and the state later.
    if (msg.bitRate != _ethBitRate)
    {
        _ethBitRate = msg.bitRate;
        CallHandlers(msg.bitRate);
    }

    if (msg.state != _ethState)
    {
        _ethState = msg.state;
        CallHandlers(msg.state);
    }

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
