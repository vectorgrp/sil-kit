// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#include "GenericPublisher.hpp"

#include "ib/mw/IComAdapter.hpp"

namespace ib {
namespace sim {
namespace generic {

GenericPublisher::GenericPublisher(mw::IComAdapter* comAdapter)
    : _comAdapter{comAdapter}
{
}

GenericPublisher::GenericPublisher(mw::IComAdapter* comAdapter, cfg::GenericPort config)
    : _comAdapter{comAdapter}
    , _config{std::move(config)}
{
}

auto GenericPublisher::Config() const -> const cfg::GenericPort&
{
    return _config;
}

void GenericPublisher::Publish(std::vector<uint8_t> data)
{
    _comAdapter->SendIbMessage(_endpointAddr, GenericMessage{std::move(data)});
}

void GenericPublisher::Publish(const uint8_t* data, std::size_t size)
{
    Publish({data, data + size});
}

void GenericPublisher::SetEndpointAddress(const mw::EndpointAddress& endpointAddress)
{
    _endpointAddr = endpointAddress;
}

auto GenericPublisher::EndpointAddress() const -> const mw::EndpointAddress&
{
    return _endpointAddr;
}

} // namespace generic
} // namespace sim
} // namespace ib
