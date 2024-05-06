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

#include "LinController.hpp"

#include <iostream>
#include <chrono>

#include "silkit/services/lin/string_utils.hpp"
#include "IServiceDiscovery.hpp"
#include "ServiceDatatypes.hpp"
#include "Tracing.hpp"

#include "ILogger.hpp"
#include "WireLinMessages.hpp"

namespace
{
using namespace SilKit::Services::Lin;
auto to_wire(const LinControllerConfig& config) -> WireLinControllerConfig
{
    WireLinControllerConfig result{};
    result.baudRate = config.baudRate;
    result.controllerMode = config.controllerMode;
    result.frameResponses = config.frameResponses;

    return result;
}

auto to_wire(const SilKit::Experimental::Services::Lin::LinControllerDynamicConfig& config) -> WireLinControllerConfig
{
    WireLinControllerConfig result{};
    result.baudRate = config.baudRate;
    result.controllerMode = config.controllerMode;
    result.simulationMode = WireLinControllerConfig::SimulationMode::Dynamic;

    return result;
}

}
namespace SilKit {
namespace Services {
namespace Lin {

LinController::LinController(Core::IParticipantInternal* participant, SilKit::Config::LinController config,
                               Services::Orchestration::ITimeProvider* timeProvider)
    : _participant{participant}
    , _config{config}
    , _logger{participant->GetLogger()}
    , _simulationBehavior{participant, this, timeProvider}
    , _timeProvider{timeProvider}
    , _replayActive{Tracing::IsValidReplayConfig(_config.replay)}
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
                    Logging::Info(_logger,
                                  "Controller '{}' is using the simulated network '{}' and will route all messages to "
                                  "the network simulator '{}'",
                                  _config.name, remoteServiceDescriptor.GetNetworkName(),
                                  remoteServiceDescriptor.GetParticipantName());
                    SetDetailedBehavior(remoteServiceDescriptor);
                }
            }
            else
            {
                if (discoveryType == Core::Discovery::ServiceDiscoveryEvent::Type::ServiceRemoved
                    && IsRelevantNetwork(remoteServiceDescriptor))
                
                {
                    Logging::Warn(_logger,
                                  "The network simulator for controller '{}' left the simulation. The controller is no "
                                  "longer simulated.",
                                  _config.name);
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
    _simulationBehavior.SendMsg(std::move(msg));
}

//------------------------
// Error handling
//------------------------

void LinController::ThrowIfUninitialized(const std::string& callingMethodName) const
{
    if (_controllerStatus == LinControllerStatus::Unknown)
    {
        std::string errorMsg = callingMethodName
                               + " must only be called when the controller is initialized! Check "
                                 "whether a call to LinController::Init is missing.";
        _logger->Error(errorMsg);
        throw SilKit::StateError{errorMsg};
    }
}

void LinController::ThrowIfNotMaster(const std::string& callingMethodName) const
{
    if (_controllerMode != LinControllerMode::Master)
    {
        std::string errorMsg = callingMethodName
                               + " must only be called in master mode!";
        _logger->Error(errorMsg);
        throw SilKitError{errorMsg};
    }
}

void LinController::ThrowIfDynamic(const std::string& callingMethodName) const
{
    if (_useDynamicResponse)
    {
        std::string errorMsg = callingMethodName
                               + " can not be called if the node was initialized using InitDynamic!";
        _logger->Error(errorMsg);
        throw SilKitError{errorMsg};
    }
}

void LinController::ThrowIfNotDynamic(const std::string& callingMethodName) const
{
    if (_useDynamicResponse)
    {
        std::string errorMsg = callingMethodName
                               + " can only be called if the node was initialized using InitDynamic!";
        _logger->Error(errorMsg);
        throw SilKitError{errorMsg};
    }
}

void LinController::ThrowIfNotConfiguredTxUnconditional(LinId linId)
{
    if (GetThisLinNode().responses[linId].responseMode != LinFrameResponseMode::TxUnconditional)
    {
        std::string errorMsg = fmt::format("This node must be configured with LinFrameResponseMode::TxUnconditional to "
                                           "update the TxBuffer for ID {}",
                                           static_cast<uint16_t>(linId));
        _logger->Error(errorMsg);
        throw SilKit::ConfigurationError{errorMsg};
    }
}

void LinController::WarnOnWrongDataLength(const LinFrame& receivedFrame, const LinFrame& configuredFrame) const
{
    std::string errorMsg =
        fmt::format("Mismatch between configured ({}) and received ({}) LinDataLength in LinFrame with ID {}",
                    configuredFrame.dataLength, receivedFrame.dataLength, static_cast<uint16_t>(receivedFrame.id));
    _logger->Warn(errorMsg);
}

void LinController::WarnOnWrongChecksum(const LinFrame& receivedFrame, const LinFrame& configuredFrame) const
{
    std::string errorMsg = fmt::format(
        "Mismatch between configured ({}) and received ({}) LinChecksumModel in LinFrame with ID {}",
        configuredFrame.checksumModel, receivedFrame.checksumModel, static_cast<uint16_t>(receivedFrame.id));
    _logger->Warn(errorMsg);
}

void LinController::WarnOnReceptionWithInvalidDataLength(LinDataLength invalidDataLength,
                                                         const std::string& fromParticipantName,
                                                         const std::string& fromServiceName) const
{
    std::string errorMsg =
        fmt::format("LinController received transmission with invalid payload length {} from {{{}, {}}}. This "
                    "tranmission is ignored.",
                    static_cast<uint16_t>(invalidDataLength), fromParticipantName, fromServiceName);
    _logger->Warn(errorMsg);
}

void LinController::WarnOnReceptionWithInvalidLinId(LinId invalidLinId, const std::string& fromParticipantName,
                                                    const std::string& fromServiceName) const
{
    std::string errorMsg = fmt::format(
        "LinController received transmission with invalid LIN ID {} from {{{}, {}}}. This transmission is ignored.",
        static_cast<uint16_t>(invalidLinId), fromParticipantName, fromServiceName);
    _logger->Warn(errorMsg);
}

void LinController::WarnOnReceptionWhileInactive() const
{
    std::string errorMsg = fmt::format("Inactive LinController received a transmission. This transmission is ignored.");
    _logger->Warn(errorMsg);
}

void LinController::WarnOnUnneededStatusChange(LinControllerStatus status) const
{
    std::string errorMsg =
        fmt::format("Invalid LinController status change: controller is already in {} mode.", status);
    _logger->Warn(errorMsg);
}

void LinController::WarnOnInvalidLinId(LinId invalidLinId, const std::string& callingMethodName) const
{
    std::string errorMsg =
        fmt::format("Invalid ID={} in call to '{}'", static_cast<uint16_t>(invalidLinId), callingMethodName);
    _logger->Warn(errorMsg);
}

void LinController::WarnOnUnusedResponseMode(const std::string& callingMethodName) const
{
    std::string errorMsg =
        fmt::format("LinFrameResponseMode::Unused is not allowed in call to '{}'.", callingMethodName);
    _logger->Warn(errorMsg);
}

void LinController::WarnOnResponseModeReconfiguration(LinId id, LinFrameResponseMode currentResponseMode) const
{
    std::string errorMsg =
        fmt::format("Can't set response mode for ID={}. Mode is already configured to {}.", id, currentResponseMode);
    _logger->Warn(errorMsg);
}


void LinController::WarnOnUnconfiguredSlaveResponse(LinId id) const
{
    std::string errorMsg =
        fmt::format("No slave has configured a response for ID={}. Use Init() or SetFrameResponse() on the slave node to configure responses.", id);
    _logger->Warn(errorMsg);
}

void LinController::WarnOnSendFrameSlaveResponseWithMasterTx(LinId id) const
{
    std::string errorMsg =
        fmt::format("Master has already configured a response on ID={}. Ignoring this call to SendFrame()", id);
    _logger->Warn(errorMsg);
}

void LinController::ThrowOnSendAttemptWithUndefinedChecksum(const LinFrame& frame) const
{
    std::string errorMsg =
        fmt::format("LinFrame with ID {} has an undefined checksum model.",
                    static_cast<uint16_t>(frame.id));
    _logger->Error(errorMsg);
    throw SilKit::StateError{errorMsg};
}

void LinController::ThrowOnSendAttemptWithUndefinedDataLength(const LinFrame& frame) const
{
    std::string errorMsg =
        fmt::format("LinFrame with ID {} has an undefined data length.",
                    static_cast<uint16_t>(frame.id));
    _logger->Error(errorMsg);
    throw SilKit::StateError{errorMsg};
}

void LinController::ThrowOnErroneousInitialization() const
{
    std::string errorMsg{"A LinController can't be initialized with LinControllerMode::Inactive!"};
    _logger->Error(errorMsg);
    throw SilKit::StateError{errorMsg};
}

void LinController::ThrowOnDuplicateInitialization() const
{
    std::string errorMsg{"LinController::Init() must only be called once!"};
    _logger->Error(errorMsg);
    throw SilKit::StateError{errorMsg};
}

//------------------------
// Public API
//------------------------

void LinController::Init(LinControllerConfig config)
{
    if (config.controllerMode == LinControllerMode::Inactive)
    {
        ThrowOnErroneousInitialization();
    }

    if (_controllerStatus != LinControllerStatus::Unknown)
    {
        ThrowOnDuplicateInitialization();
    }

    auto& node = GetThisLinNode();
    node.controllerMode = config.controllerMode;
    node.controllerStatus = LinControllerStatus::Operational;
    node.UpdateResponses(config.frameResponses, _logger);

    _controllerMode = config.controllerMode;
    _controllerStatus = LinControllerStatus::Operational;

    SendMsg(to_wire(config));
}

void LinController::InitDynamic(const SilKit::Experimental::Services::Lin::LinControllerDynamicConfig& config)
{
    if (config.controllerMode == LinControllerMode::Inactive)
    {
        ThrowOnErroneousInitialization();
    }

    if (_controllerStatus != LinControllerStatus::Unknown)
    {
        ThrowOnDuplicateInitialization();
    }

    auto& node = GetThisLinNode();
    node.controllerMode = config.controllerMode;
    node.controllerStatus = LinControllerStatus::Operational;
    node.simulationMode = WireLinControllerConfig::SimulationMode::Dynamic;

    _controllerMode = config.controllerMode;
    _controllerStatus = LinControllerStatus::Operational;
    _useDynamicResponse = true;

    SendMsg(to_wire(config));
}

void LinController::SendDynamicResponse(const LinFrame& frame)
{
    if (!_useDynamicResponse)
    {
        ThrowIfNotDynamic(__FUNCTION__);
    }

    // prepare the response update
    LinFrameResponse response{};
    response.frame = frame;
    response.responseMode = LinFrameResponseMode::TxUnconditional;

    // distribute the response update
    UpdateFrameResponse(response);

    // invoke actual response
    LinSendFrameHeaderRequest request{};
    request.id = response.frame.id;
    request.timestamp = _timeProvider->Now();
    _simulationBehavior.ProcessFrameHeaderRequest(request);
}

auto LinController::Mode() const noexcept -> LinControllerMode
{
    return _controllerMode;
}

auto LinController::Status() const noexcept -> LinControllerStatus
{
    return _controllerStatus;
}

void LinController::SendFrameInternal(LinFrame frame, LinFrameResponseType responseType)
{
    if (responseType == LinFrameResponseType::MasterResponse)
    {
        // Update local response reconfiguration
        LinFrameResponse response{};
        response.frame = frame;
        response.responseMode = LinFrameResponseMode::TxUnconditional;
        UpdateFrameResponse(response);
    }
    else
    {
        // Only allow SendFrame of unconfigured LIN Ids for LinFrameResponseType::MasterResponse
        // that LinSlaveConfigurationHandler and GetSlaveConfiguration stays valid.
        if (!HasRespondingSlave(frame.id) && !HasDynamicNode())
        {
            WarnOnUnconfiguredSlaveResponse(frame.id);
            CallLinFrameStatusEventHandler(
                LinFrameStatusEvent{_timeProvider->Now(), frame, LinFrameStatus::LIN_RX_NO_RESPONSE});
            return;
        }

        if (responseType == LinFrameResponseType::SlaveResponse)
        {
            // As the master, we configure for RX in case of unconfigured SlaveResponse
            auto currentResponseMode = GetThisLinNode().responses[frame.id].responseMode;
            if (currentResponseMode == LinFrameResponseMode::Unused)
            {
                std::vector<LinFrameResponse> responseUpdate;
                LinFrameResponse response{};
                response.frame = frame;
                response.responseMode = LinFrameResponseMode::Rx;
                UpdateFrameResponse(response);
            }
            else if (currentResponseMode == LinFrameResponseMode::TxUnconditional)
            {
                WarnOnSendFrameSlaveResponseWithMasterTx(frame.id);
                return;
            }
        }
        else if (responseType == LinFrameResponseType::SlaveToSlave)
        {
            CallLinFrameStatusEventHandler(
                LinFrameStatusEvent{_timeProvider->Now(), frame, LinFrameStatus::LIN_TX_OK});
        }
    }

    // Detailed: Send LinSendFrameRequest to BusSim
    // Trivial: SendFrameHeader
    SendMsg(LinSendFrameRequest{frame, responseType});
}

void LinController::SendFrame(LinFrame frame, LinFrameResponseType responseType)
{
    ThrowIfUninitialized(__FUNCTION__);
    ThrowIfNotMaster(__FUNCTION__);
    ThrowIfDynamic(__FUNCTION__);

    if (Tracing::IsReplayEnabledFor(_config.replay, Config::Replay::Direction::Send))
    {
        Logging::Debug(_logger, _logOnce,
            "LinController: Ignoring SendFrame API call due to Replay config on {}", _config.name);
        return;
    }

    SendFrameInternal(std::move(frame), responseType);
}

void LinController::SendFrameHeader(LinId linId)
{
    ThrowIfUninitialized(__FUNCTION__);
    ThrowIfNotMaster(__FUNCTION__);

    // Detailed: Send LinSendFrameHeaderRequest to BusSim
    // Trivial: Good case (numResponses == 1): Distribute LinSendFrameHeaderRequest, the receiving Tx-Node will generate the LinTransmission.
    //          Error case: Generate the LinTransmission and trigger a FrameStatusUpdate with 
    //                      LIN_RX_NO_RESPONSE (numResponses == 0) or LIN_RX_ERROR (numResponses > 1).
    SendMsg(LinSendFrameHeaderRequest{_timeProvider->Now(), linId});
}

void LinController::UpdateTxBuffer(LinFrame frame)
{
    ThrowIfUninitialized(__FUNCTION__);
    ThrowIfDynamic(__FUNCTION__);
    ThrowIfNotConfiguredTxUnconditional(frame.id);

    // Update the local payload
    GetThisLinNode().UpdateTxBuffer(frame.id, std::move(frame.data), _logger);

    // Detailed: Send LinFrameResponseUpdate with updated payload to BusSim
    // Trivial: Nop
    _simulationBehavior.UpdateTxBuffer(frame);
}

void LinController::SetFrameResponse(LinFrameResponse response)
{
    ThrowIfUninitialized(__FUNCTION__);
    ThrowIfDynamic(__FUNCTION__);

    if (response.frame.id >= _maxLinId)
    {
        WarnOnInvalidLinId(response.frame.id, __FUNCTION__);
        return;
    }
    if (response.responseMode == LinFrameResponseMode::Unused)
    {
        WarnOnUnusedResponseMode(__FUNCTION__);
        return;
    }
    // Don't allow reconfiguration
    auto currentResponseMode = GetThisLinNode().responses[response.frame.id].responseMode;
    if (currentResponseMode != LinFrameResponseMode::Unused)
    {
        WarnOnResponseModeReconfiguration(response.frame.id, currentResponseMode);
        return;
    }

    if (Tracing::IsReplayEnabledFor(_config.replay, Config::Replay::Direction::Send))
    {
        Logging::Debug(_logger, _logOnce,
            "LinController: Ignoring SetFrameResponse API call due to Replay config on {}", _config.name);
        return;
    }

    UpdateFrameResponse(response);
}

void LinController::UpdateFrameResponse(LinFrameResponse response)
{
    // Local update
    GetThisLinNode().UpdateResponses({response}, _logger);

    // Distribute update
    LinFrameResponseUpdate responseUpdate{};
    responseUpdate.frameResponses.push_back(response);
    SendMsg(responseUpdate);
}

void LinController::GoToSleep()
{
    ThrowIfUninitialized(__FUNCTION__);
    ThrowIfNotMaster(__FUNCTION__);

    if (Tracing::IsReplayEnabledFor(_config.replay, Config::Replay::Direction::Send))
    {
        Logging::Debug(_logger, _logOnce,
            "LinController: Ignoring GoToSleep API call due to Replay config on {}", _config.name);
        return;
    }

    // Detailed: Send LinSendFrameRequest with GoToSleep-Frame and set LinControllerStatus::SleepPending. BusSim will trigger LinTransmission.
    // Trivial: Directly send LinTransmission with GoToSleep-Frame and call GoToSleepInternal() on this controller.
    _simulationBehavior.GoToSleep();

    _controllerStatus = LinControllerStatus::Sleep;
}

void LinController::GoToSleepInternal()
{
    ThrowIfUninitialized(__FUNCTION__);

    SetControllerStatusInternal(LinControllerStatus::Sleep);
}

void LinController::Wakeup()
{
    ThrowIfUninitialized(__FUNCTION__);

    // Detailed: Send LinWakeupPulse and call WakeupInternal()
    // Trivial: Send LinWakeupPulse and call WakeupInternal(), self-deliver LinWakeupPulse with TX
    _simulationBehavior.Wakeup();
}

void LinController::WakeupInternal()
{
    ThrowIfUninitialized(__FUNCTION__);

    SetControllerStatusInternal(LinControllerStatus::Operational);
}

Experimental::Services::Lin::LinSlaveConfiguration LinController::GetSlaveConfiguration()
{
    ThrowIfNotMaster(__FUNCTION__);

    return Experimental::Services::Lin::LinSlaveConfiguration{_linIdsRespondedBySlaves};
}

//------------------------
// Helpers
//------------------------

bool LinController::HasRespondingSlave(LinId id)
{
    auto it = std::find(_linIdsRespondedBySlaves.begin(), _linIdsRespondedBySlaves.end(), id);
    const bool result = it != _linIdsRespondedBySlaves.end();
    return result;
}

bool LinController::HasDynamicNode()
{
    const auto it = std::find_if(_linNodes.begin(), _linNodes.end(), [](const LinNode& node) {
        return node.simulationMode == WireLinControllerConfig::SimulationMode::Dynamic;
    });
    const bool result = it != _linNodes.end();
    return result;
}

void LinController::UpdateLinIdsRespondedBySlaves(const std::vector<LinFrameResponse>& responsesUpdate)
{
    for (auto&& response : responsesUpdate)
    {
        if (response.responseMode == LinFrameResponseMode::TxUnconditional)
        {
            if (!HasRespondingSlave(response.frame.id))
            {
                _linIdsRespondedBySlaves.push_back(response.frame.id);
            }
        }
    }
}

void LinController::SetControllerStatusInternal(LinControllerStatus status)
{
    if (_controllerStatus == status)
    {
        WarnOnUnneededStatusChange(status);
    }

    _controllerStatus = status;

    LinControllerStatusUpdate msg;
    msg.status = status;

    SendMsg(msg);
}

void LinController::HandleResponsesUpdate(const IServiceEndpoint* from,
                                          const std::vector<LinFrameResponse>& responsesToUpdate)
{
    auto& linNode = GetLinNode(from->GetServiceDescriptor().to_endpointAddress());
    linNode.UpdateResponses(responsesToUpdate, _logger);

    if (linNode.controllerMode == LinControllerMode::Slave)
    {
        UpdateLinIdsRespondedBySlaves(responsesToUpdate);
    }
    auto& callbacks = std::get<CallbacksT<Experimental::Services::Lin::LinSlaveConfigurationEvent>>(_callbacks);
    auto receptionTime = _timeProvider->Now();
    if (callbacks.Size() == 0)
    {
        // No handlers yet, but received a LinSlaveConfiguration -> trigger upon handler addition
        _triggerLinSlaveConfigurationHandlers = true;
        _receptionTimeLinSlaveConfiguration = receptionTime;
    }
    else
    {
        CallHandlers(Experimental::Services::Lin::LinSlaveConfigurationEvent{receptionTime});
    }
}

//------------------------
// Node bookkeeping
//------------------------

void LinController::LinNode::UpdateResponses(std::vector<LinFrameResponse> responsesToUpdate, Services::Logging::ILogger* logger)
{
    for (auto&& response : responsesToUpdate)
    {
        auto linId = response.frame.id;
        if (linId >= responses.size())
        {
            Logging::Warn(logger, "Ignoring LinFrameResponse update for invalid ID={}", static_cast<uint16_t>(linId));
            continue;
        }
        responses[linId] = std::move(response);
   }
}

void LinController::LinNode::UpdateTxBuffer(LinId linId, std::array<uint8_t, 8> data,
                                            Services::Logging::ILogger* logger)
{
    if (linId >= responses.size())
    {
        Logging::Warn(logger, "Ignoring LinFrameResponse update for invalid ID={}", static_cast<uint16_t>(linId));
        return;
    }
    responses[linId].frame.data = data;
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

auto LinController::GetResponse(LinId id) -> std::pair<int, LinFrame>
{
    LinFrame responseFrame;
    responseFrame.id = id;

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
// ReceiveMsg
//------------------------

void LinController::ReceiveMsg(const IServiceEndpoint* from, const LinSendFrameHeaderRequest& msg)
{
    if (!AllowReception(from))
    {
        return;
    }

    if (_useDynamicResponse)
    {
        SilKit::Experimental::Services::Lin::LinFrameHeaderEvent headerEvent{};
        headerEvent.id = msg.id;
        headerEvent.timestamp = msg.timestamp;
        CallHandlers(headerEvent);
        return;
    }

    // Detailed: Depends on how LinSendFrameHeaderRequest will work with BusSim, currently NOP
    // Trivial: Generate LinTransmission
    // In future: Also Trigger OnHeaderCallback
    _simulationBehavior.ProcessFrameHeaderRequest(msg);
}

void LinController::ReceiveMsg(const IServiceEndpoint* from, const LinTransmission& msg)
{
    if (!AllowReception(from))
    {
        return;
    }

    if (_controllerMode == LinControllerMode::Inactive)
    {
        WarnOnReceptionWhileInactive();
        return;
    }

    const auto& frame = msg.frame;
    bool isGoToSleepFrame = frame.id == GoToSleepFrame().id && frame.data == GoToSleepFrame().data;

    if (frame.dataLength != LinDataLengthUnknown && frame.dataLength > _maxDataLength)
    {
        WarnOnReceptionWithInvalidDataLength(frame.dataLength, from->GetServiceDescriptor().GetParticipantName(),
                                             from->GetServiceDescriptor().GetServiceName());
        return;
    }

    if (frame.id >= _maxLinId)
    {
        WarnOnReceptionWithInvalidLinId(frame.id, from->GetServiceDescriptor().GetParticipantName(),
                                             from->GetServiceDescriptor().GetServiceName());
        return;
    }

    _tracer.Trace(SilKit::Services::TransmitDirection::RX, msg.timestamp, frame);


    // Detailed: Just use msg.status
    // Trivial: Evaluate status using cached response
    const LinFrameStatus msgStatus = _simulationBehavior.CalcFrameStatus(msg, isGoToSleepFrame);

    if (msgStatus != LinFrameStatus::NOT_OK)
    {
        // Dispatch frame to handlers
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

void LinController::ReceiveMsg(const IServiceEndpoint* from, const LinWakeupPulse& msg)
{
    if (!AllowReception(from))
    {
        return;
    }

    CallHandlers(LinWakeupEvent{msg.timestamp, msg.direction});
}

void LinController::ReceiveMsg(const IServiceEndpoint* from, const LinFrameResponseUpdate& msg)
{
    // Self-delivered messages are rejected
    if (from->GetServiceDescriptor() == _serviceDescriptor)
        return;

    HandleResponsesUpdate(from, msg.frameResponses);
}

void LinController::ReceiveMsg(const IServiceEndpoint* from, const WireLinControllerConfig& msg)
{
    // Self-delivered messages are rejected
    if (from->GetServiceDescriptor() == _serviceDescriptor)
        return;

    auto& linNode = GetLinNode(from->GetServiceDescriptor().to_endpointAddress());
    linNode.controllerMode = msg.controllerMode;
    linNode.controllerStatus = LinControllerStatus::Operational;
    linNode.simulationMode = msg.simulationMode;

    HandleResponsesUpdate(from, msg.frameResponses);
}

void LinController::ReceiveMsg(const IServiceEndpoint* from, const LinControllerStatusUpdate& msg)
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

auto LinController::AddFrameHeaderHandler(SilKit::Experimental::Services::Lin::LinFrameHeaderHandler handler ) -> HandlerId
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

void LinController::RemoveFrameHeaderHandler(HandlerId handlerId)
{
    if (!RemoveHandler<SilKit::Experimental::Services::Lin::LinFrameHeaderEvent>(handlerId))
    {
        _participant->GetLogger()->Warn("RemoveFrameHeaderHandler failed: Unknown HandlerId.");
    }
}

HandlerId LinController::AddLinSlaveConfigurationHandler(
    Experimental::Services::Lin::LinSlaveConfigurationHandler handler)
{
    auto handlerId = AddHandler(std::move(handler));

    // Trigger handler if a LinSlaveConfigurations was received before adding a handler
    // No need to cache the LinSlaveConfigs (just the reception time), 
    // as the user has to actively call GetSlaveConfiguration in the callback
    if (_triggerLinSlaveConfigurationHandlers)
    {
        _triggerLinSlaveConfigurationHandlers = false;
        CallHandlers(Experimental::Services::Lin::LinSlaveConfigurationEvent{_receptionTimeLinSlaveConfiguration});
    }
    return handlerId;
}

void LinController::RemoveLinSlaveConfigurationHandler(HandlerId handlerId)
{
    if (!RemoveHandler<Experimental::Services::Lin::LinSlaveConfigurationEvent>(handlerId))
    {
        _participant->GetLogger()->Warn("RemoveLinSlaveConfigurationHandler failed: Unknown HandlerId.");
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


// IReplayDataProvider

void LinController::ReplayMessage(const IReplayMessage* replayMessage)
{

    if (!_replayActive)
    {
        return;
    }
 
    if (_controllerMode != LinControllerMode::Master)
    {
        Logging::Debug(_logger, "ReplayMessage: skipping, because controller mode is {}", _controllerMode);
        return;
    }
    // The LinFrame Response Updates ensures that all controllers have the same notion of the
    // response that is going to be generated by a slave.

    auto&& frame = dynamic_cast<const LinFrame&>(*replayMessage);
    const auto isSleepFrame = (frame.id == GoToSleepFrame().id
        && frame.data == GoToSleepFrame().data);
    const auto isReceive = replayMessage->GetDirection() == TransmitDirection::RX;
    LinTransmission tm{};

    tm.timestamp = replayMessage->Timestamp();
    tm.frame = std::move(frame);
    tm.status = isReceive ? LinFrameStatus::LIN_RX_OK : LinFrameStatus::LIN_TX_OK;

    // ensure slave responses are updated locally
    LinFrameResponse response;
    response.frame = tm.frame;
    response.responseMode = isReceive ? LinFrameResponseMode::Rx : LinFrameResponseMode::TxUnconditional;
    UpdateFrameResponse(response);

    if (isSleepFrame)
    {
        _simulationBehavior.GoToSleep();
        _controllerStatus = LinControllerStatus::Sleep;
        return;
    }

    //broadcast to slaves
    auto responseType = isReceive ? LinFrameResponseType::SlaveResponse : LinFrameResponseType::MasterResponse;
    SendFrameInternal(tm.frame, responseType);
}

} // namespace Lin
} // namespace Services
} // namespace SilKit
