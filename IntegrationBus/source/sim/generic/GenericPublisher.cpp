// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "GenericPublisher.hpp"

namespace ib {
namespace sim {
namespace generic {

GenericPublisher::GenericPublisher(mw::IComAdapterInternal* comAdapter, mw::sync::ITimeProvider* timeProvider)
    : _comAdapter{comAdapter}
    , _timeProvider{timeProvider}
{
}

GenericPublisher::GenericPublisher(mw::IComAdapterInternal* comAdapter, cfg::GenericPort config, mw::sync::ITimeProvider* timeProvider)
    : GenericPublisher{comAdapter, timeProvider}
{
    _config = std::move(config);
}

auto GenericPublisher::Config() const -> const cfg::GenericPort&
{
    return _config;
}

void GenericPublisher::Publish(std::vector<uint8_t> data)
{
    GenericMessage msg{std::move(data)};
    _tracer.Trace(ib::sim::TransmitDirection::TX,
        _timeProvider->Now(),
        msg);

    _comAdapter->SendIbMessage(this, std::move(msg));
}

void GenericPublisher::Publish(const uint8_t* data, std::size_t size)
{
    Publish({data, data + size});
}

void GenericPublisher::SetTimeProvider(mw::sync::ITimeProvider* provider)
{
    _timeProvider = provider;
}


} // namespace generic
} // namespace sim
} // namespace ib
