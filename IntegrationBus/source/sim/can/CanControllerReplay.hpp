// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IReplayDataController.hpp"
#include "CanController.hpp"

namespace ib {
namespace sim {
namespace can {

class CanControllerReplay
    : public ICanController
    , public IIbToCanController
    , public mw::sync::ITimeConsumer
    , public extensions::ITraceMessageSource
    , public tracing::IReplayDataController
{
public:
    // ----------------------------------------
    // Constructors and Destructor
    CanControllerReplay(mw::IComAdapter* comAdapter, cfg::CanController config, mw::sync::ITimeProvider* timeProvider);

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
    void ReceiveIbMessage(ib::mw::EndpointAddress from, const sim::can::CanMessage& msg) override;

    void SetEndpointAddress(const ::ib::mw::EndpointAddress& endpointAddress) override;
    auto EndpointAddress() const -> const ::ib::mw::EndpointAddress& override;

    //ITimeConsumer
    void SetTimeProvider(ib::mw::sync::ITimeProvider* timeProvider) override;

    // ITraceMessageSource
    void AddSink(extensions::ITraceMessageSink* sink) override;
    
    // IReplayDataProvider

    void ReplayMessage(const extensions::IReplayMessage* replayMessage) override;

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


} // namespace can
} // namespace sim
} // namespace ib
