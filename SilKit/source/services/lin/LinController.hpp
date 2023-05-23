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

#pragma once

#include <map>
#include <set>

#include "silkit/services/lin/ILinController.hpp"
#include "silkit/experimental/services/lin/LinDatatypesExtensions.hpp"

#include "ILinControllerExtensions.hpp"
#include "ITimeConsumer.hpp"
#include "IParticipantInternal.hpp"
#include "ITraceMessageSource.hpp"
#include "IReplayDataController.hpp"
#include "ParticipantConfiguration.hpp"
#include "IMsgForLinController.hpp"
#include "SimBehavior.hpp"
#include "SynchronizedHandlers.hpp"
#include "WireLinMessages.hpp"
#include "ILogger.hpp"

namespace SilKit {
namespace Services {
namespace Lin {

class LinController
    : public ILinController
    , public ILinControllerExtensions
    , public IMsgForLinController
    , public ITraceMessageSource
    , public Tracing::IReplayDataController
    , public Core::IServiceEndpoint
{
public:
    // ----------------------------------------
    // Public Data Types

public:
    // ----------------------------------------
    // Constructors and Destructor
    LinController() = delete;
    LinController(const LinController&) = delete;
    LinController(LinController&&) = delete;
    LinController(Core::IParticipantInternal* participant, Config::LinController config,
                   Services::Orchestration::ITimeProvider* timeProvider);

public:
    // ----------------------------------------
    // Operator Implementations
    LinController& operator=(LinController& other) = delete;
    LinController& operator=(LinController&& other) = delete;

public:
    // ----------------------------------------
    // Public interface methods
    //
    // ILinController
    void Init(LinControllerConfig config) override;
    void InitDynamic(const SilKit::Experimental::Services::Lin::LinControllerDynamicConfig& config) override; // Experimental

    auto Status() const noexcept->LinControllerStatus override;

    void SendFrame(LinFrame frame, LinFrameResponseType responseType) override;
    void SendFrameHeader(LinId linId) override;

    void SendDynamicResponse(const LinFrame& frame) override; // Experimental

    void UpdateTxBuffer(LinFrame frame) override;
    void SetFrameResponse(LinFrameResponse response) override;

    void GoToSleep() override;
    void GoToSleepInternal() override;
    void Wakeup() override;
    void WakeupInternal() override;

    Experimental::Services::Lin::LinSlaveConfiguration GetSlaveConfiguration() override; // Exprimental
    HandlerId AddLinSlaveConfigurationHandler(
        Experimental::Services::Lin::LinSlaveConfigurationHandler handler) override; // Exprimental
    void RemoveLinSlaveConfigurationHandler(HandlerId handlerId) override; // Exprimental

    auto AddFrameHeaderHandler(SilKit::Experimental::Services::Lin::LinFrameHeaderHandler handler) -> HandlerId override; // Experimental
    void RemoveFrameHeaderHandler(HandlerId handlerId) override; // Experimental

    HandlerId AddFrameStatusHandler(FrameStatusHandler handler) override;
    HandlerId AddGoToSleepHandler(GoToSleepHandler handler) override;
    HandlerId AddWakeupHandler(WakeupHandler handler) override;

    void RemoveFrameStatusHandler(HandlerId handlerId) override;
    void RemoveGoToSleepHandler(HandlerId handlerId) override;
    void RemoveWakeupHandler(HandlerId handlerId) override;

    // IMsgForLinController
    void ReceiveMsg(const IServiceEndpoint* from, const LinTransmission& msg) override;
    void ReceiveMsg(const IServiceEndpoint* from, const LinWakeupPulse& msg) override;
    void ReceiveMsg(const IServiceEndpoint* from, const WireLinControllerConfig& msg) override;
    void ReceiveMsg(const IServiceEndpoint* from, const LinControllerStatusUpdate& msg) override;
    void ReceiveMsg(const IServiceEndpoint* from, const LinSendFrameHeaderRequest& msg) override;
    void ReceiveMsg(const IServiceEndpoint* from, const LinFrameResponseUpdate& msg) override;

public:
    // ----------------------------------------
    // Public inline interface methods

    //ITraceMessageSource
    inline void AddSink(ITraceMessageSink* sink, SilKit::Config::NetworkType networkType) override;

    // IReplayDataProvider
    void ReplayMessage(const IReplayMessage* replayMessage) override;

    // IServiceEndpoint
    inline void SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const Core::ServiceDescriptor& override;

public:
    // ----------------------------------------
    // Public methods
    void SetControllerStatusInternal(LinControllerStatus status);

    struct LinNode
    {
        Core::EndpointAddress address{};
        LinControllerMode controllerMode{LinControllerMode::Inactive};
        LinControllerStatus controllerStatus{LinControllerStatus::Unknown};
        WireLinControllerConfig::SimulationMode simulationMode{WireLinControllerConfig::SimulationMode::Default};
        std::array<LinFrameResponse, 64> responses;

        void UpdateResponses(std::vector<LinFrameResponse> responsesToUpdate, Services::Logging::ILogger* logger);
        void UpdateTxBuffer(LinId linId, std::array<uint8_t, 8> data, Services::Logging::ILogger* logger);

    };
    auto GetResponse(LinId id) -> std::pair<int, LinFrame>;
    auto GetThisLinNode() -> LinNode&;
    auto GetLinNode(Core::EndpointAddress addr) -> LinNode&;
    auto Mode() const noexcept -> LinControllerMode;

    void CallLinFrameStatusEventHandler(const LinFrameStatusEvent& msg);

    void RegisterServiceDiscovery();
    // Expose the simulated/trivial mode for unit tests
    void SetDetailedBehavior(const Core::ServiceDescriptor& remoteServiceDescriptor);
    void SetTrivialBehavior();

    // Public error handling for use in simulationBehavior
    void WarnOnWrongDataLength(const LinFrame& receivedFrame, const LinFrame& configuredFrame) const;
    void WarnOnWrongChecksum(const LinFrame& receivedFrame, const LinFrame& configuredFrame) const;
    void ThrowOnSendAttemptWithUndefinedChecksum(const LinFrame& frame) const;
    void ThrowOnSendAttemptWithUndefinedDataLength(const LinFrame& frame) const;

    // for ITraceMessageSource
    inline auto GetTracer() -> Tracer*;
private:
    // ----------------------------------------
    // private methods

    // allow using the SendFrame API from ReplayMessage
    void SendFrameInternal(LinFrame frame, LinFrameResponseType responseType);

    template <typename MsgT>
    HandlerId AddHandler(CallbackT<MsgT> handler);

    template <typename MsgT>
    auto RemoveHandler(HandlerId handlerId) -> bool;

    template <typename MsgT>
    void CallHandlers(const MsgT& msg);

    template <typename MsgT>
    inline void SendMsg(MsgT&& msg);

    auto IsRelevantNetwork(const Core::ServiceDescriptor& remoteServiceDescriptor) const -> bool;
    auto AllowReception(const IServiceEndpoint* from) const -> bool;

    void ThrowOnErroneousInitialization() const;
    void ThrowOnDuplicateInitialization() const;
    void ThrowIfUninitialized(const std::string& callingMethodName) const;
    void ThrowIfNotMaster(const std::string& callingMethodName) const;
    void ThrowIfDynamic(const std::string& callingMethodName) const;
    void ThrowIfNotDynamic(const std::string& callingMethodName) const;
    void ThrowIfNotConfiguredTxUnconditional(LinId linId);
    void WarnOnReceptionWithInvalidDataLength(LinDataLength invalidDataLength, const std::string& fromParticipantName,
                                              const std::string& fromServiceName) const;
    void WarnOnReceptionWithInvalidLinId(LinId invalidLinId, const std::string& fromParticipantName,
                                         const std::string& fromServiceName) const;
    void WarnOnReceptionWhileInactive() const;
    void WarnOnUnneededStatusChange(LinControllerStatus status) const;
    void WarnOnInvalidLinId(LinId invalidLinId, const std::string& callingMethodName) const;
    void WarnOnUnusedResponseMode(const std::string& callingMethodName) const;
    void WarnOnResponseModeReconfiguration(LinId id, LinFrameResponseMode currentResponseMode) const;
    void WarnOnUnconfiguredSlaveResponse(LinId id) const;
    void WarnOnSendFrameSlaveResponseWithMasterTx(LinId id) const;
    
    void UpdateLinIdsRespondedBySlaves(const std::vector<LinFrameResponse>& responsesUpdate);
    void HandleResponsesUpdate(const IServiceEndpoint* from, const std::vector<LinFrameResponse>& responsesToUpdate);
    void UpdateFrameResponse(LinFrameResponse response);

    bool HasRespondingSlave(LinId id);

public:
    bool HasDynamicNode();

private:
    // ----------------------------------------
    // private members
    Logging::LogOnceFlag _logOnce;
    Core::IParticipantInternal* _participant;
    Config::LinController _config;
    Services::Logging::ILogger* _logger;
    Tracer _tracer;
    SimBehavior _simulationBehavior;
    ::SilKit::Core::ServiceDescriptor _serviceDescriptor;

    LinControllerMode   _controllerMode{LinControllerMode::Inactive};
    LinControllerStatus _controllerStatus{LinControllerStatus::Unknown};

    template <typename MsgT>
    using CallbacksT = Util::SynchronizedHandlers<CallbackT<MsgT>>;

    std::tuple<
        CallbacksT<LinFrameStatusEvent>,
        CallbacksT<LinGoToSleepEvent>,
        CallbacksT<LinWakeupEvent>,
        CallbacksT<Experimental::Services::Lin::LinSlaveConfigurationEvent>,
        CallbacksT<SilKit::Experimental::Services::Lin::LinFrameHeaderEvent>
    > _callbacks;

    Services::Orchestration::ITimeProvider* _timeProvider{nullptr};
    bool _replayActive{false};

    std::vector<LinNode> _linNodes;
    std::vector<LinId> _linIdsRespondedBySlaves{}; // Global view of LinIds with TxUnconditional configured on any node.
    bool _triggerLinSlaveConfigurationHandlers{false};
    std::chrono::nanoseconds _receptionTimeLinSlaveConfiguration{};

    const LinId _maxDataLength = 8;
    const LinId _maxLinId = 64;

    // DynamicResponses: no preallocated FrameResponses
    std::chrono::nanoseconds _receptionTimeFrameHeader{std::chrono::nanoseconds::min()};
    bool _useDynamicResponse{false};
};

// ==================================================================
//  Inline Implementations
// ==================================================================

void LinController::AddSink(ITraceMessageSink* sink, SilKit::Config::NetworkType /*networkType*/)
{
    _tracer.AddSink(GetServiceDescriptor(), *sink);
}
void LinController::SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}
auto LinController::GetServiceDescriptor() const -> const Core::ServiceDescriptor&
{
    return _serviceDescriptor;
}
auto LinController::GetTracer() -> Tracer*
{
    return &_tracer;
}
} // namespace Lin
} // namespace Services
} // namespace SilKit
