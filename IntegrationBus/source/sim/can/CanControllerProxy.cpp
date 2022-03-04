// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "CanControllerProxy.hpp"


namespace ib {
namespace sim {
namespace can {

CanControllerProxy::CanControllerProxy(mw::IComAdapterInternal* comAdapter, ICanController* facade)
: _comAdapter(comAdapter)
    , _facade{facade}
{
    if (_facade == nullptr)
    {
        _facade = this;
    }
}

void CanControllerProxy::SetBaudRate(uint32_t rate, uint32_t fdRate)
{
    _baudRate.baudRate = rate;
    _baudRate.fdBaudRate = fdRate;
    _comAdapter->SendIbMessage(this, _baudRate);
}

void CanControllerProxy::Reset()
{
    // prepare message to be sent
    CanSetControllerMode mode;
    mode.flags.cancelTransmitRequests = 1;
    mode.flags.resetErrorHandling = 1;
    mode.mode = CanControllerState::Uninit;

    _comAdapter->SendIbMessage(this, mode);
}

void CanControllerProxy::Start()
{
    ChangeControllerMode(CanControllerState::Started);
}

void CanControllerProxy::Stop()
{
    ChangeControllerMode(CanControllerState::Stopped);
}

void CanControllerProxy::Sleep()
{
    ChangeControllerMode(CanControllerState::Sleep);
}

void CanControllerProxy::ChangeControllerMode(CanControllerState state)
{
    // prepare message to be sent
    CanSetControllerMode mode;
    mode.flags.cancelTransmitRequests = 0;
    mode.flags.resetErrorHandling = 0;
    mode.mode = state;

    _comAdapter->SendIbMessage(this, mode);
}

auto CanControllerProxy::SendMessage(const CanMessage& msg, void* userContext) -> CanTxId
{
    auto msgCopy = msg;
    msgCopy.transmitId = MakeTxId();
    msgCopy.userContext = userContext;

    _comAdapter->SendIbMessage(this, msgCopy);
    return msgCopy.transmitId;
}

auto CanControllerProxy::SendMessage(CanMessage&& msg, void* userContext) -> CanTxId
{
    auto txId = MakeTxId();

    msg.transmitId = txId;
    msg.userContext = userContext;

    _comAdapter->SendIbMessage(this, std::move(msg));

    return txId;
}

void CanControllerProxy::RegisterReceiveMessageHandler(ReceiveMessageHandler handler, DirectionMask directionMask)
{
    std::function<bool(const CanMessage&)> filter = [directionMask](const CanMessage& msg) {
        return (((DirectionMask)msg.direction & (DirectionMask)directionMask)) != 0;
    };
    RegisterHandler(handler, std::move(filter));
}

void CanControllerProxy::RegisterStateChangedHandler(StateChangedHandler handler)
{
    RegisterHandler(handler);
}

void CanControllerProxy::RegisterErrorStateChangedHandler(ErrorStateChangedHandler handler)
{
    RegisterHandler(handler);
}

void CanControllerProxy::RegisterTransmitStatusHandler(MessageStatusHandler handler, CanTransmitStatusMask statusMask)
{
    std::function<bool(const CanTransmitAcknowledge&)> filter = [statusMask](const CanTransmitAcknowledge& ack) {
        return ((CanTransmitStatusMask)ack.status & (CanTransmitStatusMask)statusMask) != 0;
    };
    RegisterHandler(handler, filter);
}

template<typename MsgT>
void CanControllerProxy::RegisterHandler(CallbackT<MsgT> handler, std::function<bool(const MsgT& msg)> filter)
{
    auto&& handlers = std::get<CallbackVector<MsgT>>(_callbacks);
    auto handler_tuple = std::make_tuple(handler, filter);
    handlers.push_back(handler_tuple);
}

void CanControllerProxy::ReceiveIbMessage(const IIbServiceEndpoint* /*from*/, const CanMessage& msg)
{
    _tracer.Trace(ib::sim::TransmitDirection::RX, msg.timestamp, msg);
    CallHandlers(msg);
}

void CanControllerProxy::ReceiveIbMessage(const IIbServiceEndpoint* /*from*/, const CanTransmitAcknowledge& msg)
{
    CallHandlers(msg);
}

void CanControllerProxy::ReceiveIbMessage(const IIbServiceEndpoint* /*from*/, const CanControllerStatus& msg)
{
    if (_controllerState != msg.controllerState)
    {
        _controllerState = msg.controllerState;
        CallHandlers(msg.controllerState);
    }
    if (_errorState != msg.errorState)
    {
        _errorState = msg.errorState;
        CallHandlers(msg.errorState);
    }
}

template<typename MsgT>
void CanControllerProxy::CallHandlers(const MsgT& msg)
{
    auto&& handlers = std::get<CallbackVector<MsgT>>(_callbacks);
    for (auto&& handlerTuple : handlers)
    {
        auto filter = std::get<std::function<bool(const MsgT& msg)>>(handlerTuple);
        auto handler = std::get<CallbackT<MsgT>>(handlerTuple);
        if (!filter || filter(msg))
        {
            handler(_facade, msg);
        }
    }
}

} // namespace can
} // namespace sim
} // namespace ib
