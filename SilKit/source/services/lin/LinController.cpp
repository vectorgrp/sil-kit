// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "LinController.hpp"

#include <iostream>
#include <chrono>

#include "silkit/core/logging/ILogger.hpp"
#include "silkit/services/lin/string_utils.hpp"
#include "IServiceDiscovery.hpp"
#include "ServiceDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace Lin {

LinController::LinController(Core::IParticipantInternal* participant, SilKit::Config::LinController config,
                               Core::Orchestration::ITimeProvider* timeProvider)
    : _participant{participant}
    , _config{config}
    , _logger{participant->GetLogger()}
    , _simulationBehavior{participant, this, timeProvider}
{

}

//------------------------
// Trivial or detailed
//------------------------

void LinController::RegisterServiceDiscovery()
{
    _participant->GetServiceDiscovery()->RegisterServiceDiscoveryHandler(
        [this](Core::Discovery::ServiceDiscoveryEvent::Type discoveryType,
                                  const Core::ServiceDescriptor& remoteServiceDescriptor) {
            // check if discovered service is a network simulator (if none is known)
            if (_simulationBehavior.IsTrivial())
            {
                // check if received descriptor has a matching simulated link
                if (discoveryType == Core::Discovery::ServiceDiscoveryEvent::Type::ServiceCreated
                    && IsRelevantNetwork(remoteServiceDescriptor))
                {
                    SetDetailedBehavior(remoteServiceDescriptor);
                }
            }
            else
            {
                if (discoveryType == Core::Discovery::ServiceDiscoveryEvent::Type::ServiceRemoved
                    && IsRelevantNetwork(remoteServiceDescriptor))
                
                {
                    SetTrivialBehavior();
                }
            }
        });
}

void LinController::SetDetailedBehavior(const Core::ServiceDescriptor& remoteServiceDescriptor)
{
    _simulationBehavior.SetDetailedBehavior(remoteServiceDescriptor);
}

void LinController::SetTrivialBehavior()
{
    _simulationBehavior.SetTrivialBehavior();
}

auto LinController::AllowReception(const IServiceEndpoint* from) const -> bool
{
    return _simulationBehavior.AllowReception(from);
}

auto LinController::IsRelevantNetwork(const Core::ServiceDescriptor& remoteServiceDescriptor) const -> bool
{
    return remoteServiceDescriptor.GetServiceType() == SilKit::Core::ServiceType::Link
           && remoteServiceDescriptor.GetNetworkName() == _serviceDescriptor.GetNetworkName();
}

template <typename MsgT>
void LinController::SendMsg(MsgT&& msg)
{
    // Detailed: Distribute SilKitMessage
    // Trivial: 
    //   LinSendFrameRequest: Call SetFrameResponse and SendFrameHeader
    //   LinSendFrameHeaderRequest: Send a LinTansmission based on the cached responses and self-deliver LinFrameStatusEvent
    //   Other: Distribute SilKitMessage
    _simulationBehavior.SendMsg(std::move(msg));
}

//------------------------
// Public API + Helpers
//------------------------

void LinController::Init(LinControllerConfig config)
{
    auto& node = GetThisLinNode();
    node.controllerMode = config.controllerMode;
    node.controllerStatus = LinControllerStatus::Operational;
    node.UpdateResponses(config.frameResponses, _logger);

    _controllerMode = config.controllerMode;
    _controllerStatus = LinControllerStatus::Operational;
    SendMsg(config);
}

auto LinController::Status() const noexcept -> LinControllerStatus
{
    return _controllerStatus;
}

void LinController::SendFrame(LinFrame frame, LinFrameResponseType responseType)
{
    if (_controllerStatus != LinControllerStatus::Operational)
    {
        std::string errorMsg{"LinController::SendFrame() must only be called when the controller is operational! Check "
                             "whether a call to LinController::Init is missing."};
        _logger->Error(errorMsg);
        throw SilKit::StateError{errorMsg};
    }
    if (_controllerMode != LinControllerMode::Master)
    {
        std::string errorMsg{"LinController::SendFrame() must only be called in master mode!"};
        _logger->Error(errorMsg);
        throw std::runtime_error{errorMsg};
    }

    LinSendFrameRequest sendFrame;
    sendFrame.frame = frame;
    sendFrame.responseType = responseType;
    SendMsg(sendFrame);
}

void LinController::SendFrameHeader(LinIdT linId)
{
    if (_controllerStatus != LinControllerStatus::Operational)
    {
        std::string errorMsg{"LinController::SendFrameHeader() must only be called when the controller is operational! "
                             "Check whether a call to LinController::Init is missing."};
        _logger->Error(errorMsg);
        throw SilKit::StateError{errorMsg};
    }
    if (_controllerMode != LinControllerMode::Master)
    {
        std::string errorMsg{"LinController::SendFrameHeader() must only be called in master mode!"};
        _logger->Error(errorMsg);
        throw std::runtime_error{errorMsg};
    }
    LinSendFrameHeaderRequest header;
    header.id = linId;
    SendMsg(header);
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
    if (_controllerStatus != LinControllerStatus::Operational)
    {
        std::string errorMsg{"LinController::SetFrameResponses() must only be called when the controller is operational! "
                             "Check whether a call to LinController::Init is missing."};
        _logger->Error(errorMsg);
        throw SilKit::StateError {errorMsg};
    }

    GetThisLinNode().UpdateResponses(responses, _logger);

    LinFrameResponseUpdate frameResponseUpdate;
    frameResponseUpdate.frameResponses = std::move(responses);
    SendMsg(frameResponseUpdate);
}

