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

#include "LinControllerReplay.hpp"

#include <iostream>
#include <chrono>

#include "silkit/services/logging/ILogger.hpp"
#include "silkit/services/lin/string_utils.hpp"

namespace {
inline bool IsReplayEnabled(const SilKit::Config::Replay& config)
{
    // Replaying on a master node requires send and receive, implicitly.
    return SilKit::tracing::IsReplayEnabledFor(config, SilKit::Config::Replay::Direction::Send)
        || SilKit::tracing::IsReplayEnabledFor(config, SilKit::Config::Replay::Direction::Receive)
        ;
}
}
namespace SilKit {
namespace Services {
namespace Lin {


LinControllerReplay::LinControllerReplay(Core::IParticipantInternal* participant, Config::LinController config,
            Services::Orchestration::ITimeProvider* timeProvider)
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

void LinControllerReplay::SendFrameHeader(LinId)
{
    // We don't allow mixing user API calls while replaying.
    _participant->GetLogger()->Debug("Replaying: ignoring call to {}.", __FUNCTION__);
    return;
}

void LinControllerReplay::UpdateTxBuffer(LinFrame, LinFrameResponseMode)
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

void LinControllerReplay::AddLinSlaveConfigurationHandler(LinSlaveConfigurationHandler handler)
{
    // FrameResponseUpdates are not part of the replay, we recreate them based on replay data.
    _controller.AddLinSlaveConfigurationHandler(std::move(handler));
}

void LinControllerReplay::ReceiveMsg(const IServiceEndpoint* from, const LinTransmission& msg)
{
    // Transmissions are always issued by a master.
    _controller.ReceiveMsg(from, msg);
}

void LinControllerReplay::ReceiveMsg(const IServiceEndpoint* from, const LinWakeupPulse& msg)
{
    //Wakeup pulses are not part of a replay, but are valid during a replay.
    _controller.ReceiveMsg(from, msg);
}

void LinControllerReplay::ReceiveMsg(const IServiceEndpoint* from, const LinControllerConfig& msg)
{
    // ControllerConfigs are not part of a replay, but are valid during a replay.
    _controller.ReceiveMsg(from, msg);
}

void LinControllerReplay::ReceiveMsg(const IServiceEndpoint* from, const LinControllerStatusUpdate& msg)
{
    // ControllerStatupsUpdates are not part of a replay, but are valid during a replay.
    _controller.ReceiveMsg(from, msg);
}

// ITraceMessageSource
void LinControllerReplay::AddSink(ITraceMessageSink* sink)
{
    // NB: Tracing in _controller is never reached as a Master, because we send with its endpoint address in ReplayMessage
    _controller.AddSink(sink);
    // for active replaying we use our own tracer:
    _tracer.AddSink(SilKit::Core::EndpointAddress{}, *sink);
}

// IReplayDataProvider

void LinControllerReplay::ReplayMessage(const IReplayMessage* replayMessage)
{
    switch (replayMessage->GetDirection())
    {
    case SilKit::Services::TransmitDirection::TX:
    case SilKit::Services::TransmitDirection::RX:
        break;
    default:
        throw std::runtime_error("LinControllerReplay: replay message has undefined Direction");
        break;
    }

    // The LinFrame Response Updates ensures that all controllers have the same notion of the
    // response that is going to be generated by a slave.

    auto frame = dynamic_cast<const LinFrame&>(*replayMessage);
    auto mode = (replayMessage->GetDirection() == SilKit::Services::TransmitDirection::RX)
        ? LinFrameResponseMode::Rx
        : LinFrameResponseMode::TxUnconditional;

    _tracer.Trace(replayMessage->GetDirection(), _timeProvider->Now(), frame);

    LinFrameResponse response;
    response.frame = frame;
    response.responseMode = mode;

    if (_mode == LinControllerMode::Master)
    {
        // When we are a master, also synthesize the frame header (SIL Kit type LinTransmission) based on the replay data.
        // NB: the actual transmission is always in RX-direction, only the callback handlers will see the actual
        //     direction.
        LinTransmission tm{};
        tm.timestamp = replayMessage->Timestamp();
        tm.frame = std::move(frame);
        tm.status = LinFrameStatus::LIN_RX_OK;
        _participant->SendMsg(this, tm);

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

} // namespace Lin
} // namespace Services
} // namespace SilKit
