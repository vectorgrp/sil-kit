// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <map>

#include "silkit/services/lin/ILinController.hpp"
#include "silkit/core/fwd_decl.hpp"

#include "ITimeConsumer.hpp"
#include "IParticipantInternal.hpp"
#include "ITraceMessageSource.hpp"
#include "ParticipantConfiguration.hpp"
#include "IMsgForLinController.hpp"
#include "SimBehavior.hpp"
#include "SynchronizedHandlers.hpp"

namespace SilKit {
namespace Services {
namespace Lin {

class LinController
    : public ILinController
    , public IMsgForLinController
    , public ITraceMessageSource
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
                   Core::Orchestration::ITimeProvider* timeProvider);

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
    auto Status() const noexcept->LinControllerStatus override;

    void SendFrame(LinFrame frame, LinFrameResponseType responseType) override;
    void SendFrameHeader(LinIdT linId) override;
    void SetFrameResponse(LinFrame frame, LinFrameResponseMode mode) override;
    void SetFrameResponses(std::vector<LinFrameResponse> responses) override;

    void GoToSleep() override;
    void GoToSleepInternal() override;
    void Wakeup() override;
    void WakeupInternal() override;

    HandlerId AddFrameStatusHandler(FrameStatusHandler handler) override;
    HandlerId AddGoToSleepHandler(GoToSleepHandler handler) override;
    HandlerId AddWakeupHandler(WakeupHandler handler) override;
    HandlerId AddFrameResponseUpdateHandler(FrameResponseUpdateHandler handler) override;

    void RemoveFrameStatusHandler(HandlerId handlerId) override;
    void RemoveGoToSleepHandler(HandlerId handlerId) override;
    void RemoveWakeupHandler(HandlerId handlerId) override;
    void RemoveFrameResponseUpdateHandler(HandlerId handlerId) override;

    // IMsgForLinController
    void ReceiveSilKitMessage(const IServiceEndpoint* from, const LinTransmission& msg) override;
    void ReceiveSilKitMessage(const IServiceEndpoint* from, const LinWakeupPulse& msg) override;
    void ReceiveSilKitMessage(const IServiceEndpoint* from, const LinControllerConfig& msg) override;
    void ReceiveSilKitMessage(const IServiceEndpoint* from, const LinFrameResponseUpdate& msg) override;
    void ReceiveSilKitMessage(const IServiceEndpoint* from, const LinControllerStatusUpdate& msg) override;

public:
    // ----------------------------------------
    // Public inline interface methods

    //ITraceMessageSource
    inline void AddSink(ITraceMessageSink* sink) override;

    // IServiceEndpoint
    inline void SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const Core::ServiceDescriptor& override;

public:
    // ----------------------------------------
    // Public methods
    void SetControllerStatus(LinControllerStatus status);

    struct LinNode
    {
        Core::EndpointAddress address{};
        LinControllerMode controllerMode{LinControllerMode::Inactive};
        LinControllerStatus controllerStatus{LinControllerStatus::Unknown};
        std::array<LinFrameResponse, 64> responses;

        void UpdateResponses(std::vector<LinFrameResponse> responses_, Core::Logging::ILogger* logger);
    };
    auto GetResponse(LinIdT id) -> std::pair<int, LinFrame>;
    auto GetThisLinNode() -> LinNode&;
    auto GetLinNode(Core::EndpointAddress addr) -> LinNode&;

    void CallLinFrameStatusEventHandler(const LinFrameStatusEvent& msg);


    void RegisterServiceDiscovery();
    // Expose the simulated/trivial mode for unit tests
    void SetDetailedBehavior(const Core::ServiceDescriptor& remoteServiceDescriptor);
    void SetTrivialBehavior();

private:
    // ----------------------------------------
    // private methods

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

private:
    // ----------------------------------------
    // private members
    Core::IParticipantInternal* _participant;
    Config::LinController _config;
    Core::Logging::ILogger* _logger;
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
        CallbacksT<LinFrameResponseUpdateEvent>
    > _callbacks;

    Tracer _tracer;

    std::vector<LinNode> _linNodes;
};

// ==================================================================
//  Inline Implementations
// ==================================================================

void LinController::AddSink(ITraceMessageSink* sink)
{
    _tracer.AddSink(Core::EndpointAddress{}, *sink);
}
void LinController::SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}
auto LinController::GetServiceDescriptor() const -> const Core::ServiceDescriptor&
{
    return _serviceDescriptor;
}
} // namespace Lin
} // namespace Services
} // namespace SilKit
