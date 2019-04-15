// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "FrControllerProxy.hpp"

#include "ib/mw/IComAdapter.hpp"

namespace ib {
namespace sim {
namespace fr {

FrControllerProxy::FrControllerProxy(mw::IComAdapter* comAdapter)
: _comAdapter(comAdapter)
{
}

void FrControllerProxy::Configure(const ControllerConfig& config)
{
    SendIbMessage(config);
}

void FrControllerProxy::UpdateTxBuffer(const TxBufferUpdate& update)
{
    SendIbMessage(update);
}

void FrControllerProxy::Run()
{
    HostCommand cmd;
    cmd.command = ChiCommand::RUN;
    SendIbMessage(cmd);
}

void FrControllerProxy::DeferredHalt()
{
    HostCommand cmd;
    cmd.command = ChiCommand::DEFERRED_HALT;
    SendIbMessage(cmd);
}

void FrControllerProxy::Freeze()
{
    HostCommand cmd;
    cmd.command = ChiCommand::FREEZE;
    SendIbMessage(cmd);
}

void FrControllerProxy::AllowColdstart()
{
    HostCommand cmd;
    cmd.command = ChiCommand::ALLOW_COLDSTART;
    SendIbMessage(cmd);
}

void FrControllerProxy::AllSlots()
{
    HostCommand cmd;
    cmd.command = ChiCommand::ALL_SLOTS;
    SendIbMessage(cmd);
}

void FrControllerProxy::Wakeup()
{
    HostCommand cmd;
    cmd.command = ChiCommand::WAKEUP;
    SendIbMessage(cmd);
}

void FrControllerProxy::RegisterMessageHandler(MessageHandler handler)
{
    RegisterHandler(handler);
}

void FrControllerProxy::RegisterMessageAckHandler(MessageAckHandler handler)
{
    RegisterHandler(handler);
}

void FrControllerProxy::RegisterWakeupHandler(WakeupHandler handler)
{
    _wakeupHandlers.emplace_back(std::move(handler));
}

void FrControllerProxy::RegisterControllerStatusHandler(ControllerStatusHandler handler)
{
    RegisterHandler(handler);
}

void FrControllerProxy::RegisterSymbolHandler(SymbolHandler handler)
{
    RegisterHandler(handler);
}

void FrControllerProxy::RegisterSymbolAckHandler(SymbolAckHandler handler)
{
    RegisterHandler(handler);
}

void FrControllerProxy::ReceiveIbMessage(ib::mw::EndpointAddress from, const FrMessage& msg)
{
    if (from.participant == _endpointAddr.participant || from.endpoint != _endpointAddr.endpoint)
        return;

    CallHandlers(msg);
}

void FrControllerProxy::ReceiveIbMessage(ib::mw::EndpointAddress from, const FrMessageAck& msg)
{
    if (from.participant == _endpointAddr.participant || from.endpoint != _endpointAddr.endpoint)
        return;

    CallHandlers(msg);
}

void FrControllerProxy::ReceiveIbMessage(ib::mw::EndpointAddress from, const FrSymbol& msg)
{
    if (from.participant == _endpointAddr.participant || from.endpoint != _endpointAddr.endpoint)
        return;

    // Call wakeup handlers on WUS and WUDOP
    switch (msg.pattern)
    {
    case SymbolPattern::CasMts:
        break;
    case SymbolPattern::Wus:
    case SymbolPattern::Wudop:
        for (auto&& handler : _wakeupHandlers)
        {
            handler(this, msg);
        }
    }

    // In addition, call the generic SymbolHandlers for every symbol
    CallHandlers(msg);
}

void FrControllerProxy::ReceiveIbMessage(ib::mw::EndpointAddress from, const FrSymbolAck& msg)
{
    if (from.participant == _endpointAddr.participant || from.endpoint != _endpointAddr.endpoint)
        return;

    CallHandlers(msg);
}

void FrControllerProxy::ReceiveIbMessage(ib::mw::EndpointAddress from, const ControllerStatus& msg)
{
    if (from.participant == _endpointAddr.participant || from.endpoint != _endpointAddr.endpoint)
        return;

    CallHandlers(msg);
}

void FrControllerProxy::SetEndpointAddress(const ib::mw::EndpointAddress& endpointAddress)
{
    _endpointAddr = endpointAddress;
}

auto FrControllerProxy::EndpointAddress() const -> const ib::mw::EndpointAddress&
{
    return _endpointAddr;
}


template<typename MsgT>
void FrControllerProxy::RegisterHandler(CallbackT<MsgT> handler)
{
    auto&& handlers = std::get<CallbackVector<MsgT>>(_callbacks);
    handlers.push_back(handler);
}

template<typename MsgT>
void FrControllerProxy::CallHandlers(const MsgT& msg)
{
    auto&& handlers = std::get<CallbackVector<MsgT>>(_callbacks);
    for (auto&& handler : handlers)
    {
        handler(this, msg);
    }
}

template<typename MsgT>
void FrControllerProxy::SendIbMessage(MsgT&& msg)
{
    _comAdapter->SendIbMessage(_endpointAddr, std::forward<MsgT>(msg));
}



} // namespace fr
} // SimModels
} // namespace ib
