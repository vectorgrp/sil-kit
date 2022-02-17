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

inline auto ToFrameResponseMode(FrameResponseType responseType) -> FrameResponseMode
{
    switch (responseType) {
    case FrameResponseType::MasterResponse:
        return FrameResponseMode::TxUnconditional;
    case FrameResponseType::SlaveResponse:
        return FrameResponseMode::Rx;
    case FrameResponseType::SlaveToSlave:
        return FrameResponseMode::Unused;
    }
    return FrameResponseMode::Unused;
};

inline auto ToTxFrameStatus(FrameStatus status) -> FrameStatus
{
    switch (status)
    {
    case FrameStatus::LIN_RX_BUSY:
        return FrameStatus::LIN_TX_BUSY;
    case FrameStatus::LIN_RX_ERROR:
        return FrameStatus::LIN_TX_ERROR;
    case FrameStatus::LIN_RX_NO_RESPONSE:
        return FrameStatus::LIN_RX_NO_RESPONSE;
    case FrameStatus::LIN_RX_OK:
        return FrameStatus::LIN_TX_OK;
    default:
        return status;
    }
}

inline auto ToTracingDir(FrameStatus status) -> ib::sim::TransmitDirection
{
    switch (status)
    {
    case FrameStatus::LIN_RX_ERROR: //[[fallthrough]]
    case FrameStatus::LIN_RX_BUSY: //[[fallthrough]]
    case FrameStatus::LIN_RX_NO_RESPONSE: //[[fallthrough]]
    case FrameStatus::LIN_RX_OK: 
        return ib::sim::TransmitDirection::RX;
    case FrameStatus::LIN_TX_ERROR: //[[fallthrough]]
    case FrameStatus::LIN_TX_BUSY: //[[fallthrough]]
    case FrameStatus::LIN_TX_HEADER_ERROR: //[[fallthrough]]
    case FrameStatus::LIN_TX_OK: 
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

LinController::LinController(mw::IComAdapterInternal* comAdapter, mw::sync::ITimeProvider* timeProvider, ILinController* facade)
    : _comAdapter{comAdapter}
    , _logger{comAdapter->GetLogger()}
    , _timeProvider{timeProvider}
    , _facade{facade}
{
    if (_facade == nullptr)
    {
        _facade = this;
    }
}

void LinController::Init(ControllerConfig config)
{
    auto& node = GetLinNode(_serviceDescriptor.to_endpointAddress());
    node.controllerMode = config.controllerMode;
    node.controllerStatus = ControllerStatus::Operational;
    node.UpdateResponses(config.frameResponses, _logger);

    _controllerMode = config.controllerMode;
    _controllerStatus = ControllerStatus::Operational;
    SendIbMessage(config);
}

auto LinController::Status() const noexcept -> ControllerStatus
{
    return _controllerStatus;
}

void LinController::SendFrame(Frame frame, FrameResponseType responseType)
{
    SendFrame(std::move(frame), std::move(responseType), _timeProvider->Now());
}

void LinController::SendFrame(Frame frame, FrameResponseType responseType, std::chrono::nanoseconds timestamp)
{
    SetFrameResponse(frame, ToFrameResponseMode(responseType));
    SendFrameHeader(frame.id, timestamp);
}

void LinController::SendFrameHeader(LinIdT linId)
{
    SendFrameHeader(linId, _timeProvider->Now());
}

void LinController::SendFrameHeader(LinIdT linId, std::chrono::nanoseconds timestamp)
{
    if (_controllerMode != ControllerMode::Master)
    {
        std::string errorMsg{"LinController::SendFrameHeader() must only be called in master mode!"};
        _logger->Error(errorMsg);
        throw std::runtime_error{errorMsg};
    }

    // we answer the call immediately based on the cached responses
    // setup a reply
    Transmission transmission;
    transmission.frame.id = linId;
    transmission.timestamp = timestamp;

    auto numResponses = 0;
    for (auto&& node : _linNodes)
    {
        if (node.controllerMode == ControllerMode::Inactive)
            continue;
        if (node.controllerStatus != ControllerStatus::Operational)
            continue;

        auto& response = node.responses[linId];
        if (response.responseMode == FrameResponseMode::TxUnconditional)
        {
            transmission.frame = response.frame;
            numResponses++;
        }
    }

    if (numResponses == 0)
    {
        transmission.status = FrameStatus::LIN_RX_NO_RESPONSE;
    }
    else if (numResponses == 1)
    {
        transmission.status = FrameStatus::LIN_RX_OK;
    }
    else if (numResponses > 1)
    {
        transmission.status = FrameStatus::LIN_RX_ERROR;
    }

    // Dispatch the LIN transmission to all connected nodes
    SendIbMessage(transmission);

    // Dispatch the LIN transmission to our own callbacks
    FrameResponseMode masterResponseMode = GetLinNode(_serviceDescriptor.to_endpointAddress()).responses[linId].responseMode;
    FrameStatus masterFrameStatus = transmission.status;
    if (masterResponseMode == FrameResponseMode::TxUnconditional)
    {
        masterFrameStatus = ToTxFrameStatus(masterFrameStatus);
    }

    _tracer.Trace(ToTracingDir(masterFrameStatus), timestamp, transmission.frame);

    // dispatch the reply locally...
    CallHandlers(_frameStatusHandler, this, transmission.frame, masterFrameStatus, transmission.timestamp);
}

void LinController::SetFrameResponse(Frame frame, FrameResponseMode mode)
{
    FrameResponse response;
    response.frame = std::move(frame);
    response.responseMode = mode;

    std::vector<FrameResponse> responses{1, response};
    SetFrameResponses(std::move(responses));
}

void LinController::SetFrameResponses(std::vector<FrameResponse> responses)
{
    auto& node = GetLinNode(_serviceDescriptor.to_endpointAddress());
    node.UpdateResponses(responses, _logger);

    FrameResponseUpdate frameResponseUpdate;
    frameResponseUpdate.frameResponses = std::move(responses);
    SendIbMessage(frameResponseUpdate);
}

void LinController::GoToSleep()
{
    if (_controllerMode != ControllerMode::Master)
    {
        std::string errorMsg{"LinController::GoToSleep() must only be called in master mode!"};
        _logger->Error(errorMsg);
        throw std::logic_error{errorMsg};
    }

    Transmission gotosleepTx;
    gotosleepTx.frame = GoToSleepFrame();
    gotosleepTx.status = FrameStatus::LIN_RX_OK;

    SendIbMessage(gotosleepTx);

    // For trivial simulations we go directly to Sleep state.
    GoToSleepInternal();
}

void LinController::GoToSleepInternal()
{
    SetControllerStatus(ControllerStatus::Sleep);
}

void LinController::Wakeup()
{
    WakeupPulse pulse;
    SendIbMessage(pulse);
    WakeupInternal();
}

void LinController::WakeupInternal()
{
    SetControllerStatus(ControllerStatus::Operational);
}

void LinController::RegisterFrameStatusHandler(FrameStatusHandler handler)
{
    _frameStatusHandler.emplace_back(std::move(handler));
}

void LinController::RegisterGoToSleepHandler(GoToSleepHandler handler)
{
    _goToSleepHandler.emplace_back(std::move(handler));
}

void LinController::RegisterWakeupHandler(WakeupHandler handler)
{
    _wakeupHandler.emplace_back(std::move(handler));
}

void LinController::RegisterFrameResponseUpdateHandler(FrameResponseUpdateHandler handler)
{
    _frameResponseUpdateHandler.emplace_back(std::move(handler));
}

void LinController::ReceiveIbMessage(const IIbServiceEndpoint* from, const Transmission& msg)
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

    if (_controllerStatus != ControllerStatus::Operational)
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
    case ControllerMode::Inactive:
        return;

    case ControllerMode::Master:
        _logger->Warn("LinController in MasterMode received a transmission from {{{}, {}}}. This indicates an erroneous setup as there should be only one LIN master!",
            from->GetServiceDescriptor().GetParticipantName(),
            from->GetServiceDescriptor().GetServiceName());
        //[[fallthrough]]

    case ControllerMode::Slave:

        auto& thisLinNode = GetLinNode(_serviceDescriptor.to_endpointAddress());
        switch (thisLinNode.responses[frame.id].responseMode)
        {
        case FrameResponseMode::Unused:
            break;
        case FrameResponseMode::Rx:
        {
            const auto msgStatus = VeriyChecksum(frame, msg.status);
            if (msgStatus == FrameStatus::LIN_RX_OK)
            {
                _tracer.Trace(ib::sim::TransmitDirection::RX, _timeProvider->Now(), frame);
            }
            CallHandlers(_frameStatusHandler, this, frame, msgStatus, msg.timestamp);
            break;
        }
        case FrameResponseMode::TxUnconditional:
            // Transmissions are always sent with FrameStatus::RX_xxx so we have to
            // convert the status to a TX_xxx if we sent this frame.
            if (msg.status == FrameStatus::LIN_RX_OK)
            {
                _tracer.Trace(ib::sim::TransmitDirection::TX, _timeProvider->Now(), frame);
            }
            CallHandlers(_frameStatusHandler, this, frame, ToTxFrameStatus(msg.status), msg.timestamp);
            break;
        }

        // Dispatch GoToSleep frames
        if (frame.id == GoToSleepFrame().id && frame.data == GoToSleepFrame().data)
        {
            _tracer.Trace(ToTracingDir(msg.status), _timeProvider->Now(), frame);
            CallHandlers(_goToSleepHandler, this);
        }
    }
}