void LinController::GoToSleep()
{
    if (_controllerMode != LinControllerMode::Master)
    {
        std::string errorMsg{"LinController::GoToSleep() must not be called for slaves or uninitialized masters!"};
        _logger->Error(errorMsg);
        throw SilKit::StateError{errorMsg};
    }

    // Detailed: Send LinSendFrameRequest with GoToSleep-Frame and set LinControllerStatus::SleepPending. BusSim will trigger LinTransmission.
    // Trivial: Directly send LinTransmission with GoToSleep-Frame and call GoToSleepInternal() on this controller.
    _simulationBehavior.GoToSleep();

    _controllerStatus = LinControllerStatus::Sleep;
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
        throw SilKit::StateError{errorMsg};
    }

    // Detailed: Send LinWakeupPulse and call WakeupInternal()
    // Trivial: Send LinWakeupPulse and call WakeupInternal(), self-deliver LinWakeupPulse with TX
    _simulationBehavior.Wakeup();
}

void LinController::WakeupInternal()
{
    SetControllerStatus(LinControllerStatus::Operational);
}

void LinController::SetControllerStatus(LinControllerStatus status)
{
    if (_controllerMode == LinControllerMode::Inactive)
    {
        std::string errorMsg{"LinController::Wakeup()/Sleep() must not be called before LinController::Init()"};
        _logger->Error(errorMsg);
        throw SilKit::StateError{errorMsg};
    }

    if (_controllerStatus == status)
    {
        _logger->Warn("LinController::SetControllerStatus() - controller is already in {} mode.", to_string(status));
    }

    _controllerStatus = status;

    LinControllerStatusUpdate msg;
    msg.status = status;

    SendMsg(msg);
}

//------------------------
// Node bookkeeping
//------------------------

void LinController::LinNode::UpdateResponses(std::vector<LinFrameResponse> responses_, Core::Logging::ILogger* logger)
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

auto LinController::GetThisLinNode() -> LinNode&
{
    return GetLinNode(_serviceDescriptor.to_endpointAddress());
}

auto LinController::GetLinNode(Core::EndpointAddress addr) -> LinNode&
{
    auto iter = std::lower_bound(_linNodes.begin(), _linNodes.end(), addr,
                                 [](const LinNode& lhs, const Core::EndpointAddress& address) {
                                     return lhs.address < address;
                                 });
    if (iter == _linNodes.end() || iter->address != addr)
    {
        LinNode node;
        node.address = addr;
        iter = _linNodes.insert(iter, node);
    }
    return *iter;
}

void LinController::CallLinFrameStatusEventHandler(const LinFrameStatusEvent& msg)
{
    // Trivial: Used to dispatch the LinFrameStatusEvent locally
    CallHandlers(msg);
}

auto LinController::GetResponse(LinIdT id) -> std::pair<int, LinFrame>
{
    LinFrame responseFrame;

    auto numResponses = 0;
    for (auto&& node : _linNodes)
    {
        if (node.controllerMode == LinControllerMode::Inactive)
            continue;
        if (node.controllerStatus != LinControllerStatus::Operational)
            continue;

        auto& response = node.responses[id];
        if (response.responseMode == LinFrameResponseMode::TxUnconditional)
        {
            responseFrame = response.frame;
            numResponses++;
        }
    }

    return {numResponses, responseFrame};
}

//------------------------
// ReceiveSilKitMessage
//------------------------

void LinController::ReceiveSilKitMessage(const IServiceEndpoint* from, const LinTransmission& msg)
{
    if (!AllowReception(from))
    {
        return;
    }

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
        _logger->Warn("Inactive LinController received a transmission.");

    _tracer.Trace(SilKit::Services::TransmitDirection::RX, msg.timestamp, frame);

    bool isGoToSleepFrame = frame.id == GoToSleepFrame().id && frame.data == GoToSleepFrame().data;

    // Detailed: Just use msg.status
    // Trivial: Evaluate status using cached response
    const LinFrameStatus msgStatus = _simulationBehavior.CalcFrameStatus(msg, isGoToSleepFrame);

    // Dispatch frame to handlers
    if (msgStatus != LinFrameStatus::LIN_RX_NO_RESPONSE)
    {
        CallHandlers(LinFrameStatusEvent{msg.timestamp, frame, msgStatus});
    }

    // Dispatch GoToSleep frames to dedicated handlers
    if (isGoToSleepFrame)
    {
        // only call GoToSleepHandlers for slaves, i.e., not for the master that issued the GoToSleep command.
        if (_controllerMode == LinControllerMode::Slave)
        {
            CallHandlers(LinGoToSleepEvent{msg.timestamp});
        }
    }
}

