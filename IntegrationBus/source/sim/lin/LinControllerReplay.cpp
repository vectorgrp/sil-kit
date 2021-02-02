// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "LinControllerReplay.hpp"

#include <iostream>
#include <chrono>

#include "ib/mw/IComAdapter.hpp"
#include "ib/mw/logging/ILogger.hpp"
#include "ib/sim/lin/string_utils.hpp"

namespace ib {
namespace sim {
namespace lin {


LinControllerReplay::LinControllerReplay(mw::IComAdapter* comAdapter, cfg::LinController config,
            mw::sync::ITimeProvider* timeProvider)
    : _replayConfig{config.replay}
    , _controller{comAdapter, timeProvider}
    , _comAdapter{comAdapter}
{
}

void LinControllerReplay::Init(ControllerConfig config)
{
    // We explicitly rely on the Master/Slave properly initializing.
    //if (tracing::IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Send))
    //{
    //    return;
    //}
    _mode = config.controllerMode;
    _controller.Init(config);
}

auto LinControllerReplay::Status() const noexcept -> ControllerStatus
{
    return _controller.Status();
}

void LinControllerReplay::SendFrame(Frame frame, FrameResponseType responseType)
{
    if (tracing::IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Send))
    {
        return;
    }
    _controller.SendFrame(std::move(frame), responseType);
}

void LinControllerReplay::SendFrame(Frame frame, FrameResponseType responseType, std::chrono::nanoseconds timestamp)
{
    if (tracing::IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Send))
    {
        return;
    }
    _controller.SendFrame(std::move(frame), std::move(responseType), timestamp);
}

void LinControllerReplay::SendFrameHeader(LinIdT linId)
{
    if (tracing::IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Send))
    {
        return;
    }
    _controller.SendFrameHeader(linId);
}

void LinControllerReplay::SendFrameHeader(LinIdT linId, std::chrono::nanoseconds timestamp)
{
    if (tracing::IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Send))
    {
        return;
    }
    _controller.SendFrameHeader(linId, timestamp);
}

void LinControllerReplay::SetFrameResponse(Frame frame, FrameResponseMode mode)
{
    // FrameResponses are not traced/replayed, we rely on the user to make appropriate calls
    //if (tracing::IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Send))
    //{
    //   return;
    //}
    _controller.SetFrameResponse(std::move(frame), std::move(mode));
}

void LinControllerReplay::SetFrameResponses(std::vector<FrameResponse> responses)
{
    _controller.SetFrameResponses(std::move(responses));
}

void LinControllerReplay::GoToSleep()
{
    if (tracing::IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Send) 
        && (_mode != ControllerMode::Master))
    {
        return;
    }
    _controller.GoToSleep();
}

void LinControllerReplay::GoToSleepInternal()
{
    if (tracing::IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Send))
    {
        return;
    }
    _controller.GoToSleepInternal();
}

void LinControllerReplay::Wakeup()
{
    //if (tracing::IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Send))
    //{
    //    return;
    //}
    //_controller.Wakeup();
}

void LinControllerReplay::WakeupInternal()
{
    //if (tracing::IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Send))
    //{
    //    return;
    //}
    _controller.WakeupInternal();
}

void LinControllerReplay::RegisterFrameStatusHandler(FrameStatusHandler handler)
{
    _frameStatusHandler.emplace_back(std::move(handler));
}

void LinControllerReplay::RegisterGoToSleepHandler(GoToSleepHandler handler)
{
    _controller.RegisterGoToSleepHandler(std::move(handler));
}

void LinControllerReplay::RegisterWakeupHandler(WakeupHandler handler)
{
    _controller.RegisterWakeupHandler(std::move(handler));
}

void LinControllerReplay::RegisterFrameResponseUpdateHandler(FrameResponseUpdateHandler handler)
{
    _controller.RegisterFrameResponseUpdateHandler(std::move(handler));
}

void LinControllerReplay::ReceiveIbMessage(ib::mw::EndpointAddress from, const Transmission& msg)
{
    if (tracing::IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Receive))
    {
        return;
    }
    _controller.ReceiveIbMessage(from, msg);
}

void LinControllerReplay::ReceiveIbMessage(ib::mw::EndpointAddress from, const WakeupPulse& msg)
{
    //if (tracing::IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Receive))
    //{
    //    return;
   // }
    _controller.ReceiveIbMessage(from, msg);
}

