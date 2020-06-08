// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "GenericSubscriber.hpp"

#include <cassert>

#include "ib/mw/IComAdapter.hpp"

namespace ib {
namespace sim {
namespace generic {

GenericSubscriber::GenericSubscriber(mw::IComAdapter* comAdapter, mw::sync::ITimeProvider* timeProvider)
    : _comAdapter{comAdapter}
    , _timeProvider{timeProvider}
{
}

GenericSubscriber::GenericSubscriber(mw::IComAdapter* comAdapter, cfg::GenericPort config, mw::sync::ITimeProvider* timeProvider)
    : _comAdapter{comAdapter}
    , _config{std::move(config)}
    , _timeProvider{timeProvider}
{
}

auto GenericSubscriber::Config() const -> const cfg::GenericPort&
{
    return _config;
}

void GenericSubscriber::SetReceiveMessageHandler(CallbackT callback)
{
    _callback = std::move(callback);
}

void GenericSubscriber::SetEndpointAddress(const mw::EndpointAddress& endpointAddress)
{
    _endpointAddr = endpointAddress;
}

auto GenericSubscriber::EndpointAddress() const -> const mw::EndpointAddress&
{
    return _endpointAddr;
}

void GenericSubscriber::ReceiveIbMessage(mw::EndpointAddress from, const GenericMessage& msg)
{
    assert(from != _endpointAddr);

    if (!_callback)
        return;

    _tracer.Trace(tracing::Direction::Receive,
        _timeProvider->Now(),
        msg);

    _callback(this, msg.data);
}

void GenericSubscriber::SetTimeProvider(mw::sync::ITimeProvider* provider)
{
    _timeProvider = provider;
}



} // namespace generic
} // namespace sim
} // namespace ib
