// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "LinController.hpp"

#include <cassert>
#include <algorithm>
#include <iostream>

#include "ib/mw/logging/ILogger.hpp"
#include "ib/sim/lin/string_utils.hpp"

namespace ib {
namespace sim {
namespace lin {

namespace {

inline auto ToFrameResponseMode(LinFrameResponseType responseType) -> LinFrameResponseMode
{
    switch (responseType) {
    case LinFrameResponseType::MasterResponse:
        return LinFrameResponseMode::TxUnconditional;
    case LinFrameResponseType::SlaveResponse:
        return LinFrameResponseMode::Rx;
    case LinFrameResponseType::SlaveToSlave:
        return LinFrameResponseMode::Unused;
    }
    return LinFrameResponseMode::Unused;
}

inline auto ToTxFrameStatus(LinFrameStatus status) -> LinFrameStatus
{
    switch (status)
    {
    case LinFrameStatus::LIN_RX_BUSY:
        return LinFrameStatus::LIN_TX_BUSY;
    case LinFrameStatus::LIN_RX_ERROR:
        return LinFrameStatus::LIN_TX_ERROR;
    case LinFrameStatus::LIN_RX_NO_RESPONSE:
        return LinFrameStatus::LIN_RX_NO_RESPONSE;
    case LinFrameStatus::LIN_RX_OK:
        return LinFrameStatus::LIN_TX_OK;
    default:
        return status;
    }
}

inline auto ToTracingDir(LinFrameStatus status) -> ib::sim::TransmitDirection
{
    switch (status)
    {
    case LinFrameStatus::LIN_RX_ERROR: //[[fallthrough]]
    case LinFrameStatus::LIN_RX_BUSY: //[[fallthrough]]
    case LinFrameStatus::LIN_RX_NO_RESPONSE: //[[fallthrough]]
    case LinFrameStatus::LIN_RX_OK: 
        return ib::sim::TransmitDirection::RX;
    case LinFrameStatus::LIN_TX_ERROR: //[[fallthrough]]
    case LinFrameStatus::LIN_TX_BUSY: //[[fallthrough]]
    case LinFrameStatus::LIN_TX_HEADER_ERROR: //[[fallthrough]]
    case LinFrameStatus::LIN_TX_OK: 
        return ib::sim::TransmitDirection::TX;
    default:
        //if invalid status given, failsafe to send.
        return ib::sim::TransmitDirection::TX;
    }
}
template <class CallbackRangeT, typename... Args>
void CallHandlers(CallbackRangeT& callbacks, const Args&... args)
{
    for (auto& callback : callbacks)
    {
        callback(args...);
    }
}

} // namespace anonymous

LinController::LinController(mw::IParticipantInternal* participant, mw::sync::ITimeProvider* timeProvider, ILinController* facade)
    : _participant{participant}
    , _logger{participant->GetLogger()}
    , _timeProvider{timeProvider}
    , _facade{facade}
{
    if (_facade == nullptr)
    {
        _facade = this;
    }
}

void LinController::Init(LinControllerConfig config)
{
    auto& node = GetLinNode(_serviceDescriptor.to_endpointAddress());
    node.controllerMode = config.controllerMode;
    node.controllerStatus = LinControllerStatus::Operational;
    node.UpdateResponses(config.frameResponses, _logger);

    _controllerMode = config.controllerMode;
    _controllerStatus = LinControllerStatus::Operational;
    SendIbMessage(config);
}

auto LinController::Status() const noexcept -> LinControllerStatus
{
    return _controllerStatus;
}

void LinController::SendFrame(LinFrame frame, LinFrameResponseType responseType)
{
    SetFrameResponse(frame, ToFrameResponseMode(responseType));
    SendFrameHeader(frame.id);
}

void LinController::SendFrameHeader(LinIdT linId)
{
    if (_controllerMode != LinControllerMode::Master)
    {
        std::string errorMsg{"LinController::SendFrameHeader() must only be called in master mode!"};
        _logger->Error(errorMsg);
        throw std::runtime_error{errorMsg};
    }

    // we answer the call immediately based on the cached responses
    // setup a reply
    LinTransmission transmission;
    transmission.frame.id = linId;
    transmission.timestamp = _timeProvider->Now();

    auto numResponses = 0;
    for (auto&& node : _linNodes)
    {
        if (node.controllerMode == LinControllerMode::Inactive)
            continue;
        if (node.controllerStatus != LinControllerStatus::Operational)
            continue;

        auto& response = node.responses[linId];
        if (response.responseMode == LinFrameResponseMode::TxUnconditional)
        {
            transmission.frame = response.frame;
            numResponses++;
        }
    }

    if (numResponses == 0)
    {
        transmission.status = LinFrameStatus::LIN_RX_NO_RESPONSE;
    }
    else if (numResponses == 1)
    {
        transmission.status = LinFrameStatus::LIN_RX_OK;
    }
    else if (numResponses > 1)
    {
        transmission.status = LinFrameStatus::LIN_RX_ERROR;
    }

    // Dispatch the LIN transmission to all connected nodes
    SendIbMessage(transmission);

    // Dispatch the LIN transmission to our own callbacks
    LinFrameResponseMode masterResponseMode =
        GetLinNode(_serviceDescriptor.to_endpointAddress()).responses[linId].responseMode;
    LinFrameStatus masterFrameStatus = transmission.status;
    if (masterResponseMode == LinFrameResponseMode::TxUnconditional)
    {
        masterFrameStatus = ToTxFrameStatus(masterFrameStatus);
    }

    _tracer.Trace(ToTracingDir(masterFrameStatus), transmission.timestamp, transmission.frame);

    // dispatch the reply locally...
    CallHandlers(_frameStatusHandler, this,
                 LinFrameStatusEvent{transmission.timestamp, transmission.frame, masterFrameStatus});

}

void LinController::SetFrameResponse(LinFrame frame, LinFrameResponseMode mode)
{
    LinFrameResponse response;
    response.frame = std::move(frame);
    response.responseMode = mode;

    std::vector<LinFrameResponse> responses{1, response};
    SetFrameResponses(std::move(responses));
}

void LinController::SetFrameResponses(std::vector<LinFrameResponse> responses)
{
    auto& node = GetLinNode(_serviceDescriptor.to_endpointAddress());
    node.UpdateResponses(responses, _logger);

    LinFrameResponseUpdate frameResponseUpdate;
    frameResponseUpdate.frameResponses = std::move(responses);
    SendIbMessage(frameResponseUpdate);
}

void LinController::GoToSleep()
{
    if (_controllerMode != LinControllerMode::Master)
    {
        std::string errorMsg{"LinController::GoToSleep() must only be called in master mode!"};
        _logger->Error(errorMsg);
        throw ib::StateError{errorMsg};
    }

    LinTransmission gotosleepTx;
    gotosleepTx.frame = GoToSleepFrame();
    gotosleepTx.status = LinFrameStatus::LIN_RX_OK;

    SendIbMessage(gotosleepTx);

    // For trivial simulations we go directly to Sleep state.
    GoToSleepInternal();
}

void LinController::GoToSleepInternal()
{
    SetControllerStatus(LinControllerStatus::Sleep);
}

void LinController::Wakeup()
{
    if (_controllerMode == LinControllerMode::Inactive)
    {
        std::string errorMsg{"LinController::Wakeup() must not be called before LinController::Init()"};
        _logger->Error(errorMsg);
        throw ib::StateError{errorMsg};
    }

    // Send to others with direction=RX
    LinWakeupPulse pulse{ _timeProvider->Now(), TransmitDirection::RX }; 
    SendIbMessage(pulse);
    // No self delivery: directly call handlers with direction=TX
    CallHandlers(_wakeupHandler, this, LinWakeupEvent{ pulse.timestamp, TransmitDirection::TX});
    WakeupInternal();
}

void LinController::WakeupInternal()
{
    SetControllerStatus(LinControllerStatus::Operational);
}

void LinController::AddFrameStatusHandler(FrameStatusHandler handler)
{
    _frameStatusHandler.emplace_back(std::move(handler));
}

void LinController::AddGoToSleepHandler(GoToSleepHandler handler)
{
    _goToSleepHandler.emplace_back(std::move(handler));
}

void LinController::AddWakeupHandler(WakeupHandler handler)
{
    _wakeupHandler.emplace_back(std::move(handler));
}

void LinController::AddFrameResponseUpdateHandler(FrameResponseUpdateHandler handler)
{
    _frameResponseUpdateHandler.emplace_back(std::move(handler));
}

void LinController::ReceiveIbMessage(const IIbServiceEndpoint* from, const LinTransmission& msg)
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

