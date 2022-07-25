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

#include "IReplayDataController.hpp"
#include "EthController.hpp"

namespace SilKit {
namespace Services {
namespace Ethernet {

class EthControllerReplay
    : public IEthernetController
    , public IMsgForEthController
    , public SilKit::Services::Orchestration::ITimeConsumer
    , public ITraceMessageSource
    , public tracing::IReplayDataController
    , public Core::IServiceEndpoint
{
public:
    // Constructors 
    EthControllerReplay() = delete;
    EthControllerReplay(Core::IParticipantInternal* participant, Config::EthernetController config,
                        Services::Orchestration::ITimeProvider* timeProvider)
        : _replayConfig{ config.replay }
        , _controller{ participant, config, timeProvider }
    {

    }

public:
    // ----------------------------------------
    // Public interface methods
    //
    // IEthernetController
    void Activate() override;
    void Deactivate() override;

    auto SendFrameEvent(EthernetFrameEvent msg) -> EthernetTxId;

    auto SendFrame(EthernetFrame msg) -> EthernetTxId override;
    void AddFrameHandler(FrameHandler handler) override;
    void AddFrameTransmitHandler(FrameTransmitHandler handler) override;
    void AddStateChangeHandler(StateChangeHandler handler) override;
    void AddBitrateChangeHandler(BitrateChangeHandler handler) override;

    // IMsgForEthController
    void ReceiveMsg(const IServiceEndpoint* from, const EthernetFrameEvent& msg) override;
    void ReceiveMsg(const IServiceEndpoint* from, const EthernetFrameTransmitEvent& msg) override;
    void ReceiveMsg(const IServiceEndpoint* from, const EthernetStatus& msg) override;

    // SilKit::Services::Orchestration::ITimeConsumer
    void SetTimeProvider(SilKit::Services::Orchestration::ITimeProvider* timeProvider) override;

    // ITraceMessageSource
    void AddSink(ITraceMessageSink* sink) override;

    // IReplayDataProvider

    void ReplayMessage(const IReplayMessage* replayMessage) override;

    // IServiceEndpoint
    inline void SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const Core::ServiceDescriptor & override;

private:
    //Private methods
    void ReplaySend(const IReplayMessage* replayMessage);
    void ReplayReceive(const IReplayMessage* replayMessage);
private:
    SilKit::Util::Optional<Config::Replay> _replayConfig;
    EthController _controller;
};

// ================================================================================
//  Inline Implementations
// ================================================================================
void EthControllerReplay::SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor)
{
    _controller.SetServiceDescriptor(serviceDescriptor);
}
auto EthControllerReplay::GetServiceDescriptor() const -> const Core::ServiceDescriptor&
{
    return _controller.GetServiceDescriptor();
}

} // namespace Ethernet
} // namespace Services
} // namespace SilKit
