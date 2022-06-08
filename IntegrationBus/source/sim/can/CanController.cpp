// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "ib/mw/logging/ILogger.hpp"

#include "IServiceDiscovery.hpp"
#include "ServiceDatatypes.hpp"
#include "CanController.hpp"

namespace ib {
namespace sim {
namespace can {

CanController::CanController(mw::IParticipantInternal* participant, ib::cfg::CanController config,
                             mw::sync::ITimeProvider* timeProvider)
    : _participant(participant)
    , _config{config}
    , _simulationBehavior{participant, this, timeProvider}
{
}

//------------------------
// Trivial or detailed
//------------------------

void CanController::RegisterServiceDiscovery()
{
    mw::service::IServiceDiscovery* disc = _participant->GetServiceDiscovery();
    disc->RegisterServiceDiscoveryHandler([this](mw::service::ServiceDiscoveryEvent::Type discoveryType,
                                                 const mw::ServiceDescriptor& remoteServiceDescriptor) {
        if (_simulationBehavior.IsTrivial())
        {
            // Check if received descriptor has a matching simulated link
            if (discoveryType == mw::service::ServiceDiscoveryEvent::Type::ServiceCreated
                && IsRelevantNetwork(remoteServiceDescriptor))
            {
                SetDetailedBehavior(remoteServiceDescriptor);
            }
        }
        else
        {
            if (discoveryType == mw::service::ServiceDiscoveryEvent::Type::ServiceRemoved
                && IsRelevantNetwork(remoteServiceDescriptor))
            {
                SetTrivialBehavior();
            }
        }
    });
}

void CanController::SetDetailedBehavior(const mw::ServiceDescriptor& remoteServiceDescriptor)
{
    _simulationBehavior.SetDetailedBehavior(remoteServiceDescriptor);
}
void CanController::SetTrivialBehavior()
{
    _simulationBehavior.SetTrivialBehavior();
}

auto CanController::IsRelevantNetwork(const mw::ServiceDescriptor& remoteServiceDescriptor) const -> bool
{
    // NetSim uses ServiceType::Link and the simulated networkName
    return remoteServiceDescriptor.GetServiceType() == ib::mw::ServiceType::Link
           && remoteServiceDescriptor.GetNetworkName() == _serviceDescriptor.GetNetworkName();
}

auto CanController::AllowReception(const IIbServiceEndpoint* from) const -> bool
{
    return _simulationBehavior.AllowReception(from);
}

template <typename MsgT>
void CanController::SendIbMessage(MsgT&& msg)
{
    _simulationBehavior.SendIbMessage(std::move(msg));
}

//------------------------
// Public API + Helpers
//------------------------

void CanController::SetBaudRate(uint32_t rate, uint32_t fdRate)
{
    _baudRate.baudRate = rate;
    _baudRate.fdBaudRate = fdRate;

    SendIbMessage(_baudRate);
}

void CanController::Reset()
{
    CanSetControllerMode mode;
    mode.flags.cancelTransmitRequests = 1;
    mode.flags.resetErrorHandling = 1;
    mode.mode = CanControllerState::Uninit;

    SendIbMessage(mode);
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

    SendIbMessage(mode);
}

auto CanController::SendFrame(const CanFrame& frame, void* userContext) -> CanTxId
{
    CanFrameEvent canFrameEvent{};
    canFrameEvent.frame = frame;
    canFrameEvent.transmitId = MakeTxId();
    canFrameEvent.userContext = userContext;

    SendIbMessage(canFrameEvent);
    return canFrameEvent.transmitId;
}

//------------------------
// ReceiveIbMessage
//------------------------

void CanController::ReceiveIbMessage(const IIbServiceEndpoint* from, const CanFrameEvent& msg)
{
    if (!AllowReception(from))
    {
        return;
    }

    _tracer.Trace(ib::sim::TransmitDirection::RX, msg.timestamp, msg);
    CallHandlers(msg);
}

void CanController::ReceiveIbMessage(const IIbServiceEndpoint* from, const CanFrameTransmitEvent& msg)
{
    if (!AllowReception(from))
    {
        return;
    }

    CallHandlers(msg);
}

void CanController::ReceiveIbMessage(const IIbServiceEndpoint* from, const CanControllerStatus& msg)
{
    if (!AllowReception(from))
    {
        return;
    }

    if (_controllerState != msg.controllerState)
    {
        _controllerState = msg.controllerState;
        CallHandlers(CanStateChangeEvent{ msg.timestamp, msg.controllerState });
    }
    if (_errorState != msg.errorState)
    {
        _errorState = msg.errorState;
        CallHandlers(CanErrorStateChangeEvent{ msg.timestamp, msg.errorState });
    }
}

//------------------------
// Handlers
//------------------------

HandlerId CanController::AddFrameHandler(FrameHandler handler, DirectionMask directionMask)
{
    std::function<bool(const CanFrameEvent&)> filter = [directionMask](const CanFrameEvent& frameEvent) {
        return (((DirectionMask)frameEvent.direction & (DirectionMask)directionMask)) != 0;
    };
    return AddHandler(handler, std::move(filter));
}
void CanController::RemoveFrameHandler(HandlerId handlerId)
{
    RemoveHandler<CanFrameEvent>(handlerId);
}

HandlerId CanController::AddStateChangeHandler(StateChangeHandler handler)
{
    return AddHandler(handler);
}
void CanController::RemoveStateChangeHandler(HandlerId handlerId)
{
    RemoveHandler<CanStateChangeEvent>(handlerId);
}

HandlerId CanController::AddErrorStateChangeHandler(ErrorStateChangeHandler handler)
{
    return AddHandler(handler);
}
void CanController::RemoveErrorStateChangeHandler(HandlerId handlerId)
{
    RemoveHandler<CanErrorStateChangeEvent>(handlerId);
}

HandlerId CanController::AddFrameTransmitHandler(FrameTransmitHandler handler, CanTransmitStatusMask statusMask)
{
    std::function<bool(const CanFrameTransmitEvent&)> filter = [statusMask](const CanFrameTransmitEvent& ack) {
        return ((CanTransmitStatusMask)ack.status & (CanTransmitStatusMask)statusMask) != 0;
    };
    return AddHandler(handler, filter);
}
void CanController::RemoveFrameTransmitHandler(HandlerId handlerId)
{
    RemoveHandler<CanFrameTransmitEvent>(handlerId);
}

template <typename MsgT>
HandlerId CanController::AddHandler(CallbackT<MsgT> handler, std::function<bool(const MsgT& msg)> filter)
{
    std::unique_lock<decltype(_callbacksMx)> lock(_callbacksMx);

    static uint64_t handlerId = 0;
    auto&& handlersMap = std::get<CallbackMap<MsgT>>(_callbacks);
    handlersMap.emplace(handlerId, FilteredCallback<MsgT>{handler, filter});
    return handlerId++;
}

template <typename MsgT>
void CanController::RemoveHandler(HandlerId handlerId)
{
    std::unique_lock<decltype(_callbacksMx)> lock(_callbacksMx);

    auto&& handlersMap = std::get<CallbackMap<MsgT>>(_callbacks);

    auto handlerToRemove = handlersMap.find(handlerId);
    if (handlerToRemove == handlersMap.end())
    {
        _participant->GetLogger()->Warn("RemoveHandler failed: Unknown HandlerId.");
    }
    else
    {
        handlersMap.erase(handlerId);
    }
}

template <typename MsgT>
void CanController::CallHandlers(const MsgT& msg)
{
    std::unique_lock<decltype(_callbacksMx)> lock(_callbacksMx);

    auto&& handlers = std::get<CallbackMap<MsgT>>(_callbacks);
    for (auto&& filteredCallback : handlers)
    {
        if (!filteredCallback.second.filter || filteredCallback.second.filter(msg))
        {
            filteredCallback.second.callback(this, msg);
        }
    }
}

} // namespace can
} // namespace sim
} // namespace ib
