// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "CanControllerReplay.hpp"

namespace ib {
namespace sim {
namespace can {


CanControllerReplay::CanControllerReplay(mw::IComAdapter* comAdapter, cfg::CanController config, mw::sync::ITimeProvider* timeProvider)
    : _controller{comAdapter, timeProvider}
    , _replayConfig{config.replay}
{
}

void CanControllerReplay::SetBaudRate(uint32_t rate, uint32_t fdRate)
{
    _controller.SetBaudRate(rate, fdRate);
}

void CanControllerReplay::Reset()
{
    _controller.Reset();
}

void CanControllerReplay::Start()
{
    _controller.Start();
}

void CanControllerReplay::Stop()
{
    _controller.Stop();
}

void CanControllerReplay::Sleep()
{
    _controller.Sleep();
}

auto CanControllerReplay::SendMessage(const CanMessage& msg) -> CanTxId
{
    // ignore the user's API calls if we're configured for replay
    if (tracing::IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Send))
    {
        return 0;
    }
    return _controller.SendMessage(msg);
}

auto CanControllerReplay::SendMessage(CanMessage&& msg) -> CanTxId
{
    // ignore the user's API calls if we're configured for replay
    if (tracing::IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Send))
    {
        return 0;
    }
    return _controller.SendMessage(std::forward<CanMessage>(msg));
}

void CanControllerReplay::RegisterReceiveMessageHandler(ReceiveMessageHandler handler)
{
    _controller.RegisterReceiveMessageHandler(std::move(handler));
}

void CanControllerReplay::RegisterStateChangedHandler(StateChangedHandler handler)
{
    _controller.RegisterStateChangedHandler(std::move(handler));
}

void CanControllerReplay::RegisterErrorStateChangedHandler(ErrorStateChangedHandler handler)
{
    _controller.RegisterErrorStateChangedHandler(std::move(handler));
}

void CanControllerReplay::RegisterTransmitStatusHandler(MessageStatusHandler handler)
{
    _controller.RegisterTransmitStatusHandler(std::move(handler));
}

void CanControllerReplay::ReceiveIbMessage(ib::mw::EndpointAddress from, const CanMessage& msg)
{
    // ignore messages that do not originate from the replay scheduler 
    if (tracing::IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Receive))
    {
        return;
    }

    _controller.ReceiveIbMessage(from, msg);
}



void CanControllerReplay::SetEndpointAddress(const ::ib::mw::EndpointAddress& endpointAddress)
{
    _controller.SetEndpointAddress(endpointAddress);
}

auto CanControllerReplay::EndpointAddress() const -> const ::ib::mw::EndpointAddress&
{
    return _controller.EndpointAddress();
}

void CanControllerReplay::SetTimeProvider(ib::mw::sync::ITimeProvider* timeProvider)
{
    _controller.SetTimeProvider(timeProvider);
}

void CanControllerReplay::AddSink(extensions::ITraceMessageSink* sink)
{
    _controller.AddSink(sink);
}

void CanControllerReplay::ReplayMessage(const extensions::IReplayMessage* replayMessage)
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
        throw std::runtime_error("CanReplayController: replay message has undefined Direction");
        break;
    }

}
void CanControllerReplay::ReplaySend(const extensions::IReplayMessage* replayMessage)
{
    // need to copy the message here.
    // will throw if invalid message type.
    auto msg = dynamic_cast<const sim::can::CanMessage&>(*replayMessage);
    _controller.SendMessage(std::move(msg));
}

void CanControllerReplay::ReplayReceive(const extensions::IReplayMessage* replayMessage)
{
    auto msg = dynamic_cast<const sim::can::CanMessage&>(*replayMessage);
    _controller.ReceiveIbMessage(tracing::ReplayEndpointAddress(), msg);
}

} // namespace can
} // namespace sim
} // namespace ib
