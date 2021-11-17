// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "FrControllerProxy.hpp"
#include "Validation.hpp"

#include "ib/mw/logging/ILogger.hpp"

namespace ib {
namespace sim {
namespace fr {

FrControllerProxy::FrControllerProxy(mw::IComAdapterInternal* comAdapter)
: _comAdapter(comAdapter)
, _endpointAddr{}
{
}

void FrControllerProxy::Configure(const ControllerConfig& config)
{
    Validate(config.clusterParams);
    Validate(config.nodeParams);

    _bufferConfigs = config.bufferConfigs;
    SendIbMessage(config);
}

void FrControllerProxy::ReconfigureTxBuffer(uint16_t txBufferIdx, const TxBufferConfig& config)
{
    if (txBufferIdx >= _bufferConfigs.size())
    {
        _comAdapter->GetLogger()->Error("FrControllerProxy::ReconfigureTxBuffer() was called with unconfigured txBufferIdx={}", txBufferIdx);
        throw std::out_of_range{"Unconfigured txBufferIdx!"};
    }

    TxBufferConfigUpdate update;
    update.txBufferIndex = txBufferIdx;
    update.txBufferConfig = config;
    SendIbMessage(update);
}

void FrControllerProxy::UpdateTxBuffer(const TxBufferUpdate& update)
{
    if (update.txBufferIndex >= _bufferConfigs.size())
    {
        _comAdapter->GetLogger()->Error("FrControllerProxy::UpdateTxBuffer() was called with unconfigured txBufferIndex={}", update.txBufferIndex);
        throw std::out_of_range{"Unconfigured txBufferIndex!"};
    }

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
    if (_comAdapter)
    {
        auto* logger = _comAdapter->GetLogger();
        if (logger)
        {
            logger->Warn("RegisterControllerStatusHandler is deprecated! use RegisterPocStatusHandler!");
        }

    }
    RegisterHandler(handler);
}

void FrControllerProxy::RegisterPocStatusHandler(PocStatusHandler handler)
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

void FrControllerProxy::RegisterCycleStartHandler(CycleStartHandler handler)
{
    RegisterHandler(handler);
}

void FrControllerProxy::ReceiveIbMessage(ib::mw::EndpointAddress from, const FrMessage& msg)
{
    if (from.participant == _endpointAddr.participant || from.endpoint != _endpointAddr.endpoint)
        return;

    _tracer.Trace(extensions::Direction::Receive, msg.timestamp, msg);

    CallHandlers(msg);
}

void FrControllerProxy::ReceiveIbMessage(ib::mw::EndpointAddress from, const FrMessageAck& msg)
{
    if (from.participant == _endpointAddr.participant || from.endpoint != _endpointAddr.endpoint)
        return;

    FrMessage tmp;
    tmp.frame = msg.frame;
    tmp.channel = msg.channel;
    tmp.timestamp = msg.timestamp;
    _tracer.Trace(extensions::Direction::Send, msg.timestamp, tmp);

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

void FrControllerProxy::ReceiveIbMessage(ib::mw::EndpointAddress from, const CycleStart& msg)
{
    if (from.participant == _endpointAddr.participant || from.endpoint != _endpointAddr.endpoint)
        return;

    CallHandlers(msg);
}

void FrControllerProxy::ReceiveIbMessage(ib::mw::EndpointAddress from, const PocStatus& msg)
{
    if (from.participant == _endpointAddr.participant || from.endpoint != _endpointAddr.endpoint)
        return;

    //interoperability with 3.0.3
    ControllerStatus status{};
    status.pocState = msg.state;
    CallHandlers(status);

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
