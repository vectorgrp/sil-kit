// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <memory>
#include <map>

#include "ib/sim/lin/ILinController.hpp"
#include "ib/mw/fwd_decl.hpp"
#include "ITimeConsumer.hpp"

#include "IParticipantInternal.hpp"
#include "ITraceMessageSource.hpp"
#include "ParticipantConfiguration.hpp"
#include "IIbToLinController.hpp"
#include "SimBehavior.hpp"

#include "SynchronizedHandlers.hpp"

namespace ib {
namespace sim {
namespace lin {

class LinController
    : public ILinController
    , public IIbToLinController
    , public extensions::ITraceMessageSource
    , public mw::IIbServiceEndpoint
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
    LinController(mw::IParticipantInternal* participant, cfg::LinController config,
                   mw::sync::ITimeProvider* timeProvider);

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

    // IIbToLinController
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const LinTransmission& msg) override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const LinWakeupPulse& msg) override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const LinControllerConfig& msg) override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const LinFrameResponseUpdate& msg) override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const LinControllerStatusUpdate& msg) override;

public:
    // ----------------------------------------
    // Public inline interface methods

    //ITraceMessageSource
    inline void AddSink(extensions::ITraceMessageSink* sink) override;

    // IIbServiceEndpoint
    inline void SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const mw::ServiceDescriptor& override;

public:
    // ----------------------------------------
    // Public methods
    void SetControllerStatus(LinControllerStatus status);

    struct LinNode
    {
        mw::EndpointAddress ibAddress{};
        LinControllerMode controllerMode{LinControllerMode::Inactive};
        LinControllerStatus controllerStatus{LinControllerStatus::Unknown};
        std::array<LinFrameResponse, 64> responses;

        void UpdateResponses(std::vector<LinFrameResponse> responses_, mw::logging::ILogger* logger);
    };
    auto GetResponse(LinIdT id) -> std::pair<int, LinFrame>;
    auto GetThisLinNode() -> LinNode&;
    auto GetLinNode(mw::EndpointAddress addr) -> LinNode&;

    void CallLinFrameStatusEventHandler(const LinFrameStatusEvent& msg);


    void RegisterServiceDiscovery();
    // Expose the simulated/trivial mode for unit tests
    void SetDetailedBehavior(const mw::ServiceDescriptor& remoteServiceDescriptor);
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
    inline void SendIbMessage(MsgT&& msg);

    auto IsRelevantNetwork(const mw::ServiceDescriptor& remoteServiceDescriptor) const -> bool;
    auto AllowReception(const IIbServiceEndpoint* from) const -> bool;

private:
    // ----------------------------------------
    // private members
    mw::IParticipantInternal* _participant;
    cfg::LinController _config;
    mw::logging::ILogger* _logger;
    SimBehavior _simulationBehavior;
    ::ib::mw::ServiceDescriptor _serviceDescriptor;

    LinControllerMode   _controllerMode{LinControllerMode::Inactive};
    LinControllerStatus _controllerStatus{LinControllerStatus::Unknown};

    template <typename MsgT>
    using CallbacksT = util::SynchronizedHandlers<CallbackT<MsgT>>;

    std::tuple<
        CallbacksT<LinFrameStatusEvent>,
        CallbacksT<LinGoToSleepEvent>,
        CallbacksT<LinWakeupEvent>,
        CallbacksT<LinFrameResponseUpdateEvent>
    > _callbacks;

    extensions::Tracer _tracer;

    std::vector<LinNode> _linNodes;
};

// ==================================================================
//  Inline Implementations
// ==================================================================

void LinController::AddSink(extensions::ITraceMessageSink* sink)
{
    _tracer.AddSink(mw::EndpointAddress{}, *sink);
}
void LinController::SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}
auto LinController::GetServiceDescriptor() const -> const mw::ServiceDescriptor&
{
    return _serviceDescriptor;
}
} // namespace lin
} // namespace sim
} // namespace ib
