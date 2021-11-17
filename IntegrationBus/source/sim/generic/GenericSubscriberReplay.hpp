// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/fwd_decl.hpp"
#include "ib/mw/logging/ILogger.hpp"
#include "GenericSubscriber.hpp"
#include "IReplayDataController.hpp"


namespace ib {
namespace sim {
namespace generic {

class GenericSubscriberReplay
    : public IGenericSubscriber
    , public IIbToGenericSubscriber
    , public mw::sync::ITimeConsumer
    , public extensions::ITraceMessageSource
    , public tracing::IReplayDataController
{
public:
    // ----------------------------------------
    GenericSubscriberReplay(mw::IComAdapterInternal* comAdapter, cfg::GenericPort config,
        mw::sync::ITimeProvider* timeProvider);

public:
    // IGenericSubscriber
    void SetReceiveMessageHandler(CallbackT callback)  override;

    auto Config() const -> const cfg::GenericPort& override;

    void ReceiveIbMessage(mw::EndpointAddress from, const GenericMessage& msg) override;
    void SetEndpointAddress(const mw::EndpointAddress& endpointAddress) override;
    auto EndpointAddress() const -> const mw::EndpointAddress& override;

    //ib::mw::sync::ITimeConsumer
    void SetTimeProvider(mw::sync::ITimeProvider* provider) override;

    // ITraceMessageSource
    void AddSink(extensions::ITraceMessageSink* sink) override;

    // IReplayDataProvider

    void ReplayMessage(const extensions::IReplayMessage* replayMessage) override;
private:
    //Private methods
    void ReplayReceive(const extensions::IReplayMessage* replayMessage);
private:
    //private Members
    cfg::Replay _replayConfig;
    GenericSubscriber _subscriber;
    mw::logging::ILogger* _logger{nullptr};
};

} // namespace generic
} // namespace sim
} // namespace ib
