// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "CanControllerProxy.hpp"


namespace ib {
namespace sim {
namespace can {

CanControllerProxy::CanControllerProxy(mw::IParticipantInternal* participant, ICanController* facade)
: _participant(participant)
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
    _participant->SendIbMessage(this, _baudRate);
}

void CanControllerProxy::Reset()
{
    // prepare message to be sent
    CanSetControllerMode mode;
    mode.flags.cancelTransmitRequests = 1;
    mode.flags.resetErrorHandling = 1;
    mode.mode = CanControllerState::Uninit;

    _participant->SendIbMessage(this, mode);
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

    _participant->SendIbMessage(this, mode);
}

auto CanControllerProxy::SendFrame(const CanFrame& frame, void* userContext) -> CanTxId
{
    CanFrameEvent canFrameEvent{};
    canFrameEvent.frame = frame;
    canFrameEvent.transmitId = MakeTxId();
    canFrameEvent.frame.userContext = userContext;

    _participant->SendIbMessage(this, canFrameEvent);
    return canFrameEvent.transmitId;
}

void CanControllerProxy::AddFrameHandler(FrameHandler handler, DirectionMask directionMask)
{
    std::function<bool(const CanFrameEvent&)> filter = [directionMask](const CanFrameEvent& frameEvent) {
        return (((DirectionMask)frameEvent.frame.direction & (DirectionMask)directionMask)) != 0;
    };
    RegisterHandler(handler, std::move(filter));
}

void CanControllerProxy::AddStateChangeHandler(StateChangeHandler handler)
{
    RegisterHandler(handler);
}

void CanControllerProxy::AddErrorStateChangeHandler(ErrorStateChangeHandler handler)
{
    RegisterHandler(handler);
}

void CanControllerProxy::AddFrameTransmitHandler(FrameTransmitHandler handler, CanTransmitStatusMask statusMask)
{
    std::function<bool(const CanFrameTransmitEvent&)> filter = [statusMask](const CanFrameTransmitEvent& ack) {
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

void CanControllerProxy::ReceiveIbMessage(const IIbServiceEndpoint* /*from*/, const CanFrameEvent& msg)
{
    _tracer.Trace(ib::sim::TransmitDirection::RX, msg.timestamp, msg);
    CallHandlers(msg);
}

void CanControllerProxy::ReceiveIbMessage(const IIbServiceEndpoint* /*from*/, const CanFrameTransmitEvent& msg)
{
    CallHandlers(msg);
}

void CanControllerProxy::ReceiveIbMessage(const IIbServiceEndpoint* /*from*/, const CanControllerStatus& msg)
{
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
