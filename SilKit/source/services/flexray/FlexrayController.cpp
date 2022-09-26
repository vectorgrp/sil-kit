/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include "FlexrayController.hpp"
#include "Validation.hpp"
#include "IServiceDiscovery.hpp"
#include "ServiceDatatypes.hpp"

#include "ILogger.hpp"

namespace SilKit {
namespace Services {
namespace Flexray {

FlexrayController::FlexrayController(Core::IParticipantInternal* participant, Config::FlexrayController config,
                                     Services::Orchestration::ITimeProvider* /*timeProvider*/)
    : _participant(participant)
    , _config{std::move(config)}
{
}

//------------------------
// Detailed Sim
//------------------------

void FlexrayController::RegisterServiceDiscovery()
{
    Core::Discovery::IServiceDiscovery* disc = _participant->GetServiceDiscovery();
    disc->RegisterServiceDiscoveryHandler(
        [this](Core::Discovery::ServiceDiscoveryEvent::Type discoveryType,
                                  const Core::ServiceDescriptor& remoteServiceDescriptor) {
            // check if discovered service is a network simulator (if none is known)
            if (!_simulatedLinkDetected)
            {
                // check if received descriptor has a matching simulated link
                if (discoveryType == Core::Discovery::ServiceDiscoveryEvent::Type::ServiceCreated
                    && IsRelevantNetwork(remoteServiceDescriptor))
                {
                    SetDetailedBehavior(remoteServiceDescriptor);
                }
            }
        });
}

auto FlexrayController::AllowReception(const IServiceEndpoint* from) const -> bool
{
    const auto& fromDescr = from->GetServiceDescriptor();
    return _simulatedLinkDetected &&
           _simulatedLink.GetParticipantName() == fromDescr.GetParticipantName()
           && _serviceDescriptor.GetServiceId() == fromDescr.GetServiceId();
}

auto FlexrayController::IsRelevantNetwork(const Core::ServiceDescriptor& remoteServiceDescriptor) const -> bool
{
    // NetSim uses ServiceType::Link and the simulated networkName
    return remoteServiceDescriptor.GetServiceType() == SilKit::Core::ServiceType::Link
           && remoteServiceDescriptor.GetNetworkName() == _serviceDescriptor.GetNetworkName();
}

// Expose for testing purposes
void FlexrayController::SetDetailedBehavior(const Core::ServiceDescriptor& remoteServiceDescriptor)
{
    _simulatedLinkDetected = true;
    _simulatedLink = remoteServiceDescriptor;
}

//------------------------
// Public API + Helpers
//------------------------

bool FlexrayController::IsClusterParametersConfigurable()
{
    return !_config.clusterParameters.has_value();
}

bool FlexrayController::IsNodeParametersConfigurable()
{
    return !_config.nodeParameters.has_value();
}

bool FlexrayController::IsTxBufferConfigsConfigurable()
{
    return _config.txBufferConfigurations.empty();
}

void FlexrayController::WarnOverride(const std::string& parameterName)
{
    std::stringstream ss;
    ss << "Discarded user-defined configuration of " << parameterName
       << ", as it was already set in the predefined configuration.";

    _participant->GetLogger()->Warn(ss.str());
}

void FlexrayController::Configure(const FlexrayControllerConfig& config)
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
    SendMsg(cfg);
}

void FlexrayController::ReconfigureTxBuffer(uint16_t txBufferIdx, const FlexrayTxBufferConfig& config)
{
    if (txBufferIdx >= _bufferConfigs.size())
    {
        Logging::Error(_participant->GetLogger(), "FlexrayController::ReconfigureTxBuffer() was called with unconfigured txBufferIdx={}", txBufferIdx);
        throw OutOfRangeError{"Unconfigured txBufferIdx!"};
    }

    if (!IsTxBufferConfigsConfigurable())
    {
        Logging::Error(_participant->GetLogger(), "ReconfigureTxBuffer() was called on a preconfigured txBuffer. This is not "
                                        "allowed and the reconfiguration will be discarded.");
        return;
    }

    FlexrayTxBufferConfigUpdate update;
    update.txBufferIndex = txBufferIdx;
    update.txBufferConfig = config;
    SendMsg(update);
}

void FlexrayController::UpdateTxBuffer(const FlexrayTxBufferUpdate& update)
{
    if (update.txBufferIndex >= _bufferConfigs.size())
    {
        Logging::Error(_participant->GetLogger(), "FlexrayController::UpdateTxBuffer() was called with unconfigured txBufferIndex={}", update.txBufferIndex);
        throw OutOfRangeError{"Unconfigured txBufferIndex!"};
    }

    if (_config.clusterParameters)
    {
        const auto isStaticSegment =
            _bufferConfigs.at(update.txBufferIndex).slotId <= _config.clusterParameters->gNumberOfStaticSlots;
        if (isStaticSegment)
        {
            const auto maxLength = _config.clusterParameters->gPayloadLengthStatic * 2u; //FR words to bytes
            if (update.payload.size() > maxLength)
            {
                Logging::Warn(_participant->GetLogger(),
                    "FlexrayController::UpdateTxBuffer() was called with FlexRayTxBufferUpdate.payload size"
                    " exceeding 2*gPayloadLengthStatic ({}). The payload will be truncated.",
                    maxLength);
            }
            if (update.payload.size() < maxLength)
            {
                Logging::Warn(_participant->GetLogger(),
                    "FlexrayController::UpdateTxBuffer() was called with FlexRayTxBufferUpdate.payload size"
                    " lower than 2*gPayloadLengthStatic ({}). The payload will be zero padded.",
                    maxLength);
            }
        }
    }
    SendMsg(MakeWireFlexrayTxBufferUpdate(update));
}

void FlexrayController::Run()
{
    FlexrayHostCommand cmd;
    cmd.command = FlexrayChiCommand::RUN;
    SendMsg(cmd);
}

void FlexrayController::DeferredHalt()
{
    FlexrayHostCommand cmd;
    cmd.command = FlexrayChiCommand::DEFERRED_HALT;
    SendMsg(cmd);
}

void FlexrayController::Freeze()
{
    FlexrayHostCommand cmd;
    cmd.command = FlexrayChiCommand::FREEZE;
    SendMsg(cmd);
}

void FlexrayController::AllowColdstart()
{
    FlexrayHostCommand cmd;
    cmd.command = FlexrayChiCommand::ALLOW_COLDSTART;
    SendMsg(cmd);
}

void FlexrayController::AllSlots()
{
    FlexrayHostCommand cmd;
    cmd.command = FlexrayChiCommand::ALL_SLOTS;
    SendMsg(cmd);
}

void FlexrayController::Wakeup()
{
    FlexrayHostCommand cmd;
    cmd.command = FlexrayChiCommand::WAKEUP;
    SendMsg(cmd);
}

//------------------------
// ReceiveMsg
//------------------------

void FlexrayController::ReceiveMsg(const IServiceEndpoint* from, const WireFlexrayFrameEvent& msg)
{
    if (!AllowReception(from))
    {
        return;
    }

    _tracer.Trace(SilKit::Services::TransmitDirection::RX, msg.timestamp, ToFlexrayFrameEvent(msg));
    CallHandlers(ToFlexrayFrameEvent(msg));
}

void FlexrayController::ReceiveMsg(const IServiceEndpoint* from, const WireFlexrayFrameTransmitEvent& msg)
{
    if (!AllowReception(from))
    {
        return;
    }

    FlexrayFrameEvent tmp;
    tmp.frame = ToFlexrayFrame(msg.frame);
    tmp.channel = msg.channel;
    tmp.timestamp = msg.timestamp;
    _tracer.Trace(SilKit::Services::TransmitDirection::TX, msg.timestamp, tmp);

    CallHandlers(ToFlexrayFrameTransmitEvent(msg));
}

void FlexrayController::ReceiveMsg(const IServiceEndpoint* from, const FlexraySymbolEvent& msg)
{
    if (!AllowReception(from))
    {
        return;
    }

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

void FlexrayController::ReceiveMsg(const IServiceEndpoint* from, const FlexraySymbolTransmitEvent& msg)
{
    if (!AllowReception(from))
    {
        return;
    }

    CallHandlers(msg);
}

void FlexrayController::ReceiveMsg(const IServiceEndpoint* from, const FlexrayCycleStartEvent& msg)
{
    if (!AllowReception(from))
    {
        return;
    }

    CallHandlers(msg);
}

void FlexrayController::ReceiveMsg(const IServiceEndpoint* from, const FlexrayPocStatusEvent& msg)
{
    if (!AllowReception(from))
    {
        return;
    }

    CallHandlers(msg);
}


template <typename MsgT>
void FlexrayController::SendMsg(MsgT&& msg)
{
    _participant->SendMsg(this, std::forward<MsgT>(msg));
}

//------------------------
// Handlers
//------------------------

HandlerId FlexrayController::AddFrameHandler(FrameHandler handler)
{
    return AddHandler(std::move(handler));
}

void FlexrayController::RemoveFrameHandler(HandlerId handlerId)
{
    if (!RemoveHandler<FlexrayFrameEvent>(handlerId))
    {
        Logging::Warn(_participant->GetLogger(), "RemoveFrameHandler failed: Unknown HandlerId.");
    }
}

HandlerId FlexrayController::AddFrameTransmitHandler(FrameTransmitHandler handler)
{
    return AddHandler(std::move(handler));
}

void FlexrayController::RemoveFrameTransmitHandler(HandlerId handlerId)
{
    if (!RemoveHandler<FlexrayFrameTransmitEvent>(handlerId))
    {
        Logging::Warn(_participant->GetLogger(),"RemoveFrameTransmitHandler failed: Unknown HandlerId.");
    }
}

HandlerId FlexrayController::AddWakeupHandler(WakeupHandler handler)
{
    return AddHandler(std::move(handler));
}

void FlexrayController::RemoveWakeupHandler(HandlerId handlerId)
{
    if (!RemoveHandler<FlexrayWakeupEvent>(handlerId))
    {
        Logging::Warn(_participant->GetLogger(), "RemoveWakeupHandler failed: Unknown HandlerId.");
    }
}

HandlerId FlexrayController::AddPocStatusHandler(PocStatusHandler handler)
{
    return AddHandler(std::move(handler));
}

void FlexrayController::RemovePocStatusHandler(HandlerId handlerId)
{
    if (!RemoveHandler<FlexrayPocStatusEvent>(handlerId))
    {
        Logging::Warn(_participant->GetLogger(), "RemovePocStatusHandler failed: Unknown HandlerId.");
    }
}

HandlerId FlexrayController::AddSymbolHandler(SymbolHandler handler)
{
    return AddHandler(std::move(handler));
}

void FlexrayController::RemoveSymbolHandler(HandlerId handlerId)
{
    if (!RemoveHandler<FlexraySymbolEvent>(handlerId))
    {
        Logging::Warn(_participant->GetLogger(), "RemoveSymbolHandler failed: Unknown HandlerId.");
    }
}

HandlerId FlexrayController::AddSymbolTransmitHandler(SymbolTransmitHandler handler)
{
    return AddHandler(std::move(handler));
}

void FlexrayController::RemoveSymbolTransmitHandler(HandlerId handlerId)
{
    if (!RemoveHandler<FlexraySymbolTransmitEvent>(handlerId))
    {
        Logging::Warn(_participant->GetLogger(), "RemoveSymbolTransmitHandler failed: Unknown HandlerId.");
    }
}

HandlerId FlexrayController::AddCycleStartHandler(CycleStartHandler handler)
{
    return AddHandler(std::move(handler));
}

void FlexrayController::RemoveCycleStartHandler(HandlerId handlerId)
{
    if (!RemoveHandler<FlexrayCycleStartEvent>(handlerId))
    {
        Logging::Warn(_participant->GetLogger(), "RemoveCycleStartHandler failed: Unknown HandlerId.");
    }
}

template <typename MsgT>
HandlerId FlexrayController::AddHandler(CallbackT<MsgT> handler)
{
    auto& callbacks = std::get<CallbacksT<MsgT>>(_callbacks);
    return callbacks.Add(std::move(handler));
}

template <typename MsgT>
auto FlexrayController::RemoveHandler(HandlerId handlerId) -> bool
{
    auto& callbacks = std::get<CallbacksT<MsgT>>(_callbacks);
    return callbacks.Remove(handlerId);
}

template <typename MsgT>
void FlexrayController::CallHandlers(const MsgT& msg)
{
    auto& callbacks = std::get<CallbacksT<MsgT>>(_callbacks);
    callbacks.InvokeAll(this, msg);
}

} // namespace Flexray
} // namespace Services
} // namespace SilKit
