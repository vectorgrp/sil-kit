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
    , public mw::IIbServiceEndpoint
{
public:
    // ----------------------------------------
    // Public Data Types

    LinControllerReplay(mw::IParticipantInternal* participant, cfg::LinController config,
            mw::sync::ITimeProvider* timeProvider);

public:
    // ----------------------------------------
    // Public interface methods
    //
    // ILinController
    void Init(LinControllerConfig config) override;
    auto Status() const noexcept -> LinControllerStatus override;

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
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const LinControllerStatusUpdate& msg) override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const LinFrameResponseUpdate& msg) override;

public:
    // ----------------------------------------
    // Public interface methods

    // ITraceMessageSource
    void AddSink(extensions::ITraceMessageSink* sink) override;

    // IReplayDataProvider

    void ReplayMessage(const extensions::IReplayMessage* replayMessage) override;

    // IIbServiceEndpoint
    inline void SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const mw::ServiceDescriptor & override;

private:
    // ----------------------------------------
    // private members
    cfg::Replay _replayConfig{};
    LinController _controller;
    mw::IParticipantInternal* _participant{nullptr};
    // for local callbacks
    std::vector<FrameStatusHandler> _frameStatusHandler; 
    std::vector<GoToSleepHandler> _goToSleepHandler;
    LinControllerMode _mode{LinControllerMode::Inactive};
    // For tracing on a Master
    extensions::Tracer _tracer;
    mw::sync::ITimeProvider* _timeProvider{nullptr};
};

// ================================================================================
//  Inline Implementations
// ================================================================================

void LinControllerReplay::SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor)
{
    _controller.SetServiceDescriptor(serviceDescriptor);
}
auto LinControllerReplay::GetServiceDescriptor() const -> const mw::ServiceDescriptor&
{
    return _controller.GetServiceDescriptor();
}

} // namespace lin
} // namespace sim
} // namespace ib
