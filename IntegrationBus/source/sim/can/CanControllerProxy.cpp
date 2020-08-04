// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "CanControllerProxy.hpp"

#include "ib/mw/IComAdapter.hpp"

namespace ib {
namespace sim {
namespace can {

CanControllerProxy::CanControllerProxy(mw::IComAdapter* comAdapter)
: _comAdapter(comAdapter)
{
}

void CanControllerProxy::SetBaudRate(uint32_t rate, uint32_t fdRate)
{
    _baudRate.baudRate = rate;
    _baudRate.fdBaudRate = fdRate;
    _comAdapter->SendIbMessage(_endpointAddr, _baudRate);
}

void CanControllerProxy::Reset()
{
    // prepare message to be sent
    CanSetControllerMode mode;
    mode.flags.cancelTransmitRequests = 1;
    mode.flags.resetErrorHandling = 1;
    mode.mode = CanControllerState::Uninit;

    _comAdapter->SendIbMessage(_endpointAddr, mode);
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

    _comAdapter->SendIbMessage(_endpointAddr, mode);
}

auto CanControllerProxy::SendMessage(const CanMessage& msg) -> CanTxId
{
    auto msgCopy = msg;
    msgCopy.transmitId = MakeTxId();

    //keep a copy until acknowledged by network simulator
    _transmittedMessages[msgCopy.transmitId] = msg;

    _comAdapter->SendIbMessage(_endpointAddr, msgCopy);
    return msgCopy.transmitId;
}

auto CanControllerProxy::SendMessage(CanMessage&& msg) -> CanTxId
{
    auto txId = MakeTxId();

    msg.transmitId = txId;

    //keep a copy until acknowledged by network simulator
    _transmittedMessages[msg.transmitId] = msg;

    _comAdapter->SendIbMessage(_endpointAddr, std::move(msg));

    return txId;
}

void CanControllerProxy::RegisterReceiveMessageHandler(ReceiveMessageHandler handler)
{
    RegisterHandler(handler);
}

void CanControllerProxy::RegisterStateChangedHandler(StateChangedHandler handler)
{
    RegisterHandler(handler);
}

void CanControllerProxy::RegisterErrorStateChangedHandler(ErrorStateChangedHandler handler)
{
    RegisterHandler(handler);
}

void CanControllerProxy::RegisterTransmitStatusHandler(MessageStatusHandler handler)
{
    RegisterHandler(handler);
}

template<typename MsgT>
void CanControllerProxy::RegisterHandler(CallbackT<MsgT> handler)
{
    auto&& handlers = std::get<CallbackVector<MsgT>>(_callbacks);
    handlers.push_back(handler);
}

void CanControllerProxy::ReceiveIbMessage(ib::mw::EndpointAddress from, const CanMessage& msg)
{
    if (from.participant == _endpointAddr.participant || from.endpoint != _endpointAddr.endpoint)
        return;

    _tracer.Trace(extensions::Direction::Receive, msg.timestamp, msg);

    CallHandlers(msg);
}

void CanControllerProxy::ReceiveIbMessage(ib::mw::EndpointAddress from, const CanTransmitAcknowledge& msg)
{
    if (from.participant == _endpointAddr.participant || from.endpoint != _endpointAddr.endpoint)
        return;

    auto transmittedMsg = _transmittedMessages.find(msg.transmitId);
    if (transmittedMsg != _transmittedMessages.end())
    {
        if (msg.status == CanTransmitStatus::Transmitted)
        {
            _tracer.Trace(extensions::Direction::Send, msg.timestamp,
                transmittedMsg->second);
        }

        _transmittedMessages.erase(msg.transmitId);
    }

    CallHandlers(msg);
}

void CanControllerProxy::ReceiveIbMessage(ib::mw::EndpointAddress from, const CanControllerStatus& msg)
{
    if (from.participant == _endpointAddr.participant || from.endpoint != _endpointAddr.endpoint)
        return;

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
    for (auto&& handler : handlers)
    {
        handler(this, msg);
    }
}

void CanControllerProxy::SetEndpointAddress(const ::ib::mw::EndpointAddress& endpointAddress)
{
    _endpointAddr = endpointAddress;
}

auto CanControllerProxy::EndpointAddress() const -> const ::ib::mw::EndpointAddress&
{
    return _endpointAddr;
}



} // namespace can
} // namespace sim
} // namespace ib
