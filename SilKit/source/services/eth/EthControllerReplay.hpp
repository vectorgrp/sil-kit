// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IReplayDataController.hpp"
#include "EthController.hpp"

namespace SilKit {
namespace Services {
namespace Ethernet {

class EthControllerReplay
    : public IEthernetController
    , public IMsgForEthController
    , public SilKit::Core::Orchestration::ITimeConsumer
    , public ITraceMessageSource
    , public tracing::IReplayDataController
    , public Core::IServiceEndpoint
{
public:
    // Constructors 
    EthControllerReplay() = delete;
    EthControllerReplay(Core::IParticipantInternal* participant, Config::EthernetController config,
                        Core::Orchestration::ITimeProvider* timeProvider)
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
    void ReceiveSilKitMessage(const IServiceEndpoint* from, const EthernetFrameEvent& msg) override;
    void ReceiveSilKitMessage(const IServiceEndpoint* from, const EthernetFrameTransmitEvent& msg) override;
    void ReceiveSilKitMessage(const IServiceEndpoint* from, const EthernetStatus& msg) override;

    // SilKit::Core::Orchestration::ITimeConsumer
    void SetTimeProvider(SilKit::Core::Orchestration::ITimeProvider* timeProvider) override;

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
