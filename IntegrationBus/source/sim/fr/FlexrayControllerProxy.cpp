// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "FlexrayControllerProxy.hpp"
#include "Validation.hpp"

#include "ib/mw/logging/ILogger.hpp"

namespace ib {
namespace sim {
namespace fr {

FlexrayControllerProxy::FlexrayControllerProxy(mw::IParticipantInternal* participant, cfg::FlexrayController config,
                                               IFlexrayController* facade)
    : _participant(participant)
    , _facade{facade}
    , _config{std::move(config)}
{
    if (_facade == nullptr)
    {
        _facade = this;
    }
}

void FlexrayControllerProxy::Configure(const FlexrayControllerConfig& config)
{
    FlexrayControllerConfig cfg = config;
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

void FlexrayControllerProxy::ReconfigureTxBuffer(uint16_t txBufferIdx, const FlexrayTxBufferConfig& config)
{
    if (txBufferIdx >= _bufferConfigs.size())
    {
        _participant->GetLogger()->Error("FlexrayControllerProxy::ReconfigureTxBuffer() was called with unconfigured txBufferIdx={}", txBufferIdx);
        throw std::out_of_range{"Unconfigured txBufferIdx!"};
    }

    if (!IsTxBufferConfigsConfigurable())
    {
        _participant->GetLogger()->Error("ReconfigureTxBuffer() was called on a preconfigured txBuffer. This is not "
                                        "allowed and the reconfiguration will be discarded.");
        return;
    }

    FlexrayTxBufferConfigUpdate update;
    update.txBufferIndex = txBufferIdx;
    update.txBufferConfig = config;
    SendIbMessage(update);
}

void FlexrayControllerProxy::UpdateTxBuffer(const FlexrayTxBufferUpdate& update)
{
    if (update.txBufferIndex >= _bufferConfigs.size())
    {
        _participant->GetLogger()->Error("FlexrayControllerProxy::UpdateTxBuffer() was called with unconfigured txBufferIndex={}", update.txBufferIndex);
        throw std::out_of_range{"Unconfigured txBufferIndex!"};
    }

    SendIbMessage(update);
}

void FlexrayControllerProxy::Run()
{
    FlexrayHostCommand cmd;
    cmd.command = FlexrayChiCommand::RUN;
    SendIbMessage(cmd);
}

void FlexrayControllerProxy::DeferredHalt()
{
    FlexrayHostCommand cmd;
    cmd.command = FlexrayChiCommand::DEFERRED_HALT;
    SendIbMessage(cmd);
}

void FlexrayControllerProxy::Freeze()
{
    FlexrayHostCommand cmd;
    cmd.command = FlexrayChiCommand::FREEZE;
    SendIbMessage(cmd);
}

void FlexrayControllerProxy::AllowColdstart()
{
    FlexrayHostCommand cmd;
    cmd.command = FlexrayChiCommand::ALLOW_COLDSTART;
    SendIbMessage(cmd);
}

void FlexrayControllerProxy::AllSlots()
{
    FlexrayHostCommand cmd;
    cmd.command = FlexrayChiCommand::ALL_SLOTS;
    SendIbMessage(cmd);
}

void FlexrayControllerProxy::Wakeup()
{
    FlexrayHostCommand cmd;
    cmd.command = FlexrayChiCommand::WAKEUP;
    SendIbMessage(cmd);
}

void FlexrayControllerProxy::AddFrameHandler(FrameHandler handler)
{
    RegisterHandler(handler);
}

void FlexrayControllerProxy::AddFrameTransmitHandler(FrameTransmitHandler handler)
{
    RegisterHandler(handler);
}

void FlexrayControllerProxy::AddWakeupHandler(WakeupHandler handler)
{
    RegisterHandler(handler);
}

void FlexrayControllerProxy::AddPocStatusHandler(PocStatusHandler handler)
{
    RegisterHandler(handler);
}

void FlexrayControllerProxy::AddSymbolHandler(SymbolHandler handler)
{
    RegisterHandler(handler);
}

void FlexrayControllerProxy::AddSymbolTransmitHandler(SymbolTransmitHandler handler)
{
    RegisterHandler(handler);
}

void FlexrayControllerProxy::AddCycleStartHandler(CycleStartHandler handler)
{
    RegisterHandler(handler);
}

void FlexrayControllerProxy::ReceiveIbMessage(const IIbServiceEndpoint* /*from*/, const FlexrayFrameEvent& msg)
{
    _tracer.Trace(ib::sim::TransmitDirection::RX, msg.timestamp, msg);

    CallHandlers(msg);
}

void FlexrayControllerProxy::ReceiveIbMessage(const IIbServiceEndpoint* /*from*/, const FlexrayFrameTransmitEvent& msg)
{
    FlexrayFrameEvent tmp;
    tmp.frame = msg.frame;
    tmp.channel = msg.channel;
    tmp.timestamp = msg.timestamp;
    _tracer.Trace(ib::sim::TransmitDirection::TX, msg.timestamp, tmp);

    CallHandlers(msg);
}

void FlexrayControllerProxy::ReceiveIbMessage(const IIbServiceEndpoint* /*from*/, const FlexraySymbolEvent& msg)
{
    // Call wakeup handlers on WUS and WUDOP
    switch (msg.pattern)
    {
    case FlexraySymbolPattern::CasMts:
        break;
    case FlexraySymbolPattern::Wus:
    case FlexraySymbolPattern::Wudop:
        // Synthesize a FlexrayWakeupEvent triggered by this FlexraySymbolEvent
        CallHandlers(FlexrayWakeupEvent{msg});
    }

    // In addition, call the generic SymbolHandlers for every symbol
    CallHandlers(msg);
}

void FlexrayControllerProxy::ReceiveIbMessage(const IIbServiceEndpoint* /*from*/, const FlexraySymbolTransmitEvent& msg)
{
    CallHandlers(msg);
}

void FlexrayControllerProxy::ReceiveIbMessage(const IIbServiceEndpoint* /*from*/, const FlexrayCycleStartEvent& msg)
{
    CallHandlers(msg);
}

void FlexrayControllerProxy::ReceiveIbMessage(const IIbServiceEndpoint* /*from*/, const FlexrayPocStatusEvent& msg)
{
    CallHandlers(msg);
}

void FlexrayControllerProxy::WarnOverride(const std::string& parameterName)
{
    std::stringstream ss;
    ss << "Discarded user-defined configuration of " << parameterName
       << ", as it was already set in the predefined configuration.";

    _participant->GetLogger()->Warn(ss.str());
}

template<typename MsgT>
void FlexrayControllerProxy::RegisterHandler(CallbackT<MsgT> handler)
{
    auto&& handlers = std::get<CallbackVector<MsgT>>(_callbacks);
    handlers.push_back(handler);
}

template<typename MsgT>
void FlexrayControllerProxy::CallHandlers(const MsgT& msg)
{
    auto&& handlers = std::get<CallbackVector<MsgT>>(_callbacks);
    for (auto&& handler : handlers)
    {
        handler(_facade, msg);
    }
}

template<typename MsgT>
void FlexrayControllerProxy::SendIbMessage(MsgT&& msg)
{
    _participant->SendIbMessage(this, std::forward<MsgT>(msg));
}


bool FlexrayControllerProxy::IsClusterParametersConfigurable()
{
    return !_config.clusterParameters.has_value();
}

bool FlexrayControllerProxy::IsNodeParametersConfigurable()
{
    return !_config.nodeParameters.has_value();
}

bool FlexrayControllerProxy::IsTxBufferConfigsConfigurable()
{
    return _config.txBufferConfigurations.empty();
}


} // namespace fr
} // SimModels
} // namespace ib
