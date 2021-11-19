// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <map>

#include "IReplayDataController.hpp"
#include "LinController.hpp"

namespace ib {
namespace sim {
namespace lin {

class LinControllerReplay
    : public ILinController
    , public IIbToLinController
    , public extensions::ITraceMessageSource
    , public tracing::IReplayDataController
    , public mw::IServiceId
{
public:
    // ----------------------------------------
    // Public Data Types

    LinControllerReplay(mw::IComAdapterInternal* comAdapter, cfg::LinController config,
            mw::sync::ITimeProvider* timeProvider);

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

public:
    // ----------------------------------------
    // Public interface methods

    // ITraceMessageSource
    void AddSink(extensions::ITraceMessageSink* sink) override;

    // IReplayDataProvider

    void ReplayMessage(const extensions::IReplayMessage* replayMessage) override;

    // IServiceId
    inline void SetServiceId(const mw::ServiceId& serviceId) override;
    inline auto GetServiceId() const -> const mw::ServiceId & override;

private:
    // ----------------------------------------
    // private members
    cfg::Replay _replayConfig{};
    LinController _controller;
    mw::IComAdapterInternal* _comAdapter{nullptr};
    // for local callbacks
    std::vector<FrameStatusHandler> _frameStatusHandler; 
    std::vector<GoToSleepHandler> _goToSleepHandler;
    ControllerMode _mode{ControllerMode::Inactive};
    // For tracing on a Master
    extensions::Tracer _tracer;
    mw::sync::ITimeProvider* _timeProvider{nullptr};
};

// ================================================================================
//  Inline Implementations
// ================================================================================

void LinControllerReplay::SetServiceId(const mw::ServiceId& serviceId)
{
    _controller.SetServiceId(serviceId);
}
auto LinControllerReplay::GetServiceId() const -> const mw::ServiceId&
{
    return _controller.GetServiceId();
}

} // namespace lin
} // namespace sim
} // namespace ib
