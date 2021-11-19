// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once


#include <algorithm>
#include <memory>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "ib/sim/lin/ILinController.hpp"
#include "ib/mw/sync/ITimeConsumer.hpp"
#include "ib/extensions/ITraceMessageSource.hpp"

#include "ib/mw/fwd_decl.hpp"
#include "ib/sim/datatypes.hpp"

#include "IIbToLinController.hpp"
#include "IComAdapterInternal.hpp"
#include "IServiceId.hpp"

namespace ib {
namespace sim {
namespace lin {

class LinController
    : public ILinController
    , public IIbToLinController
    , public mw::sync::ITimeConsumer
    , public extensions::ITraceMessageSource
    , public mw::IServiceId
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
    LinController(mw::IComAdapterInternal* comAdapter, mw::sync::ITimeProvider* timeProvider);

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

    void SendFrame(Frame frame, FrameResponseType responseType) override;
    void SendFrame(Frame frame, FrameResponseType responseType, std::chrono::nanoseconds timestamp) override;
    void SendFrameHeader(LinIdT linId) override;
    void SendFrameHeader(LinIdT linId, std::chrono::nanoseconds timestamp) override;
    void SetFrameResponse(Frame frame, FrameResponseMode mode) override;
    void SetFrameResponses(std::vector<FrameResponse> responses) override;

    void GoToSleep() override;
    void GoToSleepInternal() override;
    void Wakeup() override;
    void WakeupInternal() override;

    void RegisterFrameStatusHandler(FrameStatusHandler handler) override;
    void RegisterGoToSleepHandler(GoToSleepHandler handler) override;
    void RegisterWakeupHandler(WakeupHandler handler) override;
    void RegisterFrameResponseUpdateHandler(FrameResponseUpdateHandler handler) override;

     // IIbToLinController
     void ReceiveIbMessage(const IServiceId* from, const Transmission& msg) override;
     void ReceiveIbMessage(const IServiceId* from, const WakeupPulse& msg) override;
     void ReceiveIbMessage(const IServiceId* from, const ControllerConfig& msg) override;
     void ReceiveIbMessage(const IServiceId* from, const ControllerStatusUpdate& msg) override;
     void ReceiveIbMessage(const IServiceId* from, const FrameResponseUpdate& msg) override;

     void SetEndpointAddress(const mw::EndpointAddress& endpointAddress) override;
     auto EndpointAddress() const -> const mw::EndpointAddress& override;

     //ib::mw::sync::ITimeConsumer
     void SetTimeProvider(mw::sync::ITimeProvider* timeProvider) override;

    // ITraceMessageSource
    inline void AddSink(extensions::ITraceMessageSink* sink) override;

    // IServiceId
    inline void SetServiceId(const mw::ServiceId& serviceId) override;
    inline auto GetServiceId() const -> const mw::ServiceId & override;

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
    auto VeriyChecksum(const Frame& frame, FrameStatus status) -> FrameStatus;

    template <typename MsgT>
    inline void SendIbMessage(MsgT&& msg);

    inline auto GetLinNode(mw::EndpointAddress addr) -> LinNode&;
    
private:
    // ----------------------------------------
    // private members
    mw::IComAdapterInternal* _comAdapter;
    ::ib::mw::ServiceId _serviceId;
    mw::logging::ILogger* _logger;
    mw::sync::ITimeProvider* _timeProvider{ nullptr };

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
        [](const LinNode& lhs, const mw::EndpointAddress& addr) { return lhs.ibAddress < addr; }
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
    _tracer.AddSink(EndpointAddress(), *sink);
}

void LinController::SetServiceId(const mw::ServiceId& serviceId)
{
    _serviceId = serviceId;
}
auto LinController::GetServiceId() const -> const mw::ServiceId&
{
    return _serviceId;
}
} // namespace lin
} // namespace sim
} // namespace ib

