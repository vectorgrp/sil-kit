// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "GenericSubscriberReplay.hpp"

namespace ib {
namespace sim {
namespace generic {

GenericSubscriberReplay::GenericSubscriberReplay(mw::IComAdapterInternal* comAdapter,
        cfg::GenericPort config, mw::sync::ITimeProvider* timeProvider)
    : _subscriber{comAdapter, config, timeProvider}
    , _replayConfig{config.replay}
    , _logger{comAdapter->GetLogger()}
{

}


void GenericSubscriberReplay::SetReceiveMessageHandler(CallbackT callback)
{
    _subscriber.SetReceiveMessageHandler(std::move(callback));
}

void GenericSubscriberReplay::ReceiveIbMessage(mw::EndpointAddress from, const GenericMessage& msg)
{
    if (tracing::IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Receive))
    {
        return;
    }
    return _subscriber.ReceiveIbMessage(from, msg);

}

auto GenericSubscriberReplay::Config() const -> const cfg::GenericPort&
{
    return _subscriber.Config();
}


void GenericSubscriberReplay::SetEndpointAddress(const ib::mw::EndpointAddress& endpointAddress)
{
    _subscriber.SetEndpointAddress(endpointAddress);
}

auto GenericSubscriberReplay::EndpointAddress() const -> const ib::mw::EndpointAddress&
{
    return _subscriber.EndpointAddress();
}

// ib::mw::sync::ITimeConsumer
void GenericSubscriberReplay::SetTimeProvider(ib::mw::sync::ITimeProvider* timeProvider)
{
    _subscriber.SetTimeProvider(timeProvider);
}

// ITraceMessageSource
void GenericSubscriberReplay::AddSink(extensions::ITraceMessageSink* sink)
{
    _subscriber.AddSink(sink);
}

// IReplayDataProvider

void GenericSubscriberReplay::ReplayMessage(const extensions::IReplayMessage* replayMessage)
{
    using namespace ib::tracing;
    switch (replayMessage->GetDirection())
    {
    case extensions::Direction::Receive:
        if (IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Receive))
        {
            ReplayReceive(replayMessage);
        }
        break;
    case extensions::Direction::Send:
        _logger->Warn("GenericSubscriberReplay: Cannot replay message with direction Send");
        break;
    default:
        throw std::runtime_error("GenericSubscriberReplay: replay message has undefined Direction");
        break;
    }

}


void GenericSubscriberReplay::ReplayReceive(const extensions::IReplayMessage* replayMessage)
{
    // need to copy the message here.
    // will throw if invalid message type.
    sim::generic::GenericMessage msg = dynamic_cast<const sim::generic::GenericMessage&>(*replayMessage);
    _subscriber.ReceiveIbMessage(tracing::ReplayEndpointAddress(), msg);
}

} // namespace generic
} // namespace sim
} // namespace ib
