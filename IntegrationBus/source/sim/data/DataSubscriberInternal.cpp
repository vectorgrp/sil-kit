// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "DataSubscriberInternal.hpp"

#include <cassert>

#include "ib/mw/logging/ILogger.hpp"

namespace ib {
namespace sim {
namespace data {

DataSubscriberInternal::DataSubscriberInternal(mw::IComAdapterInternal* comAdapter, cfg::DataPort config, mw::sync::ITimeProvider* timeProvider, CallbackExchangeFormatT callback)
  : _comAdapter{comAdapter}, _timeProvider{timeProvider}, _callback{callback}
{
    // NB: _config contains the joined dataExchangeFormat
    _config = std::move(config);
}

auto DataSubscriberInternal::Config() const -> const cfg::DataPort&
{
    return _config;
}

void DataSubscriberInternal::SetReceiveMessageHandler(CallbackExchangeFormatT callback)
{
    _callback = std::move(callback);
}

void DataSubscriberInternal::ReceiveIbMessage(const mw::IIbServiceEndpoint* from, const DataMessage& msg)
{
    if (AllowMessageProcessing(from->GetServiceDescriptor(), _serviceDescriptor))
        return;

    ReceiveMessage(msg.data);
}

void DataSubscriberInternal::ReceiveMessage(const std::vector<uint8_t>& data)
{
    if (_callback)
        _callback(this, data, _config.dataExchangeFormat);
}

void DataSubscriberInternal::SetTimeProvider(mw::sync::ITimeProvider* provider)
{
    _timeProvider = provider;
}


} // namespace data
} // namespace sim
} // namespace ib
