// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "GenericPublisherReplay.hpp"

namespace ib {
namespace sim {
namespace generic {

GenericPublisherReplay::GenericPublisherReplay(mw::IComAdapter* comAdapter,
        cfg::GenericPort config, mw::sync::ITimeProvider* timeProvider)
    : _publisher{comAdapter, config, timeProvider}
    , _replayConfig{config.replay}
{

}


void GenericPublisherReplay::Publish(std::vector<uint8_t> data)
{
    // ignore the user's API calls if we're configured for replay
    if (tracing::IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Send))
    {
        return;
    }

    return _publisher.Publish(std::move(data));
}

void GenericPublisherReplay::Publish(const uint8_t* data, std::size_t size)
{
    // ignore the user's API calls if we're configured for replay
    if (tracing::IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Send))
    {
        return;
    }
    _publisher.Publish(data, size);
}

auto GenericPublisherReplay::Config() const -> const cfg::GenericPort&
{
    return _publisher.Config();
}


void GenericPublisherReplay::SetEndpointAddress(const ib::mw::EndpointAddress& endpointAddress)
{
    _publisher.SetEndpointAddress(endpointAddress);
}

auto GenericPublisherReplay::EndpointAddress() const -> const ib::mw::EndpointAddress&
{
    return _publisher.EndpointAddress();
}

// ib::mw::sync::ITimeConsumer
void GenericPublisherReplay::SetTimeProvider(ib::mw::sync::ITimeProvider* timeProvider)
{
    _publisher.SetTimeProvider(timeProvider);
}

// ITraceMessageSource
void GenericPublisherReplay::AddSink(extensions::ITraceMessageSink* sink)
{
    _publisher.AddSink(sink);
}

// IReplayDataProvider

void GenericPublisherReplay::ReplayMessage(const extensions::IReplayMessage* replayMessage)
{
    using namespace ib::tracing;
    switch (replayMessage->GetDirection())
    {
    case extensions::Direction::Send:
        if (IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Send))
        {
            ReplaySend(replayMessage);
        }
        break;
    default:
        throw std::runtime_error("GenericPublisherReplay: replay message has undefined Direction");
        break;
    }

}


void GenericPublisherReplay::ReplaySend(const extensions::IReplayMessage* replayMessage)
{
    // need to copy the message here.
    // will throw if invalid message type.
    sim::generic::GenericMessage msg = dynamic_cast<const sim::generic::GenericMessage&>(*replayMessage);
    _publisher.Publish(std::move(msg.data));
}

} // namespace generic
} // namespace sim
} // namespace ib
