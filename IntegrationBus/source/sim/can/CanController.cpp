// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "CanController.hpp"

#include <algorithm>

namespace ib {
namespace sim {
namespace can {


CanController::CanController(mw::IParticipantInternal* participant, 
                             const ib::cfg::CanController& config,
                             mw::sync::ITimeProvider* timeProvider,
                             ICanController* facade)
    : _participant{participant}
    , _config{config}
    , _timeProvider{timeProvider}
    , _facade{facade}
{
    if (_facade == nullptr)
    {
        _facade = this;
    }
}

void CanController::SetBaudRate(uint32_t /*rate*/, uint32_t /*fdRate*/)
{
}

void CanController::Reset()
{
}

void CanController::Start()
{
}

void CanController::Stop()
{
}

void CanController::Sleep()
{
}

auto CanController::SendFrame(const CanFrame& frame, void* userContext) -> CanTxId
{
    auto now = _timeProvider->Now();
    // ignore the user's API calls if we're configured for replay
    //if (tracing::IsReplayEnabledFor(_config.replay, cfg::Replay::Direction::Send))
    //{
    //    return 0;
    //}

    CanFrameEvent canFrameEvent{};
    canFrameEvent.frame = frame;
    canFrameEvent.userContext = userContext;
    canFrameEvent.direction = TransmitDirection::TX;
    canFrameEvent.transmitId = MakeTxId();
    canFrameEvent.timestamp = now;

    _tracer.Trace(ib::sim::TransmitDirection::TX, now, canFrameEvent);
    // has to be called before SendIbMessage because of thread change
    CallHandlers(canFrameEvent);

    _participant->SendIbMessage(this, canFrameEvent);

    // instantly call transmit acknowledge
    CanFrameTransmitEvent ack{};
    ack.canId = frame.canId;
    ack.status = CanTransmitStatus::Transmitted;
    ack.transmitId = canFrameEvent.transmitId;
    ack.userContext = userContext;
    ack.timestamp = now;
    CallHandlers(ack);

    return canFrameEvent.transmitId;
}

void CanController::AddFrameHandler(FrameHandler handler, DirectionMask directionMask)
{
    std::function<bool(const CanFrameEvent&)> filter = [directionMask](const CanFrameEvent& msg) {
        return ((DirectionMask)msg.direction & (DirectionMask)directionMask) != 0;
    };
    RegisterHandler(handler, std::move(filter));
}

void CanController::AddStateChangeHandler(StateChangeHandler /*handler*/)
{
}

void CanController::AddErrorStateChangeHandler(ErrorStateChangeHandler /*handler*/)
{
}

void CanController::AddFrameTransmitHandler(FrameTransmitHandler handler, CanTransmitStatusMask statusMask)
{
    std::function<bool(const CanFrameTransmitEvent& )> filter = [statusMask](const CanFrameTransmitEvent& ack) {
        return ((CanTransmitStatusMask)ack.status & (CanTransmitStatusMask)statusMask) != 0; 
    };
    RegisterHandler(handler, filter);
}

template<typename MsgT>
void CanController::RegisterHandler(CallbackT<MsgT> handler, std::function<bool(const MsgT& msg)> filter)
{
    auto&& handlers = std::get<CallbackVector<MsgT>>(_callbacks);
    auto handler_tuple = std::make_tuple(handler, filter);
    handlers.push_back(handler_tuple);
}

void CanController::ReceiveIbMessage(const IIbServiceEndpoint* /*from*/, const CanFrameEvent& canFrameEvent)
{
    // ignore messages that do not originate from the replay scheduler 
    //if (tracing::IsReplayEnabledFor(_config.replay, cfg::Replay::Direction::Receive))
    //{
    //    return;
    //}
    
    auto msgCopy = canFrameEvent;
    msgCopy.direction = TransmitDirection::RX;
    CallHandlers(msgCopy);

    _tracer.Trace(ib::sim::TransmitDirection::RX, _timeProvider->Now(), std::move(msgCopy));
}

template<typename MsgT>
void CanController::CallHandlers(const MsgT& msg)
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

void CanController::SetTimeProvider(ib::mw::sync::ITimeProvider* timeProvider)
{
    _timeProvider = timeProvider;
}

void CanController::ReplayMessage(const extensions::IReplayMessage* replayMessage)
{
    using namespace ib::tracing;
    switch (replayMessage->GetDirection())
    {
    case ib::sim::TransmitDirection::TX:
        //if (IsReplayEnabledFor(_config.replay, cfg::Replay::Direction::Send))
        //{
        //    ReplaySend(replayMessage);
        //}
        break;
    case ib::sim::TransmitDirection::RX:
        //if (IsReplayEnabledFor(_config.replay, cfg::Replay::Direction::Receive))
        //{
        //    ReplayReceive(replayMessage);
        //}
        break;
    default:
        throw std::runtime_error("CanReplayController: replay message has undefined Direction");
        break;
    }
}

void CanController::ReplaySend(const extensions::IReplayMessage* replayMessage)
{
    // need to copy the message here.
    // will throw if invalid message type.
    auto msg = dynamic_cast<const sim::can::CanFrameEvent&>(*replayMessage);
    SendFrame(std::move(msg.frame));
}

void CanController::ReplayReceive(const extensions::IReplayMessage* replayMessage)
{
    static tracing::ReplayServiceDescriptor replayService;
    auto msg = dynamic_cast<const sim::can::CanFrameEvent&>(*replayMessage);
    ReceiveIbMessage(&replayService, msg);
}


} // namespace can
} // namespace sim
} // namespace ib
