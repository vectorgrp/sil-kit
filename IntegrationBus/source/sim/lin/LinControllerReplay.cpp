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
    // Replaying
    // we explicitly rely on the Master/Slave controllers properly 
    // initializing as part of the user's application code.
    _mode = config.controllerMode;
    _controller.Init(config);
}

auto LinControllerReplay::Status() const noexcept -> ControllerStatus
{
    return _controller.Status();
}

void LinControllerReplay::SendFrame(Frame frame, FrameResponseType responseType)
{
    // SendFrame is an API only used by a master, we ensure that the API
    // is not called during a replay. That is, we don't support mixing
    // replay data and application data.
    if (tracing::IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Send))
    {
        return;
    }
    _controller.SendFrame(std::move(frame), responseType);
}

void LinControllerReplay::SendFrame(Frame frame, FrameResponseType responseType, std::chrono::nanoseconds timestamp)
{
    // we don't allow mixing user API calls while replaying
    if (tracing::IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Send))
    {
        return;
    }
    _controller.SendFrame(std::move(frame), std::move(responseType), timestamp);
}

void LinControllerReplay::SendFrameHeader(LinIdT linId)
{
    // we don't allow mixing user API calls while replaying
    if (tracing::IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Send))
    {
        return;
    }
    _controller.SendFrameHeader(linId);
}

void LinControllerReplay::SendFrameHeader(LinIdT linId, std::chrono::nanoseconds timestamp)
{
    // we don't allow mixing user API calls while replaying
    if (tracing::IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Send))
    {
        return;
    }
    _controller.SendFrameHeader(linId, timestamp);
}

void LinControllerReplay::SetFrameResponse(Frame frame, FrameResponseMode mode)
{
    // FrameResponses are not part of traced data, they are made by the replaying
    // logic based on frames available from the traced data.
    //if (tracing::IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Send))
    //{
    //   return;
    //}
    _controller.SetFrameResponse(std::move(frame), std::move(mode));
}

void LinControllerReplay::SetFrameResponses(std::vector<FrameResponse> responses)
{
    // FrameResponses are not part of traced data, they are made by the replaying
    // logic based on frames available from the traced data.
    _controller.SetFrameResponses(std::move(responses));
}

void LinControllerReplay::GoToSleep()
{
    // we rely on the master being able to send sleep frames
    if (tracing::IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Send) 
        && (_mode != ControllerMode::Master))
    {
        return;
    }
    _controller.GoToSleep();
}

void LinControllerReplay::GoToSleepInternal()
{
    if (tracing::IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Send)
        && (_mode != ControllerMode::Master))
    {
        return;
    }
    _controller.GoToSleepInternal();
}

void LinControllerReplay::Wakeup()
{
    // Wakeup Pulses are not part of the replay, so we rely on the application's
    // cooperation when waking from sleep.
    _controller.Wakeup();
}

void LinControllerReplay::WakeupInternal()
{
    // Wakeup Pulses are not part of the replay, so we rely on the application's
    // cooperation when waking from sleep.
    _controller.WakeupInternal();
}

void LinControllerReplay::RegisterFrameStatusHandler(FrameStatusHandler handler)
{
    _frameStatusHandler.emplace_back(std::move(handler));
}

void LinControllerReplay::RegisterGoToSleepHandler(GoToSleepHandler handler)
{
    _goToSleepHandler.emplace_back(std::move(handler));
}

void LinControllerReplay::RegisterWakeupHandler(WakeupHandler handler)
{
    // Wakeup Pulses are not part of the replay, so we rely on the application's
    // cooperation when waking from sleep.
    // We handle the callbacks directly.
    _controller.RegisterWakeupHandler(std::move(handler));
}

void LinControllerReplay::RegisterFrameResponseUpdateHandler(FrameResponseUpdateHandler handler)
{
    // FrameResponseUpdates are not part of the replay, we recreate them based on replay data.
    // Thus, we handle the callbacks directly.
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
    //Wakeup pulses are not part of a replay, but are valid during a replay.
    _controller.ReceiveIbMessage(from, msg);
}

void LinControllerReplay::ReceiveIbMessage(mw::EndpointAddress from, const ControllerConfig& msg)
{
    // ControllerConfigs are not part of a replay, but are valid during a replay.
    _controller.ReceiveIbMessage(from, msg);
}

void LinControllerReplay::ReceiveIbMessage(mw::EndpointAddress from, const FrameResponseUpdate& msg)
{
    // FrameResponseUpdates are generated from a master during a replay.
    _controller.ReceiveIbMessage(from, msg);
}

void LinControllerReplay::ReceiveIbMessage(mw::EndpointAddress from, const ControllerStatusUpdate& msg)
{
    // ControllerStatupsUpdates are not part of a replay, but are valid during a replay.
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
    switch (replayMessage->GetDirection())
    {
    case extensions::Direction::Send:
    case extensions::Direction::Receive:
        break;
    default:
        throw std::runtime_error("LinControllerReplay: replay message has undefined Direction");
        break;
    }

    // The Frame Response Updates ensures that all controllers have the same notion of the
    // response that is going to be generated by a slave.

    auto frame = dynamic_cast<const Frame&>(*replayMessage);
    auto mode = (replayMessage->GetDirection() == extensions::Direction::Receive)
        ? FrameResponseMode::Rx
        : FrameResponseMode::TxUnconditional;

    //This response updates also have the side effect of triggering callbacks -- observable behavior of the public API
    FrameResponse response;
    response.frame = frame;
    response.responseMode = mode;
    FrameResponseUpdate responseUpdate;
    responseUpdate.frameResponses.emplace_back(std::move(response));
    _comAdapter->SendIbMessage(replayMessage->EndpointAddress(), responseUpdate);

    if (_mode == ControllerMode::Master)
    {
        //The actual frame is originally sent via the master, always in RX direction.
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
        // dispatch local frame status callbacks
        if (replayMessage->EndpointAddress() == _controller.EndpointAddress())
        {
            for (auto& handler : _frameStatusHandler)
            {
                handler(this, tm.frame, masterFrameStatus, tm.timestamp);
            }
        }
        // dispatch sleep callbacks
        if (tm.frame.id == GoToSleepFrame().id && tm.frame.data == GoToSleepFrame().data)
        {
            for (auto& handler : _goToSleepHandler)
            {
                handler(this);
            }
        }
    }
}

void LinControllerReplay::ReplaySend(const extensions::IReplayMessage* replayMessage)
{
}
void LinControllerReplay::ReplayReceive(const extensions::IReplayMessage* replayMessage)
{
}

} // namespace lin
} // namespace sim
} // namespace ib
