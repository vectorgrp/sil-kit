/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

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
    void SendFrameHeader(LinId linId) override;
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
