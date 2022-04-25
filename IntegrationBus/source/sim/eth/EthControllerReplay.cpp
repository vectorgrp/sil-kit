// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "EthControllerReplay.hpp"

namespace ib {
namespace sim {
namespace eth {
void EthControllerReplay::Activate()
{
    _controller.Activate();
}

void EthControllerReplay::Deactivate()
{
    _controller.Deactivate();
}

auto EthControllerReplay::SendFrameEvent(EthernetFrameEvent msg) -> EthernetTxId
{
    // ignore the user's API calls if we're configured for replay
    //if (tracing::IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Send))
    //{
    //    return 0;
    //}

    return _controller.SendFrameEvent(std::move(msg));
}

auto EthControllerReplay::SendFrame(EthernetFrame msg) -> EthernetTxId
{
    // ignore the user's API calls if we're configured for replay
    //if (tracing::IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Send))
    //{
    //    return 0;
    //}

    return _controller.SendFrame(std::move(msg));
}

void EthControllerReplay::AddFrameHandler(FrameHandler handler)
{
    _controller.AddFrameHandler(std::move(handler));
}

void EthControllerReplay::AddFrameTransmitHandler(FrameTransmitHandler handler)
{
    _controller.AddFrameTransmitHandler(std::move(handler));
}
void EthControllerReplay::AddStateChangeHandler(StateChangeHandler handler)
{
    _controller.AddStateChangeHandler(std::move(handler));
}

void EthControllerReplay::AddBitrateChangeHandler(BitrateChangeHandler handler)
{
    _controller.AddBitrateChangeHandler(std::move(handler));
}

// IIbToEthController
void EthControllerReplay::ReceiveIbMessage(const IIbServiceEndpoint* from, const EthernetFrameEvent& msg)
{
    // ignore messages that do not originate from the replay scheduler 
    //if (tracing::IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Receive))
    //{
    //    return;
    //}

    _controller.ReceiveIbMessage(from, msg);
}

// ib::mw::sync::ITimeConsumer
void EthControllerReplay::SetTimeProvider(ib::mw::sync::ITimeProvider* timeProvider)
{
    _controller.SetTimeProvider(timeProvider);
}

// ITraceMessageSource
void EthControllerReplay::AddSink(extensions::ITraceMessageSink* sink)
{
    _controller.AddSink(sink);
}

// IReplayDataProvider

void EthControllerReplay::ReplayMessage(const extensions::IReplayMessage* replayMessage)
{
    using namespace ib::tracing;
    switch (replayMessage->GetDirection())
    {
    case ib::sim::TransmitDirection::TX:
        //if (IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Send))
        //{
        //    ReplaySend(replayMessage);
        //}
        break;
    case ib::sim::TransmitDirection::RX:
        //if (IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Receive))
        //{
        //    ReplayReceive(replayMessage);
        //}
        break;
    default:
        throw std::runtime_error("EthReplayController: replay message has undefined Direction");
        break;
    }

}


void EthControllerReplay::ReplaySend(const extensions::IReplayMessage* replayMessage)
{
    // need to copy the message here.
    // will throw if invalid message type.
    sim::eth::EthernetFrame msg = dynamic_cast<const sim::eth::EthernetFrame&>(*replayMessage);
    _controller.SendFrame(std::move(msg));
}

void EthControllerReplay::ReplayReceive(const extensions::IReplayMessage* replayMessage)
{
    static tracing::ReplayServiceDescriptor replayService;
    sim::eth::EthernetFrame frame = dynamic_cast<const sim::eth::EthernetFrame&>(*replayMessage);
    sim::eth::EthernetFrameEvent msg{};
    msg.ethFrame = std::move(frame);
    msg.timestamp = replayMessage->Timestamp();
    _controller.ReceiveIbMessage(&replayService, msg);
}

} // namespace eth
} // namespace sim
} // namespace ib