    if (_controllerStatus != LinControllerStatus::Operational)
    {
        _logger->Warn(
            "LinController received transmission with LIN ID {} while controller is in {} mode. Message is ignored.",
            static_cast<unsigned int>(frame.id),
            to_string(_controllerStatus)
        );
        return;
    }

    switch (_controllerMode)
    {
    case LinControllerMode::Inactive:
        return;

    case LinControllerMode::Master:
        _logger->Warn("LinController in MasterMode received a transmission from {{{}, {}}}. This indicates an erroneous setup as there should be only one LIN master!",
            from->GetServiceDescriptor().GetParticipantName(),
            from->GetServiceDescriptor().GetServiceName());
        //[[fallthrough]]

    case LinControllerMode::Slave:

        auto& thisLinNode = GetLinNode(_serviceDescriptor.to_endpointAddress());
        switch (thisLinNode.responses[frame.id].responseMode)
        {
        case LinFrameResponseMode::Unused:
            break;
        case LinFrameResponseMode::Rx:
        {
            const auto msgStatus = VeriyChecksum(frame, msg.status);
            if (msgStatus == LinFrameStatus::LIN_RX_OK)
            {
                _tracer.Trace(ib::sim::TransmitDirection::RX, _timeProvider->Now(), frame);
            }

            CallHandlers(_frameStatusHandler, this, LinFrameStatusEvent{ msg.timestamp, frame, msgStatus });
            break;
        }
        case LinFrameResponseMode::TxUnconditional:
            // Transmissions are always sent with LinFrameStatus::RX_xxx so we have to
            // convert the status to a TX_xxx if we sent this frame.
            if (msg.status == LinFrameStatus::LIN_RX_OK)
            {
                _tracer.Trace(ib::sim::TransmitDirection::TX, _timeProvider->Now(), frame);
            }
            CallHandlers(_frameStatusHandler, this,
                         LinFrameStatusEvent{msg.timestamp, frame, ToTxFrameStatus(msg.status)});
            break;
        }

        // Dispatch GoToSleep frames
        if (frame.id == GoToSleepFrame().id && frame.data == GoToSleepFrame().data)
        {
            _tracer.Trace(ToTracingDir(msg.status), _timeProvider->Now(), frame);
            CallHandlers(_goToSleepHandler, this, LinGoToSleepEvent{ _timeProvider->Now() });
        }
    }
}

void LinController::ReceiveIbMessage(const IIbServiceEndpoint* /*from*/, const LinWakeupPulse& msg)
{
    CallHandlers(_wakeupHandler, this, LinWakeupEvent{ msg.timestamp, msg.direction});
}

void LinController::ReceiveIbMessage(const IIbServiceEndpoint* from, const LinControllerConfig& msg)
{
    auto& linNode = GetLinNode(from->GetServiceDescriptor().to_endpointAddress());

    linNode.controllerMode = msg.controllerMode;
    linNode.controllerStatus = LinControllerStatus::Operational;
    linNode.UpdateResponses(msg.frameResponses, _logger);

    for (auto& response : msg.frameResponses)
    {
        CallHandlers(
            _frameResponseUpdateHandler, this,
            LinFrameResponseUpdateEvent{ from->GetServiceDescriptor().to_string(), response});
    }
}

void LinController::ReceiveIbMessage(const IIbServiceEndpoint* from, const LinControllerStatusUpdate& msg)
{
    auto& linNode = GetLinNode(from->GetServiceDescriptor().to_endpointAddress());
    linNode.controllerStatus = msg.status;
}

void LinController::ReceiveIbMessage(const IIbServiceEndpoint* from, const LinFrameResponseUpdate& msg)
{
    auto& linNode = GetLinNode(from->GetServiceDescriptor().to_endpointAddress());
    linNode.UpdateResponses(msg.frameResponses, _logger);

    for (auto& response : msg.frameResponses)
    {
        CallHandlers(
            _frameResponseUpdateHandler, this,
            LinFrameResponseUpdateEvent{ from->GetServiceDescriptor().to_string(), response});
    }
}

void LinController::SetTimeProvider(mw::sync::ITimeProvider* timeProvider)
{
    _timeProvider = timeProvider;
}

void LinController::SetControllerStatus(LinControllerStatus status)
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

auto LinController::VeriyChecksum(const LinFrame& frame, LinFrameStatus status) -> LinFrameStatus
{
    if (status != LinFrameStatus::LIN_RX_OK)
        return status;

    auto& node = GetLinNode(_serviceDescriptor.to_endpointAddress());
    auto& expectedFrame = node.responses[frame.id].frame;

    if (expectedFrame.dataLength != frame.dataLength || expectedFrame.checksumModel != frame.checksumModel)
    {
        return LinFrameStatus::LIN_RX_ERROR;
    }

    return status;
}


template <typename MsgT>
void LinController::SendIbMessage(MsgT&& msg)
{
    _participant->SendIbMessage(this, std::forward<MsgT>(msg));
}

// ================================================================================
//  LinController::LinNode
// ================================================================================
void LinController::LinNode::UpdateResponses(std::vector<LinFrameResponse> responses_, mw::logging::ILogger* logger)
{
    for (auto&& response : responses_)
    {
        auto linId = response.frame.id;
        if (linId >= responses.size())
        {
            logger->Warn("Ignoring LinFrameResponse update for linId={}", static_cast<uint16_t>(linId));
            continue;
        }
        responses[linId] = std::move(response);
    }
}


} // namespace lin
} // namespace sim
} // namespace ib
