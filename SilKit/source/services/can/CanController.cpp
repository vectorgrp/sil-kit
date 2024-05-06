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

#include "silkit/services/logging/ILogger.hpp"

#include "IServiceDiscovery.hpp"
#include "ServiceDatatypes.hpp"
#include "CanController.hpp"
#include "Tracing.hpp"

namespace SilKit {
namespace Services {
namespace Can {

CanController::CanController(Core::IParticipantInternal* participant, SilKit::Config::CanController config,
                             Services::Orchestration::ITimeProvider* timeProvider)
    : _participant(participant)
    , _config{std::move(config)}
    , _simulationBehavior{participant, this, timeProvider}
    , _replayActive{Tracing::IsValidReplayConfig(_config.replay)}
    , _logger{participant->GetLogger()}
{
}

//------------------------
// Trivial or detailed
//------------------------

void CanController::RegisterServiceDiscovery()
{
    Core::Discovery::IServiceDiscovery* disc = _participant->GetServiceDiscovery();
    disc->RegisterServiceDiscoveryHandler([this](Core::Discovery::ServiceDiscoveryEvent::Type discoveryType,
                                                 const Core::ServiceDescriptor& remoteServiceDescriptor) {
        if (_simulationBehavior.IsTrivial())
        {
            // Check if received descriptor has a matching simulated link
            if (discoveryType == Core::Discovery::ServiceDiscoveryEvent::Type::ServiceCreated
                && IsRelevantNetwork(remoteServiceDescriptor))
            {
                Logging::Info(_logger,
                              "Controller '{}' is using the simulated network '{}' and will route all messages to "
                              "the network simulator '{}'",
                              _config.name, remoteServiceDescriptor.GetNetworkName(),
                              remoteServiceDescriptor.GetParticipantName());
                SetDetailedBehavior(remoteServiceDescriptor);
            }
        }
        else
        {
            if (discoveryType == Core::Discovery::ServiceDiscoveryEvent::Type::ServiceRemoved
                && IsRelevantNetwork(remoteServiceDescriptor))
            {
                Logging::Warn(_logger,
                              "The network simulator for controller '{}' left the simulation. The controller is no "
                              "longer simulated.",
                              _config.name);
                SetTrivialBehavior();
            }
        }
    });
}

void CanController::SetDetailedBehavior(const Core::ServiceDescriptor& remoteServiceDescriptor)
{
    _simulationBehavior.SetDetailedBehavior(remoteServiceDescriptor);
}

void CanController::SetTrivialBehavior()
{
    _simulationBehavior.SetTrivialBehavior();
}

auto CanController::GetState() -> CanControllerState
{
    return _controllerState;
}

auto CanController::IsRelevantNetwork(const Core::ServiceDescriptor& remoteServiceDescriptor) const -> bool
{
    // NetSim uses ServiceType::Link and the simulated networkName
    return remoteServiceDescriptor.GetServiceType() == SilKit::Core::ServiceType::Link
           && remoteServiceDescriptor.GetNetworkName() == _serviceDescriptor.GetNetworkName();
}

auto CanController::AllowReception(const IServiceEndpoint* from) const -> bool
{
    return _simulationBehavior.AllowReception(from);
}

template <typename MsgT>
void CanController::SendMsg(MsgT&& msg)
{
    _simulationBehavior.SendMsg(std::move(msg));
}

//------------------------
// Public API + Helpers
//------------------------

void CanController::SetBaudRate(uint32_t rate, uint32_t fdRate, uint32_t xlRate)
{
    _baudRate.baudRate = rate;
    _baudRate.fdBaudRate = fdRate;
    _baudRate.xlBaudRate = xlRate;

    SendMsg(_baudRate);
}

void CanController::Reset()
{
    CanSetControllerMode mode;
    mode.flags.cancelTransmitRequests = 1;
    mode.flags.resetErrorHandling = 1;
    mode.mode = CanControllerState::Uninit;

    SendMsg(mode);
}

void CanController::Start()
{
    ChangeControllerMode(CanControllerState::Started);
}

void CanController::Stop()
{
    ChangeControllerMode(CanControllerState::Stopped);
}

void CanController::Sleep()
{
    ChangeControllerMode(CanControllerState::Sleep);
}

void CanController::ChangeControllerMode(CanControllerState state)
{
    CanSetControllerMode mode;
    mode.flags.cancelTransmitRequests = 0;
    mode.flags.resetErrorHandling = 0;
    mode.mode = state;

    SendMsg(mode);
}

void CanController::SendFrame(const CanFrame& frame, void* userContext)
{
    if (Tracing::IsReplayEnabledFor(_config.replay, Config::Replay::Direction::Send))
    {
        // do not allow user messages from the public API.
        // ReplaySend will send all frames.
        Logging::Debug(_logger, _logOnce, "CanController: Ignoring SendFrame API call due to Replay config on {}",
                       _config.name);
        return;
    }
    WireCanFrameEvent wireCanFrameEvent{};
    wireCanFrameEvent.frame = MakeWireCanFrame(frame);
    wireCanFrameEvent.userContext = userContext;

    SendMsg(wireCanFrameEvent);
}

//------------------------
// ReceiveMsg
//------------------------

void CanController::ReceiveMsg(const IServiceEndpoint* from, const WireCanFrameEvent& msg)
{
    if (!AllowReception(from))
    {
        return;
    }

    if (Tracing::IsReplayEnabledFor(_config.replay, Config::Replay::Direction::Receive))
    {
        Logging::Debug(_logger, _logOnce, "CanController: Ignoring ReceiveMsg API call due to Replay config on {}",
                       _config.name);
        return;
    }

    auto canFrameEvent = ToCanFrameEvent(msg);

    const auto frameDirection = static_cast<DirectionMask>(msg.direction);
    constexpr auto txDirection = static_cast<DirectionMask>(TransmitDirection::TX);
    if ((frameDirection & txDirection) != txDirection)
    {
        canFrameEvent.userContext = nullptr;
    }

    _tracer.Trace(msg.direction, msg.timestamp, canFrameEvent);

    CallHandlers(canFrameEvent);
}

void CanController::ReceiveMsg(const IServiceEndpoint* from, const CanFrameTransmitEvent& msg)
{
    if (!AllowReception(from))
    {
        return;
    }

    CallHandlers(msg);
}

void CanController::ReceiveMsg(const IServiceEndpoint* from, const CanControllerStatus& msg)
{
    if (!AllowReception(from))
    {
        return;
    }

    if (_controllerState != msg.controllerState)
    {
        _controllerState = msg.controllerState;
        CallHandlers(CanStateChangeEvent{msg.timestamp, msg.controllerState});
    }
    if (_errorState != msg.errorState)
    {
        _errorState = msg.errorState;
        CallHandlers(CanErrorStateChangeEvent{msg.timestamp, msg.errorState});
    }
}

//------------------------
// Replay
//------------------------

void CanController::ReplayMessage(const SilKit::IReplayMessage* message)
{
    if (!_replayActive)
    {
        return;
    }

    using namespace SilKit::Tracing;
    switch (message->GetDirection())
    {
    case SilKit::Services::TransmitDirection::TX:
        if (IsReplayEnabledFor(_config.replay, Config::Replay::Direction::Send))
        {
            ReplaySend(message);
        }
        break;
    case SilKit::Services::TransmitDirection::RX:
        if (IsReplayEnabledFor(_config.replay, Config::Replay::Direction::Receive))
        {
            ReplayReceive(message);
        }
        break;
    default:
        throw SilKitError("CanController: replay message has undefined Direction");
        break;
    }
}

void CanController::ReplaySend(const IReplayMessage* replayMessage)
{
    // need to copy the message here.
    // will throw if invalid message type.
    auto msgEvent = dynamic_cast<const Services::Can::WireCanFrameEvent&>(*replayMessage);
    msgEvent.userContext = nullptr;

    SendMsg(msgEvent);
}

void CanController::ReplayReceive(const IReplayMessage* replayMessage)
{
    static Tracing::ReplayServiceDescriptor replayService;
    auto frameEvent = dynamic_cast<const Services::Can::WireCanFrameEvent&>(*replayMessage);
    Services::Can::WireCanFrameEvent msg{};
    msg.timestamp = replayMessage->Timestamp();
    msg.frame = std::move(frameEvent.frame);
    msg.direction = TransmitDirection::RX;
    msg.userContext = nullptr;

    const auto msgEvent = ToCanFrameEvent(msg);
    _tracer.Trace(msg.direction, msg.timestamp, msgEvent);
    CallHandlers(msgEvent);
}

//------------------------
// Handlers
//------------------------

HandlerId CanController::AddFrameHandler(FrameHandler handler, DirectionMask directionMask)
{
    auto filter = FilterT<CanFrameEvent>{[directionMask](const CanFrameEvent& frameEvent) {
        return (((DirectionMask)frameEvent.direction & (DirectionMask)directionMask)) != 0;
    }};
    return AddHandler(std::move(handler), std::move(filter));
}

void CanController::RemoveFrameHandler(HandlerId handlerId)
{
    if (!RemoveHandler<CanFrameEvent>(handlerId))
    {
        _participant->GetLogger()->Warn("RemoveFrameHandler failed: Unknown HandlerId.");
    }
}

HandlerId CanController::AddStateChangeHandler(StateChangeHandler handler)
{
    return AddHandler(std::move(handler));
}

void CanController::RemoveStateChangeHandler(HandlerId handlerId)
{
    if (!RemoveHandler<CanStateChangeEvent>(handlerId))
    {
        _participant->GetLogger()->Warn("RemoveStateChangeHandler failed: Unknown HandlerId.");
    }
}

HandlerId CanController::AddErrorStateChangeHandler(ErrorStateChangeHandler handler)
{
    return AddHandler(std::move(handler));
}

void CanController::RemoveErrorStateChangeHandler(HandlerId handlerId)
{
    if (!RemoveHandler<CanErrorStateChangeEvent>(handlerId))
    {
        _participant->GetLogger()->Warn("RemoveErrorStateChangeHandler failed: Unknown HandlerId.");
    }
}

HandlerId CanController::AddFrameTransmitHandler(FrameTransmitHandler handler, CanTransmitStatusMask statusMask)
{
    auto filter = FilterT<CanFrameTransmitEvent>{[statusMask](const CanFrameTransmitEvent& ack) {
        return ((CanTransmitStatusMask)ack.status & (CanTransmitStatusMask)statusMask) != 0;
    }};
    return AddHandler(std::move(handler), std::move(filter));
}

void CanController::RemoveFrameTransmitHandler(HandlerId handlerId)
{
    if (!RemoveHandler<CanFrameTransmitEvent>(handlerId))
    {
        _participant->GetLogger()->Warn("RemoveFrameTransmitHandler failed: Unknown HandlerId.");
    }
}

template <typename MsgT>
HandlerId CanController::AddHandler(CallbackT<MsgT> handler, FilterT<MsgT> filter)
{
    auto& callbacks = std::get<FilteredCallbacks<MsgT>>(_callbacks);
    return callbacks.Add(std::move(handler), std::move(filter));
}

template <typename MsgT>
bool CanController::RemoveHandler(HandlerId handlerId)
{
    auto& callbacks = std::get<FilteredCallbacks<MsgT>>(_callbacks);
    return callbacks.Remove(handlerId);
}

template <typename MsgT>
void CanController::CallHandlers(const MsgT& msg)
{
    auto& callbacks = std::get<FilteredCallbacks<MsgT>>(_callbacks);
    callbacks.InvokeAll(this, msg);
}

} // namespace Can
} // namespace Services
} // namespace SilKit
