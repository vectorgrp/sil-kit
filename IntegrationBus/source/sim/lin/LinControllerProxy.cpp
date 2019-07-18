// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "LinControllerProxy.hpp"

#include <iostream>

#include "ib/mw/IComAdapter.hpp"
#include "ib/mw/logging/spdlog.hpp"
#include "ib/sim/lin/string_utils.hpp"

namespace ib {
namespace sim {
namespace lin {

namespace {
inline bool AreMatchingProxyEndpoints(mw::EndpointAddress lhs, mw::EndpointAddress rhs)
{
    return lhs.participant != rhs.participant
        && lhs.endpoint == rhs.endpoint;
}
template <class CallbackRangeT, typename... Args>
void CallEach(CallbackRangeT& callbacks, const Args&... args)
{
    for (auto& callback : callbacks)
    {
        callback(args...);
    }
}
}

LinControllerProxy::LinControllerProxy(mw::IComAdapter* comAdapter)
    : _comAdapter{comAdapter}
    , _logger{comAdapter->GetLogger()}
{
}

void LinControllerProxy::Init(ControllerConfig config)
{
    _controllerMode = config.controllerMode;
    _controllerStatus = ControllerStatus::Operational;
    SendIbMessage(config);
}

auto LinControllerProxy::Status() const noexcept -> ControllerStatus
{
    return _controllerStatus;
}

void LinControllerProxy::SendFrame(Frame frame, FrameResponseType responseType)
{
    if (_controllerMode != ControllerMode::Master)
    {
        std::string errorMsg{"LinController::SendFrame() must only be called in master mode!"};
        _logger->error(errorMsg);
        throw std::runtime_error{errorMsg};
    }
    SendFrameRequest sendFrame;
    sendFrame.frame = frame;
    sendFrame.responseType = responseType;
    SendIbMessage(sendFrame);
}

void LinControllerProxy::SendFrameHeader(LinIdT linId)
{
    if (_controllerMode != ControllerMode::Master)
    {
        std::string errorMsg{"LinController::SendFrameHeader() must only be called in master mode!"};
        _logger->error(errorMsg);
        throw std::runtime_error{errorMsg};
    }
    SendFrameHeaderRequest header;
    header.id = linId;
    SendIbMessage(header);
}

void LinControllerProxy::SetFrameResponse(Frame frame, FrameResponseMode mode)
{
    FrameResponse response;
    response.frame = std::move(frame);
    response.responseMode = mode;

    std::vector<FrameResponse> responses{1, response};
    SetFrameResponses(std::move(responses));
}

void LinControllerProxy::SetFrameResponses(std::vector<FrameResponse> responses)
{
    FrameResponseUpdate frameResponseUpdate;
    frameResponseUpdate.frameResponses = std::move(responses);
    SendIbMessage(frameResponseUpdate);
}

void LinControllerProxy::GoToSleep()
{
    if (_controllerMode != ControllerMode::Master)
    {
        std::string errorMsg{"LinController::GoToSleep() must only be called in master mode!"};
        _logger->error(errorMsg);
        throw std::logic_error{errorMsg};
    }

    SendFrameRequest gotosleepFrame;
    gotosleepFrame.frame = GoToSleepFrame();
    gotosleepFrame.responseType = FrameResponseType::MasterResponse;

    SendIbMessage(gotosleepFrame);
    GoToSleepInternal();
}

void LinControllerProxy::GoToSleepInternal()
{
    SetControllerStatus(ControllerStatus::Sleep);
}

void LinControllerProxy::Wakeup()
{
    WakeupPulse pulse;
    SendIbMessage(pulse);
    WakeupInternal();
}

void LinControllerProxy::WakeupInternal()
{
    SetControllerStatus(ControllerStatus::Operational);
}

void LinControllerProxy::RegisterFrameStatusHandler(FrameStatusHandler handler)
{
    _frameStatusHandler.emplace_back(std::move(handler));
}

void LinControllerProxy::RegisterGoToSleepHandler(GoToSleepHandler handler)
{
    _goToSleepHandler.emplace_back(std::move(handler));
}

void LinControllerProxy::RegisterWakeupHandler(WakeupHandler handler)
{
    _wakeupHandler.emplace_back(std::move(handler));
}

void LinControllerProxy::RegisterFrameResponseUpdateHandler(FrameResponseUpdateHandler handler)
{
    _frameResponseUpdateHandler.emplace_back(std::move(handler));
}

void LinControllerProxy::ReceiveIbMessage(ib::mw::EndpointAddress from, const Transmission& msg)
{
    if (!AreMatchingProxyEndpoints(from, _endpointAddr)) return;

    auto& frame = msg.frame;

    if (frame.dataLength > 8)
    {
        _logger->warn(
            "LinController received transmission with payload length {} from {{{}, {}}}",
            static_cast<unsigned int>(frame.dataLength),
            from.participant,
            from.endpoint);
        return;
    }

    if (frame.id >= 64)
    {
        _logger->warn(
            "LinController received transmission with invalid LIN ID {} from {{{}, {}}}",
            frame.id,
            from.participant,
            from.endpoint);
        return;
    }

    if (_controllerMode == ControllerMode::Inactive)
        _logger->warn("Inactive LinControllerProxy received a transmission.");

    // Dispatch frame to handlers
    CallEach(_frameStatusHandler, this, frame, msg.status, msg.timestamp);

    // Dispatch GoToSleep frames to dedicated handlers
    if (frame.id == GoToSleepFrame().id)
    {
        if (frame.data != GoToSleepFrame().data)
        {
            _logger->warn("LinController received diagnostic frame, which does not match expected GoToSleep payload");
        }

        // only call GoToSleepHandlers for slaves, i.e., not for the master that issued the GoToSleep command.
        if (_controllerMode == ControllerMode::Slave)
            CallEach(_goToSleepHandler, this);
    }
}

void LinControllerProxy::ReceiveIbMessage(ib::mw::EndpointAddress from, const WakeupPulse& /*msg*/)
{
    if (!AreMatchingProxyEndpoints(from, _endpointAddr)) return;
    CallEach(_wakeupHandler, this);
}

void LinControllerProxy::ReceiveIbMessage(mw::EndpointAddress from, const ControllerConfig& msg)
{
    // We also receive FrameResponseUpdate from other controllers, although we would not need them in VIBE simulation.
    // However, we also want to make users of FrameResponseUpdateHandlers happy when using the VIBE simulation.
    if (from == _endpointAddr) return;

    for (auto& response : msg.frameResponses)
    {
        CallEach(_frameResponseUpdateHandler, this, from, response);
    }
}

void LinControllerProxy::ReceiveIbMessage(mw::EndpointAddress from, const FrameResponseUpdate& msg)
{
    // We also receive FrameResponseUpdate from other controllers, although we would not need them in VIBE simulation.
    // However, we also want to make users of FrameResponseUpdateHandlers happy when using the VIBE simulation.
    if (from == _endpointAddr) return;

    for (auto& response : msg.frameResponses)
    {
        CallEach(_frameResponseUpdateHandler, this, from, response);
    }
}

void LinControllerProxy::SetEndpointAddress(const ::ib::mw::EndpointAddress& endpointAddress)
{
    _endpointAddr = endpointAddress;
}

auto LinControllerProxy::EndpointAddress() const -> const ::ib::mw::EndpointAddress&
{
    return _endpointAddr;
}

void LinControllerProxy::SetControllerStatus(ControllerStatus status)
{
    if (_controllerMode == ControllerMode::Inactive)
    {
        std::string errorMsg{"LinController::Wakeup()/Sleep() must not be called before LinController::Init()"};
        _logger->error(errorMsg);
        throw std::runtime_error{errorMsg};
    }

    if (_controllerStatus == status)
    {
        spdlog::warn("LinController::SetControllerStatus() - controller is already in {} mode.", to_string(status));
    }

    _controllerStatus = status;

    ControllerStatusUpdate msg;
    msg.status = status;

    SendIbMessage(msg);
}

template <typename MsgT>
void LinControllerProxy::SendIbMessage(MsgT&& msg)
{
    _comAdapter->SendIbMessage(_endpointAddr, std::forward<MsgT>(msg));
}


} // namespace lin
} // namespace sim
} // namespace ib
