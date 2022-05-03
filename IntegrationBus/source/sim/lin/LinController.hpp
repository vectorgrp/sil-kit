// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once


#include <algorithm>
#include <memory>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "ib/sim/lin/ILinController.hpp"
#include "ib/mw/sync/ITimeConsumer.hpp"

#include "ib/mw/fwd_decl.hpp"
#include "ib/sim/datatypes.hpp"

#include "IIbToLinController.hpp"
#include "IParticipantInternal.hpp"
#include "IIbServiceEndpoint.hpp"
#include "ITraceMessageSource.hpp"

#include "ParticipantConfiguration.hpp"

namespace ib {
namespace sim {
namespace lin {

class LinController
    : public ILinController
    , public IIbToLinController
    , public mw::sync::ITimeConsumer
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
    LinController(const LinController&) = default;
    LinController(LinController&&) = default;
    LinController(mw::IParticipantInternal* participant, mw::sync::ITimeProvider* timeProvider, ILinController* facade = nullptr);

public:
    // ----------------------------------------
    // Operator Implementations
    LinController& operator=(LinController& other) = default;
    LinController& operator=(LinController&& other) = default;

public:
    // ----------------------------------------
    // Public interface methods
    //
    // ILinController
    void Init(ControllerConfig config) override;
    auto Status() const noexcept -> ControllerStatus override;

    void SendFrame(LinFrame frame, FrameResponseType responseType) override;
    void SendFrameHeader(LinIdT linId) override;
    void SetFrameResponse(LinFrame frame, FrameResponseMode mode) override;
    void SetFrameResponses(std::vector<FrameResponse> responses) override;

    void GoToSleep() override;
    void GoToSleepInternal() override;
    void Wakeup() override;
    void WakeupInternal() override;

    void AddFrameStatusHandler(FrameStatusHandler handler) override;
    void AddGoToSleepHandler(GoToSleepHandler handler) override;
    void AddWakeupHandler(WakeupHandler handler) override;
    void AddFrameResponseUpdateHandler(FrameResponseUpdateHandler handler) override;

     // IIbToLinController
     void ReceiveIbMessage(const IIbServiceEndpoint* from, const Transmission& msg) override;
     void ReceiveIbMessage(const IIbServiceEndpoint* from, const WakeupPulse& msg) override;
     void ReceiveIbMessage(const IIbServiceEndpoint* from, const ControllerConfig& msg) override;
     void ReceiveIbMessage(const IIbServiceEndpoint* from, const ControllerStatusUpdate& msg) override;
     void ReceiveIbMessage(const IIbServiceEndpoint* from, const FrameResponseUpdate& msg) override;

     //ib::mw::sync::ITimeConsumer
     void SetTimeProvider(mw::sync::ITimeProvider* timeProvider) override;

    // ITraceMessageSource
    inline void AddSink(extensions::ITraceMessageSink* sink) override;

    // IIbServiceEndpoint
    inline void SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const mw::ServiceDescriptor & override;

private:
    // ----------------------------------------
    // private data types
    struct LinNode
    {
        mw::EndpointAddress           ibAddress;
        ControllerMode                controllerMode{ControllerMode::Inactive};
        ControllerStatus              controllerStatus{ControllerStatus::Unknown};
        std::array<FrameResponse, 64> responses;

        void UpdateResponses(std::vector<FrameResponse> responses_, mw::logging::ILogger* logger);
    };

private:
    // ----------------------------------------
    // private methods
    void SetControllerStatus(ControllerStatus status);
    auto VeriyChecksum(const LinFrame& frame, FrameStatus status) -> FrameStatus;

    template <typename MsgT>
    inline void SendIbMessage(MsgT&& msg);

    inline auto GetLinNode(mw::EndpointAddress addr) -> LinNode&;

private:
    // ----------------------------------------
    // private members
    mw::IParticipantInternal* _participant;
    ::ib::mw::ServiceDescriptor _serviceDescriptor;
    mw::logging::ILogger* _logger;
    mw::sync::ITimeProvider* _timeProvider{ nullptr };
    ILinController* _facade{ nullptr };

    ControllerMode   _controllerMode{ControllerMode::Inactive};
    ControllerStatus _controllerStatus{ControllerStatus::Unknown};

    std::vector<LinNode> _linNodes;

    std::vector<FrameStatusHandler>         _frameStatusHandler;
    std::vector<GoToSleepHandler>           _goToSleepHandler;
    std::vector<WakeupHandler>              _wakeupHandler;
    std::vector<FrameResponseUpdateHandler> _frameResponseUpdateHandler;

    extensions::Tracer _tracer;
};

// ================================================================================
//  Inline Implementations
// ================================================================================
auto LinController::GetLinNode(mw::EndpointAddress addr) -> LinNode&
{
    auto iter = std::lower_bound(_linNodes.begin(), _linNodes.end(), addr,
        [](const LinNode& lhs, const mw::EndpointAddress& address) { return lhs.ibAddress < address; }
    );
    if (iter == _linNodes.end() || iter->ibAddress != addr)
    {
        LinNode node;
        node.ibAddress = addr;
        iter = _linNodes.insert(iter, node);
    }
    return *iter;
}

void LinController::AddSink(extensions::ITraceMessageSink* sink)
{
    _tracer.AddSink(ib::mw::EndpointAddress{}, *sink);
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

