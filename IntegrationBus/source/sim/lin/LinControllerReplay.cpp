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
    : _replayConfig{config}
    , _controller{comAdapter, timeProvider}
{
}

void LinControllerReplay::Init(ControllerConfig config)
{
    _controller.Init(config);
}

auto LinControllerReplay::Status() const noexcept -> ControllerStatus
{
    return _controller.Status();
}

void LinControllerReplay::SendFrame(Frame frame, FrameResponseType responseType)
{
    _controller.SendFrame(std::move(frame), responseType);
}

void LinControllerReplay::SendFrame(Frame frame, FrameResponseType responseType, std::chrono::nanoseconds timestamp)
{
    _controller.SendFrame(std::move(frame), std::move(responseType), timestamp);
}

void LinControllerReplay::SendFrameHeader(LinIdT linId)
{
    _controller.SendFrameHeader(linId);
}

void LinControllerReplay::SendFrameHeader(LinIdT linId, std::chrono::nanoseconds timestamp)
{
    _controller.SendFrameHeader(linId, timestamp);
}

void LinControllerReplay::SetFrameResponse(Frame frame, FrameResponseMode mode)
{
    _controller.SetFrameResponse(std::move(frame), std::move(mode));
}

void LinControllerReplay::SetFrameResponses(std::vector<FrameResponse> responses)
{
    _controller.SetFrameResponses(std::move(responses));
}

void LinControllerReplay::GoToSleep()
{
    _controller.GoToSleep();
}

void LinControllerReplay::GoToSleepInternal()
{
    _controller.GoToSleepInternal();
}

void LinControllerReplay::Wakeup()
{
    _controller.Wakeup();
}

void LinControllerReplay::WakeupInternal()
{
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
    _controller.ReceiveIbMessage(from, msg);
}

void LinControllerReplay::ReceiveIbMessage(ib::mw::EndpointAddress from, const WakeupPulse& msg)
{
    _controller.ReceiveIbMessage(from, msg);
}

void LinControllerReplay::ReceiveIbMessage(mw::EndpointAddress from, const ControllerConfig& msg)
{
    _controller.ReceiveIbMessage(from, msg);
}

void LinControllerReplay::ReceiveIbMessage(mw::EndpointAddress from, const FrameResponseUpdate& msg)
{
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


} // namespace lin
} // namespace sim
} // namespace ib
