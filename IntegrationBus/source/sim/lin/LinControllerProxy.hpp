// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <memory>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "ib/sim/lin/ILinController.hpp"
#include "ib/mw/fwd_decl.hpp"

#include "IIbToLinControllerProxy.hpp"
#include "IParticipantInternal.hpp"
#include "ITraceMessageSource.hpp"

namespace ib {
namespace sim {
namespace lin {

class LinControllerProxy
    : public ILinController
    , public IIbToLinControllerProxy
    , public extensions::ITraceMessageSource
    , public mw::IIbServiceEndpoint
{
public:
    // ----------------------------------------
    // Public Data Types

public:
    // ----------------------------------------
    // Constructors and Destructor
    LinControllerProxy() = delete;
    LinControllerProxy(const LinControllerProxy&) = default;
    LinControllerProxy(LinControllerProxy&&) = default;
    LinControllerProxy(mw::IParticipantInternal* participant, ILinController* facade = nullptr);

public:
    // ----------------------------------------
    // Operator Implementations
    LinControllerProxy& operator=(LinControllerProxy& other) = default;
    LinControllerProxy& operator=(LinControllerProxy&& other) = default;

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

    void AddFrameStatusHandler(FrameStatusHandler handler) override;
    void AddGoToSleepHandler(GoToSleepHandler handler) override;
    void AddWakeupHandler(WakeupHandler handler) override;
    void AddFrameResponseUpdateHandler(FrameResponseUpdateHandler handler) override;

    // IIbToLinController
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const LinTransmission& msg) override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const LinWakeupPulse& msg) override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const LinControllerConfig& msg) override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const LinFrameResponseUpdate& msg) override;

public:
    // ----------------------------------------
    // Public interface methods

    //ITraceMessageSource
    inline void AddSink(extensions::ITraceMessageSink* sink) override;

    // IIbServiceEndpoint
    inline void SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const mw::ServiceDescriptor & override;

private:
//    // ----------------------------------------
//    // private data types

private:
    // ----------------------------------------
    // private methods
    void SetControllerStatus(LinControllerStatus status);

    template <typename MsgT>
    inline void SendIbMessage(MsgT&& msg);

private:
    // ----------------------------------------
    // private members
    mw::IParticipantInternal* _participant;
    mw::logging::ILogger* _logger;
    ::ib::mw::ServiceDescriptor _serviceDescriptor;
    ILinController* _facade{ nullptr };

    LinControllerMode   _controllerMode{LinControllerMode::Inactive};
    LinControllerStatus _controllerStatus{LinControllerStatus::Unknown};

    std::vector<FrameStatusHandler>         _frameStatusHandler;
    std::vector<GoToSleepHandler>           _goToSleepHandler;
    std::vector<WakeupHandler>              _wakeupHandler;
    std::vector<FrameResponseUpdateHandler> _frameResponseUpdateHandler;

    extensions::Tracer _tracer;
};

//// ==================================================================
//  Inline Implementations
// ==================================================================
void LinControllerProxy::AddSink(extensions::ITraceMessageSink* sink)
{
    _tracer.AddSink(mw::EndpointAddress{}, *sink);
}

void LinControllerProxy::SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}
auto LinControllerProxy::GetServiceDescriptor() const -> const mw::ServiceDescriptor&
{
    return _serviceDescriptor;
}
} // namespace lin
} // namespace sim
} // namespace ib
