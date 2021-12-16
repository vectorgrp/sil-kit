// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "GenericSubscriber.hpp"

#include <cassert>

#include "ib/mw/logging/ILogger.hpp"

namespace ib {
namespace sim {
namespace generic {

GenericSubscriber::GenericSubscriber(mw::IComAdapterInternal* comAdapter, mw::sync::ITimeProvider* timeProvider)
    : _comAdapter{comAdapter}
    , _timeProvider{timeProvider}
{
}

GenericSubscriber::GenericSubscriber(mw::IComAdapterInternal* comAdapter, cfg::GenericPort config, mw::sync::ITimeProvider* timeProvider)
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
    _serviceId.legacyEpa = endpointAddress;
}

auto GenericSubscriber::EndpointAddress() const -> const mw::EndpointAddress&
{
    return _serviceId.legacyEpa;
}

void GenericSubscriber::ReceiveIbMessage(const mw::IIbServiceEndpoint* from, const GenericMessage& msg)
{
    if (AllowMessageProcessing(from->GetServiceId(), _serviceId))
        return;
    ReceiveMessage(msg);
}

void GenericSubscriber::ReceiveMessage( const GenericMessage& msg)
{
    _tracer.Trace(extensions::Direction::Receive,
        _timeProvider->Now(),
        msg);

    if (_callback)
        _callback(this, msg.data);
}


void GenericSubscriber::SetTimeProvider(mw::sync::ITimeProvider* provider)
{
    _timeProvider = provider;
}


} // namespace generic
} // namespace sim
} // namespace ib
