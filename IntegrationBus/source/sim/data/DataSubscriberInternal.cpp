// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "DataSubscriberInternal.hpp"

#include <cassert>

#include "ib/mw/logging/ILogger.hpp"

namespace ib {
namespace sim {
namespace data {

DataSubscriberInternal::DataSubscriberInternal(mw::IComAdapterInternal* comAdapter, cfg::DataPort config, mw::sync::ITimeProvider* timeProvider, DataHandlerT defaultHandler, IDataSubscriber* parent)
  : _comAdapter{comAdapter}, _timeProvider{timeProvider}, _defaultHandler{ defaultHandler }, _parent {parent}
{
    // NB: _config contains the joined dataExchangeFormat
    _config = std::move(config);
}

auto DataSubscriberInternal::Config() const -> const cfg::DataPort&
{
    return _config;
}

void DataSubscriberInternal::SetDefaultReceiveMessageHandler(DataHandlerT handler)
{
    _defaultHandler = std::move(handler);
}

void DataSubscriberInternal::RegisterSpecificDataHandlerInternal(DataHandlerT handler)
{
    _specificHandlers.push_back(handler);
}

void DataSubscriberInternal::ReceiveIbMessage(const mw::IIbServiceEndpoint* from, const DataMessage& msg)
{
    if (AllowMessageProcessing(from->GetServiceDescriptor(), _serviceDescriptor))
        return;

    ReceiveMessage(msg.data);
}

void DataSubscriberInternal::ReceiveMessage(const std::vector<uint8_t>& data)
{
    bool anySpecificHandlerExecuted{ false };
    if (!_specificHandlers.empty())
    {
        for (auto handler : _specificHandlers)
        {
            if (handler)
            {
                handler(_parent, data);
                anySpecificHandlerExecuted = true;
            }
        }
    }

    if (_defaultHandler && !anySpecificHandlerExecuted)
    {
        _defaultHandler(_parent, data);
    }
}

void DataSubscriberInternal::SetTimeProvider(mw::sync::ITimeProvider* provider)
{
    _timeProvider = provider;
}


} // namespace data
} // namespace sim
} // namespace ib
