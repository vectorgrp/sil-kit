// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "LinControllerReplay.hpp"

#include <iostream>
#include <chrono>

#include "ib/mw/logging/ILogger.hpp"
#include "ib/sim/lin/string_utils.hpp"

namespace {
inline bool IsReplayEnabled(const ib::cfg::Replay& config)
{
    // Replaying on a master node requires send and receive, implicitly.
    return ib::tracing::IsReplayEnabledFor(config, ib::cfg::Replay::Direction::Send)
        || ib::tracing::IsReplayEnabledFor(config, ib::cfg::Replay::Direction::Receive)
        ;
}
}
namespace ib {
namespace sim {
namespace lin {


LinControllerReplay::LinControllerReplay(mw::IParticipantInternal* participant, cfg::LinController config,
            mw::sync::ITimeProvider* timeProvider)
    : _replayConfig{config.replay}
    , _controller{participant, timeProvider}
    , _participant{participant}
    , _timeProvider{timeProvider}
{
}

void LinControllerReplay::Init(LinControllerConfig config)
{
    // Replaying:
    // We explicitly rely on the Master/Slave controllers to properly 
    // initialize as part of the user's application code.

    _mode = config.controllerMode;
    _controller.Init(config);

    // Replaying is only supported on a master node.
    if (_mode == LinControllerMode::Slave && IsReplayEnabled(_replayConfig))
    {
        _participant->GetLogger()->Warn("Replaying on a slave controller is not supported! "
            "Please use tracing and replay on a master controller!");
        throw std::runtime_error("Replaying is not supported on Slave controllers!");
    }
}

auto LinControllerReplay::Status() const noexcept -> LinControllerStatus
{
    return _controller.Status();
}

void LinControllerReplay::SendFrame(LinFrame, LinFrameResponseType)
{
    // SendFrame is an API only used by a master, we ensure that the API
    // is not called during a replay. That is, we don't support mixing
    // replay frames and user-supplied frames.
    _participant->GetLogger()->Debug("Replaying: ignoring call to {}.", __FUNCTION__);
    return;
}

void LinControllerReplay::SendFrameHeader(LinIdT)
{
    // We don't allow mixing user API calls while replaying.
    _participant->GetLogger()->Debug("Replaying: ignoring call to {}.", __FUNCTION__);
    return;
}

void LinControllerReplay::SetFrameResponse(LinFrame, LinFrameResponseMode)
{
    // We don't allow mixing user API calls while replaying.
    _participant->GetLogger()->Debug("Replaying: ignoring call to {}.", __FUNCTION__);
    return;
}

void LinControllerReplay::SetFrameResponses(std::vector<LinFrameResponse>)
{
    // we don't allow mixing user API calls while replaying
    _participant->GetLogger()->Debug("Replaying: ignoring call to {}.", __FUNCTION__);
    return;
}

void LinControllerReplay::GoToSleep()
{
    // We rely on the master being able to send sleep frames.
    _controller.GoToSleep();
}

void LinControllerReplay::GoToSleepInternal()
{
    // We rely on the master being able to send sleep frames.
    _controller.GoToSleepInternal();
}

void LinControllerReplay::Wakeup()
{
    // Wakeup Pulses are not part of the replay, so we rely on the application's
    // cooperation when waking from sleep, i.e. we do allow API calls here!
    _controller.Wakeup();
}

void LinControllerReplay::WakeupInternal()
{
    // Wakeup Pulses are not part of the replay, so we rely on the application's
    // cooperation when waking from sleep.
    _controller.WakeupInternal();
}

void LinControllerReplay::AddFrameStatusHandler(FrameStatusHandler handler)
{
    // LinFrame status callbacks might be triggered from a master node when doing a replay.
    // Thus, we handle them directly.
    _frameStatusHandler.emplace_back(std::move(handler));
}

void LinControllerReplay::AddGoToSleepHandler(GoToSleepHandler handler)
{
    // We call sleep handlers directly, since sleep frames might originate from a replay.
    _goToSleepHandler.emplace_back(std::move(handler));
}

void LinControllerReplay::AddWakeupHandler(WakeupHandler handler)
{
    // Wakeup Pulses are not part of the replay, so we rely on the application's
    // cooperation when waking from sleep.
    _controller.AddWakeupHandler(std::move(handler));
}

void LinControllerReplay::AddFrameResponseUpdateHandler(FrameResponseUpdateHandler handler)
{
    // FrameResponseUpdates are not part of the replay, we recreate them based on replay data.
    _controller.AddFrameResponseUpdateHandler(std::move(handler));
}

void LinControllerReplay::ReceiveIbMessage(const IIbServiceEndpoint* from, const LinTransmission& msg)
{
    // Transmissions are always issued by a master.
    _controller.ReceiveIbMessage(from, msg);
}

void LinControllerReplay::ReceiveIbMessage(const IIbServiceEndpoint* from, const LinWakeupPulse& msg)
{
    //Wakeup pulses are not part of a replay, but are valid during a replay.
    _controller.ReceiveIbMessage(from, msg);
}

void LinControllerReplay::ReceiveIbMessage(const IIbServiceEndpoint* from, const LinControllerConfig& msg)
{
    // ControllerConfigs are not part of a replay, but are valid during a replay.
    _controller.ReceiveIbMessage(from, msg);
}

void LinControllerReplay::ReceiveIbMessage(const IIbServiceEndpoint* from, const LinFrameResponseUpdate& msg)
{
    // FrameResponseUpdates are generated from a master during a replay.
    _controller.ReceiveIbMessage(from, msg);
}

void LinControllerReplay::ReceiveIbMessage(const IIbServiceEndpoint* from, const LinControllerStatusUpdate& msg)
{
    // ControllerStatupsUpdates are not part of a replay, but are valid during a replay.
    _controller.ReceiveIbMessage(from, msg);
}

// ITraceMessageSource
void LinControllerReplay::AddSink(extensions::ITraceMessageSink* sink)
{
    // NB: Tracing in _controller is never reached as a Master, because we send with its endpoint address in ReplayMessage
    _controller.AddSink(sink);
    // for active replaying we use our own tracer:
    _tracer.AddSink(ib::mw::EndpointAddress{}, *sink);
}

// IReplayDataProvider

void LinControllerReplay::ReplayMessage(const extensions::IReplayMessage* replayMessage)
{
    switch (replayMessage->GetDirection())
    {
    case ib::sim::TransmitDirection::TX:
    case ib::sim::TransmitDirection::RX:
        break;
    default:
        throw std::runtime_error("LinControllerReplay: replay message has undefined Direction");
        break;
    }

    // The LinFrame Response Updates ensures that all controllers have the same notion of the
    // response that is going to be generated by a slave.

    auto frame = dynamic_cast<const LinFrame&>(*replayMessage);
    auto mode = (replayMessage->GetDirection() == ib::sim::TransmitDirection::RX)
        ? LinFrameResponseMode::Rx
        : LinFrameResponseMode::TxUnconditional;

    _tracer.Trace(replayMessage->GetDirection(), _timeProvider->Now(), frame);

    LinFrameResponse response;
    response.frame = frame;
    response.responseMode = mode;
    LinFrameResponseUpdate responseUpdate;
    responseUpdate.frameResponses.emplace_back(std::move(response));
    _participant->SendIbMessage(this, responseUpdate);

    if (_mode == LinControllerMode::Master)
    {
        // When we are a master, also synthesize the frame header (IB type LinTransmission) based on the replay data.
        // NB: the actual transmission is always in RX-direction, only the callback handlers will see the actual
        //     direction.
        LinTransmission tm{};
        tm.timestamp = replayMessage->Timestamp();
        tm.frame = std::move(frame);
        tm.status = LinFrameStatus::LIN_RX_OK;
        _participant->SendIbMessage(this, tm);

        // dispatch local frame status handlers
        //LinFrameStatus masterFrameStatus = tm.status;
        //if (mode == LinFrameResponseMode::TxUnconditional)
        //{
        //    masterFrameStatus = LinFrameStatus::LIN_TX_OK;
        //}
        //if (replayMessage->EndpointAddress() == _controller.EndpointAddress())
        //{
        //    for (auto& handler : _frameStatusHandler)
        //    {
        //        handler(this, tm.frame, masterFrameStatus, tm.timestamp);
        //    }
        //}
        // dispatch sleep handler
        if (tm.frame.id == GoToSleepFrame().id && tm.frame.data == GoToSleepFrame().data)
        {
            for (auto& handler : _goToSleepHandler)
            {
                handler(this, LinGoToSleepEvent{ tm.timestamp });
            }
        }
    }
}

} // namespace lin
} // namespace sim
} // namespace ib
