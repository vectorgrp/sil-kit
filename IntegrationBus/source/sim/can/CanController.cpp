// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "CanController.hpp"

#include <algorithm>

namespace ib {
namespace sim {
namespace can {


CanController::CanController(mw::IComAdapterInternal* comAdapter, 
                             const ib::cfg::v1::datatypes::CanController& config,
                             mw::sync::ITimeProvider* timeProvider)
    : _comAdapter{comAdapter}
    , _config{config}
    , _timeProvider{timeProvider}
{
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

auto CanController::SendMessage(const CanMessage& msg, void* userContext) -> CanTxId
{
    auto now = _timeProvider->Now();
    // ignore the user's API calls if we're configured for replay
    //if (tracing::IsReplayEnabledFor(_config.replay, cfg::Replay::Direction::Send))
    //{
    //    return 0;
    //}

    auto msgCopy = msg;
    msgCopy.userContext = userContext;
    msgCopy.transmitId = MakeTxId();
    msgCopy.direction = TransmitDirection::TX;
    msgCopy.timestamp = now;

    _tracer.Trace(ib::sim::TransmitDirection::TX, now, msg);
    // has to be called before SendIbMessage because of thread change
    CallHandlers(msg);

    _comAdapter->SendIbMessage(this, msgCopy);

    // instantly call transmit acknowledge
    CanTransmitAcknowledge ack;
    ack.canId = msg.canId;
    ack.status = CanTransmitStatus::Transmitted;
    ack.transmitId = msgCopy.transmitId;
    ack.timestamp = msg.timestamp;
    ack.userContext = userContext;
    ack.timestamp = now;
    CallHandlers(ack);

    return msgCopy.transmitId;
}

auto CanController::SendMessage(CanMessage&& msg, void* userContext) -> CanTxId
{
    auto now = _timeProvider->Now();
    // ignore the user's API calls if we're configured for replay
    //if (tracing::IsReplayEnabledFor(_config.replay, cfg::Replay::Direction::Send))
    //{
    //    return 0;
    //}

    msg.userContext = userContext;
    auto txId = MakeTxId();
    msg.transmitId = txId;
    msg.direction = TransmitDirection::TX;
    msg.timestamp = now;

    _tracer.Trace(ib::sim::TransmitDirection::TX, now, msg);
    CallHandlers(msg);

    _comAdapter->SendIbMessage(this, std::move(msg));
    
    // instantly call transmit acknowledge
    CanTransmitAcknowledge ack;
    ack.canId = msg.canId;
    ack.status = CanTransmitStatus::Transmitted;
    ack.transmitId = msg.transmitId;
    ack.timestamp = msg.timestamp;
    ack.userContext = userContext;
    ack.timestamp = now;
    CallHandlers(ack);

    return txId;
}

void CanController::RegisterReceiveMessageHandler(ReceiveMessageHandler handler, DirectionMask directionMask)
{
    std::function<bool(const CanMessage&)> filter = [directionMask](const CanMessage& msg) {
        return (DirectionMask)msg.direction & (DirectionMask)directionMask;
    };
    RegisterHandler(handler, std::move(filter));
}

void CanController::RegisterStateChangedHandler(StateChangedHandler /*handler*/)
{
}

void CanController::RegisterErrorStateChangedHandler(ErrorStateChangedHandler /*handler*/)
{
}

void CanController::RegisterTransmitStatusHandler(MessageStatusHandler handler, CanTransmitStatusMask statusMask)
{
    std::function<bool(const CanTransmitAcknowledge& )> filter = [statusMask](const CanTransmitAcknowledge& ack) {
        return (CanTransmitStatusMask)ack.status & (CanTransmitStatusMask)statusMask; 
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

void CanController::ReceiveIbMessage(const IIbServiceEndpoint* from, const CanMessage& msg)
{
    // ignore messages that do not originate from the replay scheduler 
    //if (tracing::IsReplayEnabledFor(_config.replay, cfg::Replay::Direction::Receive))
    //{
    //    return;
    //}
    
    auto msgCopy = msg;
    msgCopy.userContext = nullptr;
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
            handler(this, msg);
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
    auto msg = dynamic_cast<const sim::can::CanMessage&>(*replayMessage);
    SendMessage(std::move(msg));
}

void CanController::ReplayReceive(const extensions::IReplayMessage* replayMessage)
{
    static tracing::ReplayServiceDescriptor replayService;
    auto msg = dynamic_cast<const sim::can::CanMessage&>(*replayMessage);
    ReceiveIbMessage(&replayService, msg);
}


} // namespace can
} // namespace sim
} // namespace ib
