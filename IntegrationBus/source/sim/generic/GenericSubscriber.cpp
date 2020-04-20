// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "GenericSubscriber.hpp"

#include <cassert>

#include "ib/mw/IComAdapter.hpp"

namespace ib {
namespace sim {
namespace generic {

GenericSubscriber::GenericSubscriber(mw::IComAdapter* comAdapter)
    : _comAdapter{comAdapter}
{
}

GenericSubscriber::GenericSubscriber(mw::IComAdapter* comAdapter, cfg::GenericPort config)
    : _comAdapter{comAdapter}
    , _config{std::move(config)}
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

    _callback(this, msg.data);
}

} // namespace generic
} // namespace sim
} // namespace ib