void LinController::ReceiveSilKitMessage(const IServiceEndpoint* from, const LinWakeupPulse& msg)
{
    if (!AllowReception(from))
    {
        return;
    }

    CallHandlers(LinWakeupEvent{msg.timestamp, msg.direction});
}

void LinController::ReceiveSilKitMessage(const IServiceEndpoint* from, const LinControllerConfig& msg)
{
    // We also receive LinFrameResponseUpdate from other controllers, although we would not need them in detailed simulations.
    // However, we also want to make users of FrameResponseUpdateHandlers happy when using the detailed simulations.
    // NOTE: only self-delivered messages are rejected
    if (from->GetServiceDescriptor() == _serviceDescriptor)
        return;

    auto& linNode = GetLinNode(from->GetServiceDescriptor().to_endpointAddress());
    linNode.controllerMode = msg.controllerMode;
    linNode.controllerStatus = LinControllerStatus::Operational;
    linNode.UpdateResponses(msg.frameResponses, _logger);

    for (auto& response : msg.frameResponses)
    {
        CallHandlers(LinFrameResponseUpdateEvent{ from->GetServiceDescriptor().to_string(), response});
    }
}

void LinController::ReceiveSilKitMessage(const IServiceEndpoint* from, const LinFrameResponseUpdate& msg)
{
    // We also receive LinFrameResponseUpdate from other controllers, although we would not need them in detailed simulations.
    // However, we also want to make users of FrameResponseUpdateHandlers happy when using the detailed simulations.
    // NOTE: only self-delivered messages are rejected
    if (from->GetServiceDescriptor() == _serviceDescriptor) 
        return;

    auto& linNode = GetLinNode(from->GetServiceDescriptor().to_endpointAddress());
    linNode.UpdateResponses(msg.frameResponses, _logger);

    for (auto& response : msg.frameResponses)
    {
        CallHandlers(LinFrameResponseUpdateEvent{ from->GetServiceDescriptor().to_string(), response});
    }
}

void LinController::ReceiveSilKitMessage(const IServiceEndpoint* from, const LinControllerStatusUpdate& msg)
{
    auto& linNode = GetLinNode(from->GetServiceDescriptor().to_endpointAddress());
    linNode.controllerStatus = msg.status;
}

//------------------------
// Handlers
//------------------------

HandlerId LinController::AddFrameStatusHandler(FrameStatusHandler handler)
{
    return AddHandler(std::move(handler));
}

void LinController::RemoveFrameStatusHandler(HandlerId handlerId)
{
    if (!RemoveHandler<LinFrameStatusEvent>(handlerId))
    {
        _participant->GetLogger()->Warn("RemoveFrameStatusHandler failed: Unknown HandlerId.");
    }
}

HandlerId LinController::AddGoToSleepHandler(GoToSleepHandler handler)
{
    return AddHandler(std::move(handler));
}

void LinController::RemoveGoToSleepHandler(HandlerId handlerId)
{
    if (!RemoveHandler<LinGoToSleepEvent>(handlerId))
    {
        _participant->GetLogger()->Warn("RemoveGoToSleepHandler failed: Unknown HandlerId.");
    }
}

HandlerId LinController::AddWakeupHandler(WakeupHandler handler)
{
    return AddHandler(std::move(handler));
}

void LinController::RemoveWakeupHandler(HandlerId handlerId)
{
    if (!RemoveHandler<LinWakeupEvent>(handlerId))
    {
        _participant->GetLogger()->Warn("RemoveWakeupHandler failed: Unknown HandlerId.");
    }
}

HandlerId LinController::AddFrameResponseUpdateHandler(FrameResponseUpdateHandler handler)
{
    return AddHandler(std::move(handler));
}

void LinController::RemoveFrameResponseUpdateHandler(HandlerId handlerId)
{
    if (!RemoveHandler<LinFrameResponseUpdateEvent>(handlerId))
    {
        _participant->GetLogger()->Warn("RemoveFrameResponseUpdateHandler failed: Unknown HandlerId.");
    }
}

template <typename MsgT>
HandlerId LinController::AddHandler(CallbackT<MsgT> handler)
{
    auto& callbacks = std::get<CallbacksT<MsgT>>(_callbacks);
    return callbacks.Add(std::move(handler));
}

template <typename MsgT>
auto LinController::RemoveHandler(HandlerId handlerId) -> bool
{
    auto& callbacks = std::get<CallbacksT<MsgT>>(_callbacks);
    return callbacks.Remove(handlerId);
}

template <typename MsgT>
void LinController::CallHandlers(const MsgT& msg)
{
    auto& callbacks = std::get<CallbacksT<MsgT>>(_callbacks);
    callbacks.InvokeAll(this, msg);
}

} // namespace Lin
} // namespace Services
} // namespace SilKit
