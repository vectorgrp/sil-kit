// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <memory>

#include "ib/sim/lin/ILinController.hpp"
#include "ib/extensions/ITraceMessageSource.hpp"
#include "ib/mw/fwd_decl.hpp"

#include "IIbToLinControllerFacade.hpp"
#include "IParticipantInternal.hpp"

#include "LinController.hpp"
#include "LinControllerProxy.hpp"

#include "ParticipantConfiguration.hpp"

namespace ib {
namespace sim {
namespace lin {

class LinControllerFacade
    : public ILinController
    , public IIbToLinControllerFacade
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
    LinControllerFacade() = delete;
    LinControllerFacade(LinControllerFacade&&) = default;
    LinControllerFacade(mw::IParticipantInternal* participant, cfg::LinController config, mw::sync::ITimeProvider* timeProvider);

public:
    // ----------------------------------------
    // Operator Implementations
    LinControllerFacade& operator=(LinControllerFacade&& other) = default;

public:
    // ----------------------------------------
    // Public interface methods
    //
    // ILinController
    void Init(ControllerConfig config) override;
    auto Status() const noexcept->ControllerStatus override;

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
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const Transmission& msg) override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const WakeupPulse& msg) override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const ControllerConfig& msg) override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const ControllerStatusUpdate& msg) override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const FrameResponseUpdate& msg) override;

public:
    // ----------------------------------------
    // Public interface methods
  
    //ib::mw::sync::ITimeConsumer
    void SetTimeProvider(mw::sync::ITimeProvider* timeProvider) override;

    //ITraceMessageSource
    void AddSink(extensions::ITraceMessageSink* sink) override;

    // IIbServiceEndpoint
    void SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor) override;
    auto GetServiceDescriptor() const -> const mw::ServiceDescriptor & override;

private:
    // ----------------------------------------
    // Private helper methods
    //
    auto AllowForwardToDefault(const IIbServiceEndpoint* from) const -> bool;
    auto AllowForwardToProxy(const IIbServiceEndpoint* from) const -> bool;
    auto IsNetworkSimulated() const -> bool;
    auto IsRelevantNetwork(const mw::ServiceDescriptor& remoteServiceDescriptor) const -> bool;

private:
    // ----------------------------------------
    // private members
    mw::IParticipantInternal* _participant;
    mw::ServiceDescriptor _serviceDescriptor;
    cfg::LinController _config;

    bool _simulatedLinkDetected = false;
    mw::ServiceDescriptor _simulatedLink;

    ILinController* _currentController;
    std::unique_ptr<LinController> _linController;
    std::unique_ptr<LinControllerProxy> _linControllerProxy;
};

} // namespace lin
} // namespace sim
} // namespace ib
