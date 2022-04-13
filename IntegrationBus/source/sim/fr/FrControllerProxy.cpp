// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "FrControllerProxy.hpp"
#include "Validation.hpp"

#include "ib/mw/logging/ILogger.hpp"

namespace ib {
namespace sim {
namespace fr {

FrControllerProxy::FrControllerProxy(mw::IParticipantInternal* participant, cfg::FlexRayController config,
                                     IFrController* facade)
    : _participant(participant)
    , _facade{facade}
    , _config{config}
{
    if (_facade == nullptr)
    {
        _facade = this;
    }
}

void FrControllerProxy::Configure(const ControllerConfig& config)
{
    ControllerConfig cfg = config;
    if (!IsClusterParametersConfigurable())
    {
        cfg.clusterParams = _config.clusterParameters.value();
        WarnOverride("clusterParameters");
    }
    if (!IsNodeParametersConfigurable())
    {
        cfg.nodeParams = _config.nodeParameters.value();
        WarnOverride("NodeParamters");
    }
    if (!IsTxBufferConfigsConfigurable())
    {
        cfg.bufferConfigs = _config.txBufferConfigurations;
        WarnOverride("TxBufferConfigs");
    }


    Validate(cfg.clusterParams);
    Validate(cfg.nodeParams);

    _bufferConfigs = cfg.bufferConfigs;
    SendIbMessage(cfg);
}

void FrControllerProxy::ReconfigureTxBuffer(uint16_t txBufferIdx, const TxBufferConfig& config)
{
    if (txBufferIdx >= _bufferConfigs.size())
    {
        _participant->GetLogger()->Error("FrControllerProxy::ReconfigureTxBuffer() was called with unconfigured txBufferIdx={}", txBufferIdx);
        throw std::out_of_range{"Unconfigured txBufferIdx!"};
    }

    if (_config.txBufferConfigurations.size() > 0)
    {
        _participant->GetLogger()->Error("ReconfigureTxBuffer() was called on a preconfigured txBuffer. This is not "
                                        "allowed and the reconfiguration will be discarded.");
        return;
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
        _participant->GetLogger()->Error("FrControllerProxy::UpdateTxBuffer() was called with unconfigured txBufferIndex={}", update.txBufferIndex);
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

void FrControllerProxy::ReceiveIbMessage(const IIbServiceEndpoint* /*from*/, const FrMessage& msg)
{
    _tracer.Trace(ib::sim::TransmitDirection::RX, msg.timestamp, msg);

    CallHandlers(msg);
}

void FrControllerProxy::ReceiveIbMessage(const IIbServiceEndpoint* /*from*/, const FrMessageAck& msg)
{
    FrMessage tmp;
    tmp.frame = msg.frame;
    tmp.channel = msg.channel;
    tmp.timestamp = msg.timestamp;
    _tracer.Trace(ib::sim::TransmitDirection::TX, msg.timestamp, tmp);

    CallHandlers(msg);
}

void FrControllerProxy::ReceiveIbMessage(const IIbServiceEndpoint* /*from*/, const FrSymbol& msg)
{
    // Call wakeup handlers on WUS and WUDOP
    switch (msg.pattern)
    {
    case SymbolPattern::CasMts:
        break;
    case SymbolPattern::Wus:
    case SymbolPattern::Wudop:
        for (auto&& handler : _wakeupHandlers)
        {

            handler(_facade, msg);
        }
    }

    // In addition, call the generic SymbolHandlers for every symbol
    CallHandlers(msg);
}

void FrControllerProxy::ReceiveIbMessage(const IIbServiceEndpoint* /*from*/, const FrSymbolAck& msg)
{
    CallHandlers(msg);
}

void FrControllerProxy::ReceiveIbMessage(const IIbServiceEndpoint* /*from*/, const CycleStart& msg)
{
    CallHandlers(msg);
}

void FrControllerProxy::ReceiveIbMessage(const IIbServiceEndpoint* /*from*/, const PocStatus& msg)
{
    CallHandlers(msg);
}

void FrControllerProxy::WarnOverride(const std::string& parameterName)
{
    std::stringstream ss;
    ss << "Discarded user-defined configuration of " << parameterName
       << ", as it was already set in the predefined configuration.";

    _participant->GetLogger()->Warn(ss.str());
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
        handler(_facade, msg);
    }
}

template<typename MsgT>
void FrControllerProxy::SendIbMessage(MsgT&& msg)
{
    _participant->SendIbMessage(this, std::forward<MsgT>(msg));
}


bool FrControllerProxy::IsClusterParametersConfigurable()
{
    return !_config.clusterParameters.has_value();
}

bool FrControllerProxy::IsNodeParametersConfigurable()
{
    return !_config.nodeParameters.has_value();
}

bool FrControllerProxy::IsTxBufferConfigsConfigurable()
{
    return _config.txBufferConfigurations.size() == 0;
}


} // namespace fr
} // SimModels
} // namespace ib
