// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <map>

#include "IReplayDataController.hpp"
#include "LinController.hpp"

namespace SilKit {
namespace Services {
namespace Lin {

class LinControllerReplay
    : public ILinController
    , public IMsgForLinController
    , public ITraceMessageSource
    , public tracing::IReplayDataController
    , public Core::IServiceEndpoint
{
public:
    // ----------------------------------------
    // Public Data Types

    LinControllerReplay(Core::IParticipantInternal* participant, Config::LinController config,
            Services::Orchestration::ITimeProvider* timeProvider);

public:
    // ----------------------------------------
    // Public interface methods
    //
    // ILinController
    void Init(LinControllerConfig config) override;
    auto Status() const noexcept -> LinControllerStatus override;

    void SendFrame(LinFrame frame, LinFrameResponseType responseType) override;
    void SendFrameHeader(LinIdT linId) override;
    void UpdateTxBuffer(LinFrame frame, LinFrameResponseMode mode) override;
    void SetFrameResponses(std::vector<LinFrameResponse> responses) override;

    void GoToSleep() override;
    void GoToSleepInternal() override;
    void Wakeup() override;
    void WakeupInternal() override;

    void AddFrameStatusHandler(FrameStatusHandler handler) override;
    void AddGoToSleepHandler(GoToSleepHandler handler) override;
    void AddWakeupHandler(WakeupHandler handler) override;
    void AddLinSlaveConfigurationHandler(LinSlaveConfigurationHandler handler) override;

    // IMsgForLinController
    void ReceiveMsg(const IServiceEndpoint* from, const LinTransmission& msg) override;
    void ReceiveMsg(const IServiceEndpoint* from, const LinWakeupPulse& msg) override;
    void ReceiveMsg(const IServiceEndpoint* from, const LinControllerConfig& msg) override;
    void ReceiveMsg(const IServiceEndpoint* from, const LinControllerStatusUpdate& msg) override;

public:
    // ----------------------------------------
    // Public interface methods

    // ITraceMessageSource
    void AddSink(ITraceMessageSink* sink) override;

    // IReplayDataProvider

    void ReplayMessage(const IReplayMessage* replayMessage) override;

    // IServiceEndpoint
    inline void SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const Core::ServiceDescriptor & override;

private:
    // ----------------------------------------
    // private members
    Config::Replay _replayConfig{};
    LinController _controller;
    Core::IParticipantInternal* _participant{nullptr};
    // for local callbacks
    std::vector<FrameStatusHandler> _frameStatusHandler; 
    std::vector<GoToSleepHandler> _goToSleepHandler;
    LinControllerMode _mode{LinControllerMode::Inactive};
    // For tracing on a Master
    Tracer _tracer;
    Services::Orchestration::ITimeProvider* _timeProvider{nullptr};
};

// ================================================================================
//  Inline Implementations
// ================================================================================

void LinControllerReplay::SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor)
{
    _controller.SetServiceDescriptor(serviceDescriptor);
}
auto LinControllerReplay::GetServiceDescriptor() const -> const Core::ServiceDescriptor&
{
    return _controller.GetServiceDescriptor();
}

} // namespace Lin
} // namespace Services
} // namespace SilKit
