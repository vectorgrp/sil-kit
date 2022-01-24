// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <memory>

#include "ib/sim/lin/ILinController.hpp"
#include "ib/extensions/ITraceMessageSource.hpp"
#include "ib/mw/fwd_decl.hpp"

#include "IIbToLinControllerFacade.hpp"
#include "IComAdapterInternal.hpp"

#include "LinController.hpp"
#include "LinControllerProxy.hpp"

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
    LinControllerFacade(const LinControllerFacade&) = default;
    LinControllerFacade(LinControllerFacade&&) = default;
    LinControllerFacade(mw::IComAdapterInternal* comAdapter, cfg::LinController config, mw::sync::ITimeProvider* timeProvider);

public:
    // ----------------------------------------
    // Operator Implementations
    LinControllerFacade& operator=(LinControllerFacade& other) = default;
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

    void SetEndpointAddress(const mw::EndpointAddress& endpointAddress) override;
    auto EndpointAddress() const -> const mw::EndpointAddress& override;

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
    auto DefaultFilter(const IIbServiceEndpoint* from) const -> bool;
    auto ProxyFilter(const IIbServiceEndpoint* from) const -> bool;
    auto IsLinkSimulated() const -> bool;

private:
    // ----------------------------------------
    // private members
    mw::IComAdapterInternal* _comAdapter;
    mw::ServiceDescriptor _serviceDescriptor;
    cfg::LinController _config;

    mw::ServiceDescriptor _remoteBusSimulator;

    ILinController* _currentController;
    std::unique_ptr<LinController> _linController;
    std::unique_ptr<LinControllerProxy> _linControllerProxy;
};

} // namespace lin
} // namespace sim
} // namespace ib
