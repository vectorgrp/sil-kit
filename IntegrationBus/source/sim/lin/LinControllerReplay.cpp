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
{
}

void LinControllerReplay::Init(ControllerConfig config)
{
    if (tracing::IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Send))
    {
        return;
    }
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
    if (tracing::IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Send))
    {
        return;
    }
    _controller.SetFrameResponse(std::move(frame), std::move(mode));
}

void LinControllerReplay::SetFrameResponses(std::vector<FrameResponse> responses)
{
    _controller.SetFrameResponses(std::move(responses));
}

void LinControllerReplay::GoToSleep()
{
    if (tracing::IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Send))
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
    if (tracing::IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Send))
    {
        return;
    }
    _controller.Wakeup();
}

void LinControllerReplay::WakeupInternal()
{
    if (tracing::IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Send))
    {
        return;
    }
    _controller.WakeupInternal();
}

void LinControllerReplay::RegisterFrameStatusHandler(FrameStatusHandler handler)
{
    _controller.RegisterFrameStatusHandler(std::move(handler));
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
    if (tracing::IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Receive))
    {
        return;
    }
    _controller.ReceiveIbMessage(from, msg);
}

void LinControllerReplay::ReceiveIbMessage(mw::EndpointAddress from, const ControllerConfig& msg)
{
    if (tracing::IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Receive))
    {
        return;
    }
    _controller.ReceiveIbMessage(from, msg);
}

void LinControllerReplay::ReceiveIbMessage(mw::EndpointAddress from, const FrameResponseUpdate& msg)
{
    if (tracing::IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Receive))
    {
        return;
    }
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
    using namespace ib::tracing;
    switch (replayMessage->GetDirection())
    {
    case extensions::Direction::Send:
        if (IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Send))
        {
            ReplaySend(replayMessage);
        }
        break;
    case extensions::Direction::Receive:
        if (IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Receive))
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
    Transmission tm{};
    tm.timestamp = replayMessage->Timestamp();
    auto msg = dynamic_cast<const Frame&>(*replayMessage);
    tm.frame = std::move(msg);
    //XXX Verify this: Transmission are always sent with RX_xxx, even the one marked as trace direction Send
    tm.status = FrameStatus::LIN_RX_OK;
    _controller.ReceiveIbMessage(replayMessage->EndpointAddress(), std::move(tm));
}
void LinControllerReplay::ReplayReceive(const extensions::IReplayMessage* replayMessage)
{
    Transmission tm{};
    tm.timestamp = replayMessage->Timestamp();
    auto msg = dynamic_cast<const Frame&>(*replayMessage);
    tm.frame = std::move(msg);
    tm.status = FrameStatus::LIN_RX_OK;
    _controller.ReceiveIbMessage(replayMessage->EndpointAddress(), tm);
}

} // namespace lin
} // namespace sim
} // namespace ib