void LinControllerReplay::ReceiveIbMessage(mw::EndpointAddress from, const ControllerConfig& msg)
{
    //if (tracing::IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Receive))
    //{
    //    return;
    //}
    _controller.ReceiveIbMessage(from, msg);
}

void LinControllerReplay::ReceiveIbMessage(mw::EndpointAddress from, const FrameResponseUpdate& msg)
{
    //if (tracing::IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Receive))
    //{
    //    return;
    //}

    _controller.ReceiveIbMessage(from, msg);
}

void LinControllerReplay::ReceiveIbMessage(mw::EndpointAddress from, const ControllerStatusUpdate& msg)
{
    //if (tracing::IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Receive))
    //{
    //    return;
    //}
    _controller.ReceiveIbMessage(from, msg);
}

void LinControllerReplay::SetEndpointAddress(const ::ib::mw::EndpointAddress& endpointAddress)
{
    _controller.SetEndpointAddress(endpointAddress);
}

auto LinControllerReplay::EndpointAddress() const -> const ::ib::mw::EndpointAddress&
{
    return _controller.EndpointAddress();
}

// ITraceMessageSource
void LinControllerReplay::AddSink(extensions::ITraceMessageSink* sink)
{
    _controller.AddSink(sink);
}

// IReplayDataProvider

void LinControllerReplay::ReplayMessage(const extensions::IReplayMessage* replayMessage)
{
    // XXX Master should be able to replay in both directions? Verify
    using namespace ib::tracing;
    switch (replayMessage->GetDirection())
    {
    case extensions::Direction::Send:
        if (IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Send) 
            || _mode == ControllerMode::Master)
        {
            ReplaySend(replayMessage);
        }
        break;
    case extensions::Direction::Receive:
        if (IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Receive)
            || _mode == ControllerMode::Master)
        {
            ReplayReceive(replayMessage);
        }
        break;
    default:
        throw std::runtime_error("LinControllerReplay: replay message has undefined Direction");
        break;
    }

}

void LinControllerReplay::ReplaySend(const extensions::IReplayMessage* replayMessage)
{
    //The frame response update update's the masters notion of each node's responses.
    // Also triggers a callback
    auto frame = dynamic_cast<const Frame&>(*replayMessage);
    auto mode = (replayMessage->GetDirection() == extensions::Direction::Receive)
        ? FrameResponseMode::Rx
        : FrameResponseMode::TxUnconditional;
    //TODO this is only for triggering FrameResponseUpdate handler callbacks -- diagnostics
    FrameResponse response;
    response.frame = frame;
    response.responseMode = mode;
    FrameResponseUpdate responseUpdate;
    responseUpdate.frameResponses.emplace_back(std::move(response));
    _comAdapter->SendIbMessage(replayMessage->EndpointAddress(), responseUpdate);
    //The actual frame is send via the master, always in RX direction.
    Transmission tm{};
    tm.timestamp = replayMessage->Timestamp();
    tm.frame = std::move(frame);
    tm.status = FrameStatus::LIN_RX_OK;
    _comAdapter->SendIbMessage(replayMessage->EndpointAddress(), tm);

    FrameStatus masterFrameStatus = tm.status;
    if (mode == FrameResponseMode::TxUnconditional)
    {
        masterFrameStatus = FrameStatus::LIN_TX_OK;
    }
    // dispatch local callbacks
    if (replayMessage->EndpointAddress() == _controller.EndpointAddress())
    {
        for (auto& handler : _frameStatusHandler)
        {
            handler(this, tm.frame, masterFrameStatus, tm.timestamp);
        }
    }
}
void LinControllerReplay::ReplayReceive(const extensions::IReplayMessage* replayMessage)
{
    Transmission tm{};
    tm.timestamp = replayMessage->Timestamp();
    auto msg = dynamic_cast<const Frame&>(*replayMessage);
    tm.frame = std::move(msg);
    tm.status = FrameStatus::LIN_RX_OK;
    _controller.ReceiveIbMessage(replayMessage->EndpointAddress(), tm);
    // dispatch local callbacks
    if (replayMessage->EndpointAddress() == _controller.EndpointAddress())
    {
        for (auto& handler : _frameStatusHandler)
        {
            handler(this, tm.frame, tm.status, tm.timestamp);
        }
    }
}

} // namespace lin
} // namespace sim
} // namespace ib
