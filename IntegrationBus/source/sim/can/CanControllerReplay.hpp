// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IReplayDataController.hpp"
#include "CanController.hpp"
#include "IIbServiceEndpoint.hpp"

namespace ib {
namespace sim {
namespace can {

class CanControllerReplay
    : public ICanController
    , public IIbToCanController
    , public mw::sync::ITimeConsumer
    , public extensions::ITraceMessageSource
    , public tracing::IReplayDataController
    , public mw::IIbServiceEndpoint
{
public:
    // ----------------------------------------
    // Constructors and Destructor
    CanControllerReplay(mw::IComAdapterInternal* comAdapter, cfg::CanController config, mw::sync::ITimeProvider* timeProvider);

public:
    // ----------------------------------------
    // Public interface methods
    //
    // ICanControllerReplay
    void SetBaudRate(uint32_t rate, uint32_t fdRate) override;

    void Reset() override;
    void Start() override;
    void Stop() override;
    void Sleep() override;

    auto SendMessage(const CanMessage& msg) -> CanTxId override;
    auto SendMessage(CanMessage&& msg) -> CanTxId override;

    void RegisterReceiveMessageHandler(ReceiveMessageHandler handler) override;
    void RegisterStateChangedHandler(StateChangedHandler handler) override;
    void RegisterErrorStateChangedHandler(ErrorStateChangedHandler handler) override;
    void RegisterTransmitStatusHandler(MessageStatusHandler handler) override;

    // IIbToCanControllerReplay
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const sim::can::CanMessage& msg) override;

    void SetEndpointAddress(const ::ib::mw::EndpointAddress& endpointAddress) override;
    auto EndpointAddress() const -> const ::ib::mw::EndpointAddress& override;

    //ITimeConsumer
    void SetTimeProvider(ib::mw::sync::ITimeProvider* timeProvider) override;

    // ITraceMessageSource
    void AddSink(extensions::ITraceMessageSink* sink) override;
    
    // IReplayDataProvider

    void ReplayMessage(const extensions::IReplayMessage* replayMessage) override;

    // IIbServiceEndpoint
    inline void SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const mw::ServiceDescriptor & override;

public:
    // ----------------------------------------
    // Public interface methods

private:
    // ----------------------------------------
    //Private methods
    
    void ReplaySend(const extensions::IReplayMessage* replayMessage);
    void ReplayReceive(const extensions::IReplayMessage* replayMessage);

private:
    // ----------------------------------------
    // private members

    cfg::Replay _replayConfig;
    CanController _controller;
};

// ================================================================================
//  Inline Implementations
// ================================================================================
void CanControllerReplay::SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor)
{
    _controller.SetServiceDescriptor(serviceDescriptor);
}
auto CanControllerReplay::GetServiceDescriptor() const -> const mw::ServiceDescriptor&
{
    return _controller.GetServiceDescriptor();
}

} // namespace can
} // namespace sim
} // namespace ib
