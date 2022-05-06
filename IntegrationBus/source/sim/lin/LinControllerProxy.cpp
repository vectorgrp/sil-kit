// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "LinControllerProxy.hpp"

#include <iostream>
#include <chrono>

#include "ib/mw/logging/ILogger.hpp"
#include "ib/sim/lin/string_utils.hpp"

namespace ib {
namespace sim {
namespace lin {

namespace {
template <class CallbackRangeT, typename... Args>
void CallEach(CallbackRangeT& callbacks, const Args&... args)
{
    for (auto& callback : callbacks)
    {
        callback(args...);
    }
}
} // end anonymous namespace

LinControllerProxy::LinControllerProxy(mw::IParticipantInternal* participant, ILinController* facade)
    : _participant{participant}
    , _logger{participant->GetLogger()}
    , _serviceDescriptor{}
    , _facade{facade}
{
    if (_facade == nullptr)
    {
        _facade = this;
    }
}

void LinControllerProxy::Init(LinControllerConfig config)
{
    _controllerMode = config.controllerMode;
    _controllerStatus = LinControllerStatus::Operational;
    SendIbMessage(config);
}

auto LinControllerProxy::Status() const noexcept -> LinControllerStatus
{
    return _controllerStatus;
}

void LinControllerProxy::SendFrame(LinFrame frame, LinFrameResponseType responseType)
{
    if (_controllerMode != LinControllerMode::Master)
    {
        std::string errorMsg{"LinController::SendFrame() must only be called in master mode!"};
        _logger->Error(errorMsg);
        throw std::runtime_error{errorMsg};
    }

    LinSendFrameRequest sendFrame;
    sendFrame.frame = frame;
    sendFrame.responseType = responseType;
    SendIbMessage(sendFrame);
}

void LinControllerProxy::SendFrameHeader(LinIdT linId)
{
    if (_controllerMode != LinControllerMode::Master)
    {
        std::string errorMsg{"LinController::SendFrameHeader() must only be called in master mode!"};
        _logger->Error(errorMsg);
        throw std::runtime_error{errorMsg};
    }
    LinSendFrameHeaderRequest header;
    header.id = linId;
    SendIbMessage(header);
}

void LinControllerProxy::SetFrameResponse(LinFrame frame, LinFrameResponseMode mode)
{
    LinFrameResponse response;
    response.frame = std::move(frame);
    response.responseMode = mode;

    std::vector<LinFrameResponse> responses{1, response};
    SetFrameResponses(std::move(responses));
}

void LinControllerProxy::SetFrameResponses(std::vector<LinFrameResponse> responses)
{
    LinFrameResponseUpdate frameResponseUpdate;
    frameResponseUpdate.frameResponses = std::move(responses);
    SendIbMessage(frameResponseUpdate);
}

void LinControllerProxy::GoToSleep()
{
    if (_controllerMode != LinControllerMode::Master)
    {
        std::string errorMsg{"LinController::GoToSleep() must not be called for slaves or uninitialized masters!"};
        _logger->Error(errorMsg);
        throw ib::StateError{errorMsg};
    }

    LinSendFrameRequest gotosleepFrame;
    gotosleepFrame.frame = GoToSleepFrame();
    gotosleepFrame.responseType = LinFrameResponseType::MasterResponse;

    SendIbMessage(gotosleepFrame);
   
    // We signal SleepPending to the network simulator, so it will be able
    // to finish sleep frame transmissions before entering Sleep state.
    // cf. AUTOSAR SWS LIN Driver section 7.3.3 [SWS_Lin_00263]
    SetControllerStatus(LinControllerStatus::SleepPending);
    // we don't expose the internal SleepPending state to users
    _controllerStatus = LinControllerStatus::Sleep;
}

void LinControllerProxy::GoToSleepInternal()
{
    SetControllerStatus(LinControllerStatus::Sleep);
}

void LinControllerProxy::Wakeup()
{
    if (_controllerMode == LinControllerMode::Inactive)
    {
        std::string errorMsg{"LinController::Wakeup() must not be called before LinController::Init()"};
        _logger->Error(errorMsg);
        throw ib::StateError{errorMsg};
    }

    // Send without direction, netsim will distribute with correct directions
    LinWakeupPulse pulse;
    SendIbMessage(pulse);
    WakeupInternal();
}

void LinControllerProxy::WakeupInternal()
{
    SetControllerStatus(LinControllerStatus::Operational);
}

void LinControllerProxy::AddFrameStatusHandler(FrameStatusHandler handler)
{
    _frameStatusHandler.emplace_back(std::move(handler));
}

void LinControllerProxy::AddGoToSleepHandler(GoToSleepHandler handler)
{
    _goToSleepHandler.emplace_back(std::move(handler));
}

void LinControllerProxy::AddWakeupHandler(WakeupHandler handler)
{
    _wakeupHandler.emplace_back(std::move(handler));
}

void LinControllerProxy::AddFrameResponseUpdateHandler(FrameResponseUpdateHandler handler)
{
    _frameResponseUpdateHandler.emplace_back(std::move(handler));
}

void LinControllerProxy::ReceiveIbMessage(const IIbServiceEndpoint* from, const LinTransmission& msg)
{
    auto& frame = msg.frame;

    if (frame.dataLength > 8)
    {
        _logger->Warn(
            "LinController received transmission with payload length {} from {{{}, {}}}",
            static_cast<unsigned int>(frame.dataLength),
            from->GetServiceDescriptor().GetParticipantName(),
            from->GetServiceDescriptor().GetServiceName());
        return;
    }

    if (frame.id >= 64)
    {
        _logger->Warn(
            "LinController received transmission with invalid LIN ID {} from {{{}, {}}}",
            frame.id,
            from->GetServiceDescriptor().GetParticipantName(),
            from->GetServiceDescriptor().GetServiceName());
        return;
    }

    if (_controllerMode == LinControllerMode::Inactive)
        _logger->Warn("Inactive LinControllerProxy received a transmission.");

    _tracer.Trace(ib::sim::TransmitDirection::RX,  msg.timestamp, frame);

    // Dispatch frame to handlers
    CallEach(_frameStatusHandler, this, LinFrameStatusEvent{ msg.timestamp, frame, msg.status });

    // Dispatch GoToSleep frames to dedicated handlers
    if (frame.id == GoToSleepFrame().id && frame.data == GoToSleepFrame().data)
    {
        // only call GoToSleepHandlers for slaves, i.e., not for the master that issued the GoToSleep command.
        if (_controllerMode == LinControllerMode::Slave)
        {
            CallEach(_goToSleepHandler, this, LinGoToSleepEvent{ msg.timestamp });
        }
    }
}

void LinControllerProxy::ReceiveIbMessage(const IIbServiceEndpoint* /*from*/, const LinWakeupPulse& msg)
{
    CallEach(_wakeupHandler, this, LinWakeupEvent{ msg.timestamp, msg.direction } );
}

void LinControllerProxy::ReceiveIbMessage(const IIbServiceEndpoint* from, const LinControllerConfig& msg)
{
    // We also receive LinFrameResponseUpdate from other controllers, although we would not need them in VIBE simulation.
    // However, we also want to make users of FrameResponseUpdateHandlers happy when using the VIBE simulation.
    // NOTE: only self-delivered messages are rejected
    if (from->GetServiceDescriptor() == _serviceDescriptor)
        return;

    for (auto& response : msg.frameResponses)
    {
        CallEach(_frameResponseUpdateHandler, this, 
            LinFrameResponseUpdateEvent{ from->GetServiceDescriptor().to_string(), response});
    }
}

void LinControllerProxy::ReceiveIbMessage(const IIbServiceEndpoint* from, const LinFrameResponseUpdate& msg)
{
    // We also receive LinFrameResponseUpdate from other controllers, although we would not need them in VIBE simulation.
    // However, we also want to make users of FrameResponseUpdateHandlers happy when using the VIBE simulation.
    // NOTE: only self-delivered messages are rejected
    if (from->GetServiceDescriptor() == _serviceDescriptor) return;

    for (auto& response : msg.frameResponses)
    {
        CallEach(_frameResponseUpdateHandler, this, 
            LinFrameResponseUpdateEvent{ from->GetServiceDescriptor().to_string(), response});
    }
}

void LinControllerProxy::SetControllerStatus(LinControllerStatus status)
{
    if (_controllerMode == LinControllerMode::Inactive)
    {
        std::string errorMsg{"LinController::Wakeup()/Sleep() must not be called before LinController::Init()"};
        _logger->Error(errorMsg);
        throw ib::StateError{errorMsg};
    }

    if (_controllerStatus == status)
    {
        _logger->Warn("LinController::SetControllerStatus() - controller is already in {} mode.", to_string(status));
    }

    _controllerStatus = status;

    LinControllerStatusUpdate msg;
    msg.status = status;

    SendIbMessage(msg);
}

template <typename MsgT>
void LinControllerProxy::SendIbMessage(MsgT&& msg)
{
    _participant->SendIbMessage(this, std::forward<MsgT>(msg));
}


} // namespace lin
} // namespace sim
} // namespace ib
