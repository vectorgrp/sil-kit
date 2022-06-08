// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IReplayDataController.hpp"
#include "EthController.hpp"

namespace ib {
namespace sim {
namespace eth {

class EthControllerReplay
    : public IEthernetController
    , public IIbToEthController
    , public ib::mw::sync::ITimeConsumer
    , public extensions::ITraceMessageSource
    , public tracing::IReplayDataController
    , public mw::IIbServiceEndpoint
{
public:
    // Constructors 
    EthControllerReplay() = delete;
    EthControllerReplay(mw::IParticipantInternal* participant, cfg::EthernetController config,
                        mw::sync::ITimeProvider* timeProvider)
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

    // IIbToEthController
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const EthernetFrameEvent& msg) override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const EthernetFrameTransmitEvent& msg) override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const EthernetStatus& msg) override;

    // ib::mw::sync::ITimeConsumer
    void SetTimeProvider(ib::mw::sync::ITimeProvider* timeProvider) override;

    // ITraceMessageSource
    void AddSink(extensions::ITraceMessageSink* sink) override;

    // IReplayDataProvider

    void ReplayMessage(const extensions::IReplayMessage* replayMessage) override;

    // IIbServiceEndpoint
    inline void SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const mw::ServiceDescriptor & override;

private:
    //Private methods
    void ReplaySend(const extensions::IReplayMessage* replayMessage);
    void ReplayReceive(const extensions::IReplayMessage* replayMessage);
private:
    ib::util::Optional<cfg::Replay> _replayConfig;
    EthController _controller;
};

// ================================================================================
//  Inline Implementations
// ================================================================================
void EthControllerReplay::SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor)
{
    _controller.SetServiceDescriptor(serviceDescriptor);
}
auto EthControllerReplay::GetServiceDescriptor() const -> const mw::ServiceDescriptor&
{
    return _controller.GetServiceDescriptor();
}

} // namespace eth
} // namespace sim
} // namespace ib