void LinController::ReceiveIbMessage(const IIbServiceEndpoint* from, const WakeupPulse& /*msg*/)
{
    CallHandlers(_wakeupHandler, this);
}

void LinController::ReceiveIbMessage(const IIbServiceEndpoint* from, const ControllerConfig& msg)
{
    auto& linNode = GetLinNode(from->GetServiceDescriptor().to_endpointAddress());

    linNode.controllerMode = msg.controllerMode;
    linNode.controllerStatus = ControllerStatus::Operational;
    linNode.UpdateResponses(msg.frameResponses, _logger);

    for (auto& response : msg.frameResponses)
    {
        CallHandlers(_frameResponseUpdateHandler, this, from->GetServiceDescriptor().to_string(), response);
    }
}

void LinController::ReceiveIbMessage(const IIbServiceEndpoint* from, const ControllerStatusUpdate& msg)
{
    auto& linNode = GetLinNode(from->GetServiceDescriptor().to_endpointAddress());
    linNode.controllerStatus = msg.status;
}

void LinController::ReceiveIbMessage(const IIbServiceEndpoint* from, const FrameResponseUpdate& msg)
{
    auto& linNode = GetLinNode(from->GetServiceDescriptor().to_endpointAddress());
    linNode.UpdateResponses(msg.frameResponses, _logger);

    for (auto& response : msg.frameResponses)
    {
        CallHandlers(_frameResponseUpdateHandler, this, from->GetServiceDescriptor().to_string(), response);
    }
}

