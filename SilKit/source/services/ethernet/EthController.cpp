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

#include "EthController.hpp"
#include "silkit/services/logging/ILogger.hpp"

#include "IServiceDiscovery.hpp"
#include "ServiceDatatypes.hpp"


namespace SilKit {
namespace Services {
namespace Ethernet {

EthController::EthController(Core::IParticipantInternal* participant, Config::EthernetController config,
                               Services::Orchestration::ITimeProvider* timeProvider)
    : _participant(participant)
    , _config{config}
    , _simulationBehavior{participant, this, timeProvider}
{
}

//------------------------
// Trivial or detailed
//------------------------

void EthController::RegisterServiceDiscovery()
{
    Core::Discovery::IServiceDiscovery* disc = _participant->GetServiceDiscovery();
    disc->RegisterServiceDiscoveryHandler(
        [this](Core::Discovery::ServiceDiscoveryEvent::Type discoveryType,
                                  const Core::ServiceDescriptor& remoteServiceDescriptor) {
            if (_simulationBehavior.IsTrivial())
            {
                // Check if received descriptor has a matching simulated link
                if (discoveryType == Core::Discovery::ServiceDiscoveryEvent::Type::ServiceCreated
                    && IsRelevantNetwork(remoteServiceDescriptor))
                {
                    SetDetailedBehavior(remoteServiceDescriptor);
                }
            }
            else
            {
                if (discoveryType == Core::Discovery::ServiceDiscoveryEvent::Type::ServiceRemoved
                    && IsRelevantNetwork(remoteServiceDescriptor))
                {
                    SetTrivialBehavior();
                }
            }
        });
}

void EthController::SetDetailedBehavior(const Core::ServiceDescriptor& remoteServiceDescriptor)
{
    _simulationBehavior.SetDetailedBehavior(remoteServiceDescriptor);
}
void EthController::SetTrivialBehavior()
{
    _simulationBehavior.SetTrivialBehavior();
}

EthernetState EthController::GetState()
{
    return _ethState;
}

auto EthController::IsRelevantNetwork(const Core::ServiceDescriptor& remoteServiceDescriptor) const -> bool
{
    // NetSim uses ServiceType::Link and the simulated networkName
    return remoteServiceDescriptor.GetServiceType() == SilKit::Core::ServiceType::Link
           && remoteServiceDescriptor.GetNetworkName() == _serviceDescriptor.GetNetworkName();
}

auto EthController::AllowReception(const IServiceEndpoint* from) const -> bool
{
    return _simulationBehavior.AllowReception(from);
}

template <typename MsgT>
void EthController::SendMsg(MsgT&& msg)
{
    _simulationBehavior.SendMsg(std::move(msg));
}

//------------------------
// Public API + Helpers
//------------------------

void EthController::Activate()
{
    // Check if the Controller has already been activated
    if (_ethState != EthernetState::Inactive)
        return;

    EthernetSetMode msg { EthernetMode::Active };
    SendMsg(msg);
}

void EthController::Deactivate()
{
    // Check if the Controller has already been deactivated
    if (_ethState == EthernetState::Inactive)
        return;

    EthernetSetMode msg{ EthernetMode::Inactive };
    SendMsg(msg);
}

void EthController::SendFrame(EthernetFrame frame, void* userContext)
{
    WireEthernetFrameEvent msg{};
    msg.frame = MakeWireEthernetFrame(frame);
    msg.userContext = userContext;

    SendMsg(std::move(msg));
}

//------------------------
// ReceiveMsg
//------------------------

void EthController::ReceiveMsg(const IServiceEndpoint* from, const WireEthernetFrameEvent& msg)
{
    if (!AllowReception(from))
    {
        return;
    }
    _tracer.Trace(msg.direction, msg.timestamp, ToEthernetFrame(msg.frame));
    CallHandlers(ToEthernetFrameEvent(msg));
}

void EthController::ReceiveMsg(const IServiceEndpoint* from, const EthernetFrameTransmitEvent& msg)
{
    if (!AllowReception(from))
    {
        return;
    }

    _simulationBehavior.OnReceiveAck(msg);
    CallHandlers(msg);
}

void EthController::ReceiveMsg(const IServiceEndpoint* from, const EthernetStatus& msg)
{
    if (!AllowReception(from))
    {
        return;
    }

    // In case we are in early startup, ensure we tell our participants the bit rate first
    // and the state later.
    if (msg.bitrate != _ethBitRate)
    {
        _ethBitRate = msg.bitrate;
        CallHandlers(EthernetBitrateChangeEvent{ msg.timestamp, msg.bitrate });
    }

    if (msg.state != _ethState)
    {
        _ethState = msg.state;
        CallHandlers(EthernetStateChangeEvent{ msg.timestamp, msg.state });
    }
}

//------------------------
// Handlers
//------------------------

HandlerId EthController::AddFrameHandler(FrameHandler handler, DirectionMask directionMask)
{
    return AddHandler(FrameHandler{[handler = std::move(handler), directionMask](
                                       IEthernetController* controller, const EthernetFrameEvent& ethernetFrameEvent) {
        if (static_cast<DirectionMask>(ethernetFrameEvent.direction) & directionMask)
        {
            handler(controller, ethernetFrameEvent);
        }
    }});
}

void EthController::RemoveFrameHandler(HandlerId handlerId)
{
    if (!RemoveHandler<EthernetFrameEvent>(handlerId))
    {
        _participant->GetLogger()->Warn("RemoveFrameHandler failed: Unknown HandlerId.");
    }
}

HandlerId EthController::AddFrameTransmitHandler(FrameTransmitHandler handler, EthernetTransmitStatusMask transmitStatusMask)
{
    return AddHandler(FrameTransmitHandler{
        [handler = std::move(handler), transmitStatusMask](
            IEthernetController* controller, const EthernetFrameTransmitEvent& ethernetFrameTransmitEvent) {
            if (static_cast<EthernetTransmitStatusMask>(ethernetFrameTransmitEvent.status) & transmitStatusMask)
            {
                handler(controller, ethernetFrameTransmitEvent);
            }
        }});
}

void EthController::RemoveFrameTransmitHandler(HandlerId handlerId)
{
    if (!RemoveHandler<EthernetFrameTransmitEvent>(handlerId))
    {
        _participant->GetLogger()->Warn("RemoveFrameTransmitHandler failed: Unknown HandlerId.");
    }
}

HandlerId EthController::AddStateChangeHandler(StateChangeHandler handler)
{
    return AddHandler(std::move(handler));
}

void EthController::RemoveStateChangeHandler(HandlerId handlerId)
{
    if (!RemoveHandler<EthernetStateChangeEvent>(handlerId))
    {
        _participant->GetLogger()->Warn("RemoveStateChangeHandler failed: Unknown HandlerId.");
    }
}

HandlerId EthController::AddBitrateChangeHandler(BitrateChangeHandler handler)
{
    return AddHandler(std::move(handler));
}

void EthController::RemoveBitrateChangeHandler(HandlerId handlerId)
{
    if (!RemoveHandler<EthernetBitrateChangeEvent>(handlerId))
    {
        _participant->GetLogger()->Warn("RemoveBitrateChangeHandler failed: Unknown HandlerId.");
    }
}

template <typename MsgT>
HandlerId EthController::AddHandler(CallbackT<MsgT> handler)
{
    auto& callbacks = std::get<CallbacksT<MsgT>>(_callbacks);
    return callbacks.Add(std::move(handler));
}

template <typename MsgT>
auto EthController::RemoveHandler(HandlerId handlerId) -> bool
{
    auto& callbacks = std::get<CallbacksT<MsgT>>(_callbacks);
    return callbacks.Remove(handlerId);
}

template <typename MsgT>
void EthController::CallHandlers(const MsgT& msg)
{
    auto& callbacks = std::get<CallbacksT<MsgT>>(_callbacks);
    callbacks.InvokeAll(this, msg);
}

} // namespace Ethernet
} // namespace Services
} // namespace SilKit
