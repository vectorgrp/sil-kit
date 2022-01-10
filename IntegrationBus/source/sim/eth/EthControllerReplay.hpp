// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IReplayDataController.hpp"
#include "EthController.hpp"

namespace ib {
namespace sim {
namespace eth {

class EthControllerReplay
    : public IEthController
    , public IIbToEthController
    , public ib::mw::sync::ITimeConsumer
    , public extensions::ITraceMessageSource
    , public tracing::IReplayDataController
    , public mw::IIbServiceEndpoint
{
public:
    // Constructors 
    EthControllerReplay() = delete;
    EthControllerReplay(mw::IComAdapterInternal* comAdapter, cfg::EthernetController config, mw::sync::ITimeProvider* timeProvider)
        : _controller{comAdapter, config, timeProvider}
        , _replayConfig{config.replay}
    {

    }

public:
    // ----------------------------------------
    // Public interface methods
    //
    // IEthController
    void Activate() override;
    void Deactivate() override;

    auto SendMessage(EthMessage msg) -> EthTxId override;

    auto SendFrame(EthFrame msg) -> EthTxId override;
    auto SendFrame(EthFrame msg, std::chrono::nanoseconds timestamp) -> EthTxId override;
    void RegisterReceiveMessageHandler(ReceiveMessageHandler handler) override;
    void RegisterMessageAckHandler(MessageAckHandler handler) override;
    void RegisterStateChangedHandler(StateChangedHandler handler) override;
    void RegisterBitRateChangedHandler(BitRateChangedHandler handler) override;

    // IIbToEthController
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const EthMessage& msg) override;

    void SetEndpointAddress(const ib::mw::EndpointAddress& endpointAddress) override;
    auto EndpointAddress() const -> const ib::mw::EndpointAddress & override;

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
    cfg::Replay _replayConfig;
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