void LinController::SetTimeProvider(mw::sync::ITimeProvider* timeProvider)
{
    _timeProvider = timeProvider;
}

void LinController::SetControllerStatus(ControllerStatus status)
{
    if (_controllerMode == ControllerMode::Inactive)
    {
        std::string errorMsg{"LinController::Wakeup()/Sleep() must not be called before LinController::Init()"};
        _logger->Error(errorMsg);
        throw std::runtime_error{errorMsg};
    }

    if (_controllerStatus == status)
    {
        _logger->Warn("LinController::SetControllerStatus() - controller is already in {} mode.", to_string(status));
    }

    _controllerStatus = status;

    ControllerStatusUpdate msg;
    msg.status = status;

    SendIbMessage(msg);
}

auto LinController::VeriyChecksum(const Frame& frame, FrameStatus status) -> FrameStatus
{
    if (status != FrameStatus::LIN_RX_OK)
        return status;

    auto& node = GetLinNode(_serviceDescriptor.to_endpointAddress());
    auto& expectedFrame = node.responses[frame.id].frame;

    if (expectedFrame.dataLength != frame.dataLength || expectedFrame.checksumModel != frame.checksumModel)
    {
        return FrameStatus::LIN_RX_ERROR;
    }

    return status;
}


template <typename MsgT>
void LinController::SendIbMessage(MsgT&& msg)
{
    _comAdapter->SendIbMessage(this, std::forward<MsgT>(msg));
}

// ================================================================================
//  LinController::LinNode
// ================================================================================
void LinController::LinNode::UpdateResponses(std::vector<FrameResponse> responses_, mw::logging::ILogger* logger)
{
    for (auto&& response : responses_)
    {
        auto linId = response.frame.id;
        if (linId >= responses.size())
        {
            logger->Warn("Ignoring FrameResponse update for linId={}", static_cast<uint16_t>(linId));
            continue;
        }
        responses[linId] = std::move(response);
    }
}


} // namespace lin
} // namespace sim
} // namespace ib
