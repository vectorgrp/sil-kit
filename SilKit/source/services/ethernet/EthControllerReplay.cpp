// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "EthControllerReplay.hpp"

namespace SilKit {
namespace Services {
namespace Ethernet {
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
    //if (tracing::IsReplayEnabledFor(_replayConfig, Config::Replay::Direction::Send))
    //{
    //    return 0;
    //}

    return _controller.SendFrameEvent(std::move(msg));
}

auto EthControllerReplay::SendFrame(EthernetFrame msg) -> EthernetTxId
{
    // ignore the user's API calls if we're configured for replay
    //if (tracing::IsReplayEnabledFor(_replayConfig, Config::Replay::Direction::Send))
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

// IMsgForEthController
void EthControllerReplay::ReceiveMsg(const IServiceEndpoint* from, const EthernetFrameEvent& msg)
{
    // ignore messages that do not originate from the replay scheduler 
    //if (tracing::IsReplayEnabledFor(_replayConfig, Config::Replay::Direction::Receive))
    //{
    //    return;
    //}

    _controller.ReceiveMsg(from, msg);
}

void EthControllerReplay::ReceiveMsg(const IServiceEndpoint* /*from*/, const EthernetFrameTransmitEvent& /*msg*/)
{
}

void EthControllerReplay::ReceiveMsg(const IServiceEndpoint* /*from*/, const EthernetStatus& /*msg*/)
{
}

// SilKit::Services::Orchestration::ITimeConsumer
void EthControllerReplay::SetTimeProvider(SilKit::Services::Orchestration::ITimeProvider* /*timeProvider*/)
{
    //_controller.SetTimeProvider(timeProvider);
}

// ITraceMessageSource
void EthControllerReplay::AddSink(ITraceMessageSink* sink)
{
    _controller.AddSink(sink);
}

// IReplayDataProvider

void EthControllerReplay::ReplayMessage(const IReplayMessage* replayMessage)
{
    using namespace SilKit::tracing;
    switch (replayMessage->GetDirection())
    {
    case SilKit::Services::TransmitDirection::TX:
        //if (IsReplayEnabledFor(_replayConfig, Config::Replay::Direction::Send))
        //{
        //    ReplaySend(replayMessage);
        //}
        break;
    case SilKit::Services::TransmitDirection::RX:
        //if (IsReplayEnabledFor(_replayConfig, Config::Replay::Direction::Receive))
        //{
        //    ReplayReceive(replayMessage);
        //}
        break;
    default:
        throw std::runtime_error("EthReplayController: replay message has undefined Direction");
        break;
    }

}


void EthControllerReplay::ReplaySend(const IReplayMessage* replayMessage)
{
    // need to copy the message here.
    // will throw if invalid message type.
    Services::Ethernet::EthernetFrame msg = dynamic_cast<const Services::Ethernet::EthernetFrame&>(*replayMessage);
    _controller.SendFrame(std::move(msg));
}

void EthControllerReplay::ReplayReceive(const IReplayMessage* replayMessage)
{
    static tracing::ReplayServiceDescriptor replayService;
    Services::Ethernet::EthernetFrame frame = dynamic_cast<const Services::Ethernet::EthernetFrame&>(*replayMessage);
    Services::Ethernet::EthernetFrameEvent msg{};
    msg.frame = std::move(frame);
    msg.timestamp = replayMessage->Timestamp();
    _controller.ReceiveMsg(&replayService, msg);
}

} // namespace Ethernet
} // namespace Services
} // namespace SilKit
